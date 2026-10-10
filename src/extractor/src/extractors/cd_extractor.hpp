#pragma once
/*
 * cd_extractor.hpp — extraction of the CD audio tracks from a CUE image.
 *
 * Uses the header-only tiny-cdio library to open original/GAME.INST (a .cue
 * referencing GAME.GOG) and to *stream* every AUDIO track of the mixed-mode
 * disc (2352-byte raw Red Book sectors, 16-bit stereo 44100 Hz). Each streamed
 * track is encoded to Ogg/Vorbis on the fly (no whole-track buffer is kept in
 * memory) and written to data/music/{n}.ogg, where {n} is the track number as
 * declared in the cue (track 01 is the data track, so audio starts at 02).
 */

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#include "tiny-cdio/disc_iso.hpp"
#include <vorbis/vorbisenc.h>

namespace AITDExtractor {

    // -----------------------------------------------------------------------
    // Streaming Ogg/Vorbis encoder (mirrors music/music_extractor.cpp).
    //
    // Interleaved little-endian 16-bit PCM is fed in arbitrary-sized chunks;
    // compressed pages are flushed as the stream is produced.
    // -----------------------------------------------------------------------
    class OggVorbisWriter {
    public:
        OggVorbisWriter() = default;
        ~OggVorbisWriter() { close(); }

        OggVorbisWriter(const OggVorbisWriter&) = delete;
        OggVorbisWriter& operator=(const OggVorbisWriter&) = delete;

        /**
         * Create the output file and write the Vorbis headers.
         * @param quality 0.0 (low) .. 1.0 (high).
         * @return false if the file could not be created / the encoder failed.
         */
        bool open(const std::string& path, int channels = 2,
                  int sampleRate = 44100, float quality = 0.8f) {
            close();

            file_ = std::fopen(path.c_str(), "wb");
            if (!file_) return false;

            channels_   = channels;
            sampleRate_ = sampleRate;

            vorbis_info_init(&vi_);
            if (vorbis_encode_init_vbr(&vi_, channels, sampleRate, quality) != 0) {
                vorbis_info_clear(&vi_);
                std::fclose(file_);
                file_ = nullptr;
                return false;
            }

            vorbis_comment_init(&vc_);
            vorbis_comment_add_tag(&vc_, "ENCODER", "open-AITD");

            vorbis_analysis_init(&vd_, &vi_);
            vorbis_block_init(&vd_, &vb_);
            ogg_stream_init(&os_, std::rand());

            ogg_packet header{}, headerComm{}, headerCode{};
            vorbis_analysis_headerout(&vd_, &vc_, &header, &headerComm, &headerCode);
            ogg_stream_packetin(&os_, &header);
            ogg_stream_packetin(&os_, &headerComm);
            ogg_stream_packetin(&os_, &headerCode);
            while (ogg_stream_flush(&os_, &og_) != 0) {
                writePage();
            }

            open_ = true;
            return true;
        }

        /**
         * Feed a chunk of interleaved 16-bit PCM. `bytes` must be a multiple of
         * (channels * 2). Returns false on a non-open/failed stream.
         */
        bool write(const uint8_t* data, size_t bytes) {
            if (!open_ || !data) return false;

            const size_t frameBytes = static_cast<size_t>(channels_) * sizeof(int16_t);
            const size_t frames     = bytes / frameBytes;
            if (frames == 0) return true;

            const int16_t* samples = reinterpret_cast<const int16_t*>(data);

            size_t done = 0;
            while (done < frames) {
                int n = static_cast<int>(std::min<size_t>(kBlockFrames, frames - done));

                float** buffer = vorbis_analysis_buffer(&vd_, n);
                for (int ch = 0; ch < channels_; ++ch) {
                    for (int i = 0; i < n; ++i) {
                        buffer[ch][i] = samples[(done + i) * channels_ + ch] / 32768.0f;
                    }
                }
                vorbis_analysis_wrote(&vd_, n);
                done += n;

                drain();
            }
            return true;
        }

        /** Flush the encoder and close the file. Safe to call more than once. */
        void close() {
            if (open_) {
                vorbis_analysis_wrote(&vd_, 0); // end of stream
                drain();
                while (ogg_stream_flush(&os_, &og_) != 0) {
                    writePage();
                }

                ogg_stream_clear(&os_);
                vorbis_block_clear(&vb_);
                vorbis_dsp_clear(&vd_);
                vorbis_comment_clear(&vc_);
                vorbis_info_clear(&vi_);
                open_ = false;
            }
            if (file_) {
                std::fclose(file_);
                file_ = nullptr;
            }
        }

    private:
        void writePage() {
            if (og_.header_len > 0 && og_.header)
                std::fwrite(og_.header, 1, og_.header_len, file_);
            if (og_.body_len > 0 && og_.body)
                std::fwrite(og_.body, 1, og_.body_len, file_);
        }

        void drain() {
            while (vorbis_analysis_blockout(&vd_, &vb_) == 1) {
                vorbis_analysis(&vb_, nullptr);
                vorbis_bitrate_addblock(&vb_);

                while (vorbis_bitrate_flushpacket(&vd_, &op_)) {
                    ogg_stream_packetin(&os_, &op_);
                    while (ogg_stream_pageout(&os_, &og_) != 0) {
                        writePage();
                    }
                }
            }
        }

        static const int kBlockFrames = 1024; // frames per vorbis_analysis_buffer() call

        std::FILE*    file_       = nullptr;
        bool          open_       = false;
        int           channels_   = 2;
        int           sampleRate_ = 44100;

        ogg_stream_state os_{};
        ogg_page         og_{};
        ogg_packet       op_{};
        vorbis_info      vi_{};
        vorbis_comment   vc_{};
        vorbis_dsp_state vd_{};
        vorbis_block     vb_{};
    };

    // -----------------------------------------------------------------------
    // Extraction
    // -----------------------------------------------------------------------

    /**
     * Stream every AUDIO track of `cuePath` into `outDir` as Ogg/Vorbis.
     *
     * @param cuePath  path to the .cue (original/GAME.INST).
     * @param outDir   output directory (data/music); created if missing.
     * @param quality  Vorbis VBR quality, 0.0 .. 1.0.
     * @param overwrite re-encode tracks whose .ogg already exists.
     * @return number of tracks successfully written.
     */
    inline int extractCdAudioTracks(const std::string& cuePath = "original/GAME.INST",
                                    const std::string& outDir  = "data/music",
                                    float quality = 0.8f,
                                    bool overwrite = false) {
        tinycdio::DiscIso disc;
        disc.open(cuePath);

        std::filesystem::create_directories(outDir);

        int written = 0;
        for (size_t idx : disc.audioTrackIndices()) {
            const tinycdio::TrackInfo& t = disc.track(idx);

            // {n} = the track number from the cue (data track 01 is skipped).
            const std::string outPath =
                outDir + "/" + std::to_string(t.number) + ".ogg";

            if (!overwrite && std::filesystem::exists(outPath)) continue;

            OggVorbisWriter writer;
            if (!writer.open(outPath, 2, 44100, quality)) continue;

            // Stream the audio track sector by sector (nothing whole in memory).
            auto reader = disc.openAudioTrack(idx);
            std::vector<uint8_t> chunk;
            while (!(chunk = reader.readChunk(2352 * 16)).empty()) {
                writer.write(chunk.data(), chunk.size());
            }
            writer.close();

            if (reader.bad()) {
                std::filesystem::remove(outPath); // do not keep a truncated file
                continue;
            }
            ++written;
        }
        return written;
    }

} // namespace AITDExtractor
