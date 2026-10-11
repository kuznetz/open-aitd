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
#include <exception>
#include <filesystem>
#include <map>
#include <regex>
#include <string>
#include <vector>

#include "tiny-cdio/disc_iso.hpp"

// loadVoc()/writeWav()/writeOgg() and the shared Ogg/Vorbis writer live in
// sound_extractor.h (the writer is used for the music tracks below as well).
#include "sound_extractor.h"

namespace AITDExtractor {

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

    /**
     * Extract the "book" audio from the data track of the CUE image.
     *
     * The files live in the INDARK directory of the ISO9660 file system and are
     * named XXYYZZ.VOC, where XX is the book number, YY the page number and ZZ
     * the chunk number (each two decimal digits). All chunks of a single page
     * are decoded with the helper from sound_extractor.h, concatenated in chunk
     * order into one continuous stream and written as a single 8-bit mono WAV
     * to data/audiobooks/XX.YY.wav — the numbers without leading zeros.
     *
     * @param cuePath   path to the .cue (original/GAME.INST).
     * @param outDir    output directory (data/audiobooks); created if missing.
     * @param overwrite re-convert pages whose .wav already exists.
     * @return number of pages successfully written.
     */
    inline int extractCdBookAudio(const std::string& cuePath = "original/GAME.INST",
                                  const std::string& outDir  = "data/audiobooks",
                                  bool overwrite = false) {
        tinycdio::DiscIso disc;
        disc.open(cuePath);

        if (!disc.hasFilesystem()) return 0;

        std::filesystem::create_directories(outDir);

        // Match \INDARK\XXYYZZ.VOC on any separator ('/' is used by ISO9660
        // paths). The digits are split from the right: ZZ = chunk, YY = page,
        // the remaining leading digits = book.
        static const std::regex bookVocRe(
            R"((?:^|[\\/])INDARK[\\/]([0-9]+)\.VOC$)", std::regex::icase);

        // Key: (book, page). Value: chunk number (ZZ) -> full path on the disc.
        std::map<std::pair<int, int>, std::map<int, std::string>> pages;

        for (const tinycdio::Entry& e : disc.walk()) {
            if (e.isDirectory) continue;

            std::smatch m;
            if (!std::regex_search(e.fullPath, m, bookVocRe)) continue;

            const std::string digits = m[1].str();
            if (digits.size() < 5) continue; // need at least page + chunk digits

            const int chunk = std::stoi(digits.substr(digits.size() - 2));
            const int page  = std::stoi(digits.substr(digits.size() - 4, 2));
            const int book  = std::stoi(digits.substr(0, digits.size() - 4));

            pages[{book, page}][chunk] = e.fullPath;
        }

        int written = 0;
        for (const auto& [key, chunks] : pages) {
            const std::string outPath = outDir + "/" + std::to_string(key.first) +
                                        "." + std::to_string(key.second) + ".ogg";
            if (!overwrite && std::filesystem::exists(outPath)) continue;

            // Concatenate the decoded PCM of every chunk of this page in order.
            std::vector<uint8_t> pcm;
            int rate = 22050;
            bool haveRate = false;

            for (const auto& [chunkNo, path] : chunks) {
                std::vector<uint8_t> bytes;
                try {
                    bytes = disc.readFile(path);
                } catch (const std::exception&) {
                    continue; // unreadable entry: skip it
                }
                if (bytes.size() <= 32) continue;

                VOCSample voc = loadVoc(reinterpret_cast<char*>(bytes.data()),
                                        static_cast<int>(bytes.size()));

                // Never read past the buffer that was actually loaded.
                const int available = static_cast<int>(bytes.size()) - 32;
                if (voc.size > available) voc.size = available;
                if (voc.size <= 0) continue;

                if (!haveRate) {
                    rate = voc.rate;
                    haveRate = true;
                }

                const uint8_t* begin = reinterpret_cast<const uint8_t*>(voc.data);
                pcm.insert(pcm.end(), begin, begin + voc.size);
            }

            if (pcm.empty()) continue;

            VOCSample combined;
            combined.data = reinterpret_cast<char*>(pcm.data());
            combined.size = static_cast<int>(pcm.size());
            combined.rate = rate;

            writeOgg(&combined, outPath);
            ++written;
        }
        return written;
    }

} // namespace AITDExtractor
