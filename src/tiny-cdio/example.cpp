// Пример использования tiny-cdio.
//
// Не собирается автоматически (см. README.md, раздел «Интеграция»).
// Компиляция вручную, например MSVC:
//   cl /std:c++17 /EHsc /I.. example.cpp
// или g++:
//   g++ -std=c++17 -I.. example.cpp -o tiny_cdio_example
//
// Запуск:
//   tiny_cdio_example path/to/disc.cue

#include "tiny_cdio.hpp"

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
        // 1. Открытие образа cue/bin и список дорожек.
        tinycdio::Disc disc = tinycdio::Disc::open(argv[1]);

        std::printf("CUE: %s\n", disc.cuePath().c_str());
        std::printf("Трек-дорог: %zu, всего секторов: %llu, mixed-mode: %s\n",
                    disc.trackCount(),
                    static_cast<unsigned long long>(disc.totalSectors()),
                    disc.isMixedMode() ? "да" : "нет");

        for (size_t i = 0; i < disc.trackCount(); ++i) {
            const tinycdio::TrackInfo& t = disc.track(i);
            std::printf("  #%02d idx=%zu %-12s start=%llu sectors=%llu\n",
                        t.number, i, modeName(t.mode),
                        static_cast<unsigned long long>(t.startSector),
                        static_cast<unsigned long long>(t.sectorCount));
        }

        // 2. Чтение аудио-дорожек.
        for (size_t i : disc.audioTrackIndices()) {
            std::vector<uint8_t> pcm = disc.readAudioTrack(i);
            std::printf("Аудио idx=%zu: %zu байт PCM (%.2f сек.)\n",
                        i, pcm.size(), pcm.size() / 2352.0 / 75.0);

            std::string wav = "track" + std::to_string(i + 1) + ".wav";
            if (disc.saveAudioTrackWav(i, wav))
                std::printf("  -> сохранено: %s\n", wav.c_str());
        }

        // 3. Чтение файловой системы ISO9660.
        if (!disc.hasFilesystem()) {
            std::printf("Файловая система ISO9660 не найдена\n");
            return 0;
        }

        std::printf("Корень файловой системы:\n");
        for (const tinycdio::Entry& e : disc.listDir("/")) {
            std::printf("  %s %-24s %llu байт\n",
                        e.isDirectory ? "[DIR ]" : "[FILE]",
                        e.name.c_str(),
                        static_cast<unsigned long long>(e.size));
        }

        // 4. Чтение содержимого файла.
        const std::string target = "AITD.EXE";
        if (disc.exists(target)) {
            std::vector<uint8_t> bytes = disc.readFile(target);
            std::printf("Файл %s: прочитано %zu байт, первые 16: ", target.c_str(), bytes.size());
            for (size_t i = 0; i < bytes.size() && i < 16; ++i)
                std::printf("%02X ", bytes[i]);
            std::printf("\n");
        }

        // 5. Потоковое чтение файла (без загрузки целиком в память).
        if (disc.exists(target)) {
            auto fr = disc.openFile(target);
            uint64_t total = 0;
            std::vector<uint8_t> chunk;
            while (!(chunk = fr.readChunk(4096)).empty())
                total += chunk.size();
            std::printf("Поток файла %s: %llu из %llu байт, ошибок: %s\n",
                        target.c_str(),
                        static_cast<unsigned long long>(total),
                        static_cast<unsigned long long>(fr.size()),
                        fr.bad() ? "да" : "нет");
        }

        // 6. Потоковое чтение аудио по секторам.
        for (size_t i : disc.audioTrackIndices()) {
            auto ar = disc.openAudioTrack(i);
            uint64_t total = 0;
            std::vector<uint8_t> chunk;
            while (!(chunk = ar.readChunk(2352 * 16)).empty())
                total += chunk.size();
            std::printf("Поток аудио idx=%zu: %llu байт, ошибок: %s\n", i,
                        static_cast<unsigned long long>(total),
                        ar.bad() ? "да" : "нет");
        }

        // Рекурсивный обход всего дерева.
        std::printf("Всего записей в дереве: %zu\n", disc.walk().size());
    } catch (const std::exception& ex) {
        std::printf("Ошибка: %s\n", ex.what());
        return 1;
    }
    return 0;
}
