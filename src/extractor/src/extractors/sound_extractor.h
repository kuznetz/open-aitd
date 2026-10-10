#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "../structs/int_types.h"
#include <vorbis/vorbisenc.h>
using namespace std;

struct VOCSample {
    char* data;
    int size;
    int rate;
};

VOCSample loadVoc(char* samplePtr, int size)
{
    VOCSample result;
    //assert(samplePtr[26] == 1); //assert first block is of sound data type
    int sampleSize = (READ_LE_U32(samplePtr + 26) >> 8) - 2;
    int frequencyDiv = *(unsigned char*)(samplePtr + 30);
    int codecId = samplePtr[31];
    char* sampleData = samplePtr + 32;
    int sampleRate = 1000000 / (256 - frequencyDiv);


    result.data = sampleData;
    result.size = sampleSize - 1;
    result.rate = sampleRate;
    //format = SF_FORMAT_VOC | SF_FORMAT_PCM_U8;
    return result;
}

typedef struct wav_header {
    // RIFF Header
    char riff_header[4]; // Contains "RIFF"
    int wav_size; // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    char wave_header[4]; // Contains "WAVE"

    // Format Header
    char fmt_header[4]; // Contains "fmt " (includes trailing space)
    int fmt_chunk_size; // Should be 16 for PCM
    short audio_format; // Should be 1 for PCM. 3 for IEEE Float
    short num_channels;
    int sample_rate;
    int byte_rate; // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    short sample_alignment; // num_channels * Bytes Per Sample
    short bit_depth; // Number of bits per sample

    // Data
    char data_header[4]; // Contains "data"
    int data_bytes; // Number of bytes in data. Number of samples * num_channels * sample byte size
    // uint8_t bytes[]; // Remainder of wave file is bytes
} wav_header;

void writeWav(VOCSample* voc ,string filename)
{
    wav_header h = {
        {'R','I','F','F'},
        voc->size + sizeof(wav_header) - 8,
        {'W','A','V','E'},
        {'f','m','t',' '},
        16,
        1,
        1,
        voc->rate,
        voc->rate,
        1,
        8,
        {'d','a','t','a'},
        voc->size
    };
    ofstream f(filename, ios::binary);
    f.write((char*)&h, sizeof(wav_header));
    f.write(voc->data, voc->size);
}

// ---------------------------------------------------------------------------
// Streaming Ogg/Vorbis encoder (mirrors music/music_extractor.cpp).
//
// Interleaved little-endian 16-bit PCM is fed in arbitrary-sized chunks;
// compressed pages are flushed as the stream is produced, so no whole-track
// buffer is ever kept in memory.
// ---------------------------------------------------------------------------
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

/**
 * Encode an 8-bit unsigned mono VOC sample to Ogg/Vorbis.
 *
 * The VOC payload from loadVoc() is unsigned 8-bit PCM; it is converted to
 * signed 16-bit PCM (in blocks, so nothing whole is buffered) and streamed to
 * `filename` through OggVorbisWriter.
 *
 * @param quality Vorbis VBR quality, 0.0 .. 1.0.
 * @return false on invalid input or if the output file could not be created.
 */
inline bool writeOgg(VOCSample* voc, string filename, float quality = 0.9f)
{
    if (!voc || !voc->data || voc->size <= 0 || voc->rate <= 0) return false;

    OggVorbisWriter writer;
    if (!writer.open(filename, 1, voc->rate, quality)) return false;

    const int kBlock = 4096; // samples converted per write() call
    std::vector<int16_t> pcm(static_cast<size_t>(std::min(voc->size, kBlock)));

    int done = 0;
    while (done < voc->size) {
        const int n = std::min(kBlock, voc->size - done);
        for (int i = 0; i < n; ++i) {
            const uint8_t u = static_cast<uint8_t>(voc->data[done + i]);
            // unsigned 8-bit -> signed 16-bit: (u - 128) << 8.
            pcm[i] = static_cast<int16_t>((static_cast<int>(u) - 128) << 8);
        }
        writer.write(reinterpret_cast<const uint8_t*>(pcm.data()),
                     static_cast<size_t>(n) * sizeof(int16_t));
        done += n;
    }

    writer.close();
    return true;
}
