// tiny-cdio usage example.
//
// Not built automatically (see README.md, "Integration" section).
// Compile manually, for example with MSVC:
//   cl /std:c++17 /EHsc /I.. example.cpp
// or g++:
//   g++ -std=c++17 -I.. example.cpp -o tiny_cdio_example
//
// Run:
//   tiny_cdio_example path/to/disc.cue

#include "disc_iso.hpp"

#include <cstdio>
#include <string>
#include <vector>

static const char* modeName(tinycdio::TrackMode m) {
    using tinycdio::TrackMode;
    switch (m) {
        case TrackMode::Audio:      return "AUDIO";
        case TrackMode::Mode1_2048: return "MODE1/2048";
        case TrackMode::Mode1_2352: return "MODE1/2352";
        case TrackMode::Mode2_2048: return "MODE2/2048";
        case TrackMode::Mode2_2336: return "MODE2/2336";
        case TrackMode::Mode2_2352: return "MODE2/2352";
        default:                    return "UNKNOWN";
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::printf("usage: %s <disc.cue>\n", argv[0]);
        return 1;
    }

    try {
        // 1. Opening a cue/bin image and listing the tracks.
        tinycdio::DiscIso disc;
        disc.open(argv[1]);

        std::printf("CUE: %s\n", disc.cuePath().c_str());
        std::printf("Tracks: %zu, total sectors: %llu, mixed-mode: %s\n",
                    disc.trackCount(),
                    static_cast<unsigned long long>(disc.totalSectors()),
                    disc.isMixedMode() ? "yes" : "no");

        for (size_t i = 0; i < disc.trackCount(); ++i) {
            const tinycdio::TrackInfo& t = disc.track(i);
            std::printf("  #%02d idx=%zu %-12s start=%llu sectors=%llu\n",
                        t.number, i, modeName(t.mode),
                        static_cast<unsigned long long>(t.startSector),
                        static_cast<unsigned long long>(t.sectorCount));
        }

        // 2. Reading audio tracks.
        for (size_t i : disc.audioTrackIndices()) {
            std::vector<uint8_t> pcm = disc.readAudioTrack(i);
            std::printf("Audio idx=%zu: %zu PCM bytes (%.2f sec)\n",
                        i, pcm.size(), pcm.size() / 2352.0 / 75.0);

            std::string wav = "track" + std::to_string(i + 1) + ".wav";
            if (disc.saveAudioTrackWav(i, wav))
                std::printf("  -> saved: %s\n", wav.c_str());
        }

        // 3. Reading the ISO9660 file system.
        if (!disc.hasFilesystem()) {
            std::printf("ISO9660 file system not found\n");
            return 0;
        }

        std::printf("File system root:\n");
        for (const tinycdio::Entry& e : disc.listDir("/")) {
            std::printf("  %s %-24s %llu bytes\n",
                        e.isDirectory ? "[DIR ]" : "[FILE]",
                        e.name.c_str(),
                        static_cast<unsigned long long>(e.size));
        }

        // 4. Reading file contents.
        const std::string target = "AITD.EXE";
        if (disc.exists(target)) {
            std::vector<uint8_t> bytes = disc.readFile(target);
            std::printf("File %s: read %zu bytes, first 16: ", target.c_str(), bytes.size());
            for (size_t i = 0; i < bytes.size() && i < 16; ++i)
                std::printf("%02X ", bytes[i]);
            std::printf("\n");
        }

        // 5. Streaming file read (without loading the whole file into memory).
        if (disc.exists(target)) {
            auto fr = disc.openFile(target);
            uint64_t total = 0;
            std::vector<uint8_t> chunk;
            while (!(chunk = fr.readChunk(4096)).empty())
                total += chunk.size();
            std::printf("File stream %s: %llu of %llu bytes, errors: %s\n",
                        target.c_str(),
                        static_cast<unsigned long long>(total),
                        static_cast<unsigned long long>(fr.size()),
                        fr.bad() ? "yes" : "no");
        }

        // 6. Streaming audio read, sector by sector.
        for (size_t i : disc.audioTrackIndices()) {
            auto ar = disc.openAudioTrack(i);
            uint64_t total = 0;
            std::vector<uint8_t> chunk;
            while (!(chunk = ar.readChunk(2352 * 16)).empty())
                total += chunk.size();
            std::printf("Audio stream idx=%zu: %llu bytes, errors: %s\n", i,
                        static_cast<unsigned long long>(total),
                        ar.bad() ? "yes" : "no");
        }

        // Recursive traversal of the whole tree.
        std::printf("Total entries in the tree: %zu\n", disc.walk().size());
    } catch (const std::exception& ex) {
        std::printf("Error: %s\n", ex.what());
        return 1;
    }
    return 0;
}
