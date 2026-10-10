#pragma once
/*
 * tiny-cdio — минималистичная header-only библиотека для чтения образов CUE/BIN.
 *
 * Создана на основе идей/логики libcdio (lib/driver/image/bincue.c,
 * lib/iso9660/*), но без внешних зависимостей и сборки: только стандартная
 * библиотека C++17.
 *
 * Поддерживаемый функционал (задача):
 *   1. Чтение только cue/bin — узнать количество дорожек.
 *   2. Чтение аудио-дорожек из mixed-mode CD (cue/bin).
 *   3. Чтение файловой системы (ISO9660 + опционально Joliet) из mixed-mode CD.
 *   4. Чтение содержимого файла из этой файловой системы.
 *
 * Ограничения (сознательно, ради «tiny»):
 *   - образ должен быть BIN/CUE (не NRG/TOC/ISO);
 *   - «сырые» сектора 2352 байта (MODE1/2352, MODE2/2352, AUDIO) — основной
 *     сценарий; дополнительно поддержаны MODE1/2048, MODE2/2048, MODE2/2336;
 *   - ISO9660: primary volume descriptor + (если есть) Joliet; Rock Ridge не
 *     разбирается (длинные POSIX-имена игнорируются, используется ISO/Joliet имя).
 *
 * Структура (разбита на 4 заголовка):
 *   - types.hpp   — константы и публичные типы (TrackMode, TrackInfo, Entry);
 *   - cue.hpp     — разбор .cue, дорожки, сырые сектора, аудио (DiscBase);
 *   - iso9660.hpp — файловая система ISO9660/Joliet (DiscIso);
 *   - tiny_cdio.hpp (этот файл) — umbrella и публичный фасад Disc.
 *
 * Пример использования:
 *   #include "tiny-cdio/tiny_cdio.hpp"
 *   auto disc = tinycdio::Disc::open("game.cue");
 *   for (size_t i = 0; i < disc.trackCount(); ++i) { ... }
 *   auto pcm = disc.readAudioTrack(2);                 // сырой PCM
 *   disc.saveAudioTrackWav(2, "track02.wav");          // PCM -> WAV
 *   for (auto& e : disc.listDir("/")) { ... }          // корень ISO9660
 *   auto bytes = disc.readFile("AITD.EXE");            // содержимое файла
 */

#include "types.hpp"
#include "cue.hpp"
#include "iso9660.hpp"

#include <string>

namespace tinycdio {

// ---------------------------------------------------------------------------
// Публичный фасад: Disc = DiscBase (cue/аудио) + DiscIso (ISO9660)
// ---------------------------------------------------------------------------
class Disc : public DiscIso {
public:
    Disc() = default;
    Disc(const Disc&) = delete;
    Disc& operator=(const Disc&) = delete;
    Disc(Disc&&) = default;
    Disc& operator=(Disc&&) = default;

    /**
     * Открыть образ: указывается путь к .cue; .bin-файлы берутся из .cue
     * (относительно каталога .cue).
     * @throws std::runtime_error при ошибке разбора/открытия.
     */
    static Disc open(const std::string& cuePath) {
        Disc d;
        d.init(cuePath);
        return d;
    }
};

} // namespace tinycdio
