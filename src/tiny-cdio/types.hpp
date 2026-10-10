#pragma once
/*
 * tiny-cdio — типы и константы.
 *
 * Часть header-only библиотеки tiny-cdio (C++17); подключается через
 * "tiny_cdio.hpp". Здесь собраны:
 *   - константы секторов (по мотивам libcdio: cdio/sector.h, cdio/iso9660.h);
 *   - публичные типы TrackMode, TrackInfo, Entry;
 *   - мелкие независимые хелперы (little-endian запись, WAV-заголовок).
 */

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace tinycdio {

// ---------------------------------------------------------------------------
// Константы (совпадают по смыслу с libcdio: cdio/sector.h, cdio/iso9660.h)
// ---------------------------------------------------------------------------
constexpr uint32_t kRawSectorSize   = 2352;   // CDIO_CD_FRAMESIZE_RAW
constexpr uint32_t kIsoBlockSize    = 2048;   // ISO_BLOCKSIZE / CDIO_CD_FRAMESIZE
constexpr uint32_t kSyncSize        = 12;     // CDIO_CD_SYNC_SIZE
constexpr uint32_t kHeaderSize      = 4;      // CDIO_CD_HEADER_SIZE
constexpr uint32_t kSubheaderSize   = 8;      // CDIO_CD_SUBHEADER_SIZE
constexpr int      kPregapSectors   = 150;    // CDIO_PREGAP_SECTORS
constexpr uint64_t kUndefinedSector = ~static_cast<uint64_t>(0);

// ---------------------------------------------------------------------------
// Публичные типы
// ---------------------------------------------------------------------------

/** Тип сектора дорожки (соответствует записям TRACK ... MODE в .cue). */
enum class TrackMode {
    Audio,        ///< AUDIO (Red Book, 2352 байт на сектор)
    Mode1_2048,   ///< MODE1/2048 (cooked)
    Mode1_2352,   ///< MODE1/2352 (raw, данные с offset 16)
    Mode2_2048,   ///< MODE2/2048 (cooked)
    Mode2_2336,   ///< MODE2/2336 (данные с offset 8)
    Mode2_2352,   ///< MODE2/2352 (XA, данные с offset 24)
    Unknown       ///< не распознан
};

/** Описание дорожки образа. */
struct TrackInfo {
    int        number        = 0;                ///< номер дорожки из .cue (с 1)
    TrackMode  mode          = TrackMode::Unknown;
    bool       isAudio       = false;            ///< true для AUDIO-дорожек
    uint32_t   sectorStride  = kRawSectorSize;   ///< размер сектора в файле, байт
    uint32_t   dataOffset    = 0;                ///< смещение полезных данных в секторе
    uint32_t   dataSize      = kRawSectorSize;   ///< сколько полезных байт в секторе
    uint64_t   startSector   = 0;                ///< INDEX 01 (0-based, внутри файла)
    uint64_t   endSector     = 0;                ///< эксклюзивный конец дорожки
    uint64_t   sectorCount   = 0;                ///< число секторов в дорожке
    bool       hasPregapIndex= false;            ///< задан ли INDEX 00
    uint64_t   index00Sector = 0;                ///< INDEX 00 (если есть)
    size_t     fileIndex     = 0;                ///< индекс файла-источника (files_)
    std::string file;                            ///< имя файла, как указано в .cue
};

/** Запись каталога ISO9660. */
struct Entry {
    std::string name;        ///< имя (без версии ";1")
    std::string fullPath;    ///< полный путь от корня, с '/'
    bool        isDirectory  = false;
    uint32_t    extentLba    = 0;   ///< ISO LBA начала данных
    uint64_t    size         = 0;   ///< размер данных, байт
};

// ---------------------------------------------------------------------------
// Мелкие независимые хелперы
// ---------------------------------------------------------------------------
namespace detail {

inline void putLe16(std::vector<uint8_t>& v, uint16_t x) {
    v.push_back(static_cast<uint8_t>(x & 0xFF));
    v.push_back(static_cast<uint8_t>((x >> 8) & 0xFF));
}

inline void putLe32(std::vector<uint8_t>& v, uint32_t x) {
    v.push_back(static_cast<uint8_t>(x & 0xFF));
    v.push_back(static_cast<uint8_t>((x >> 8) & 0xFF));
    v.push_back(static_cast<uint8_t>((x >> 16) & 0xFF));
    v.push_back(static_cast<uint8_t>((x >> 24) & 0xFF));
}

/** Собрать 44-байтный WAV-заголовок (PCM) под указанный объём данных. */
inline std::vector<uint8_t> makeWavHeader(uint64_t dataBytes,
                                          uint32_t sampleRate = 44100,
                                          uint16_t channels = 2,
                                          uint16_t bits = 16) {
    std::vector<uint8_t> h;
    uint32_t dataSz = static_cast<uint32_t>(dataBytes);
    uint16_t blockAlign = static_cast<uint16_t>(channels * bits / 8);
    uint32_t byteRate = sampleRate * blockAlign;

    h.insert(h.end(), {'R', 'I', 'F', 'F'});
    putLe32(h, 36 + dataSz);
    h.insert(h.end(), {'W', 'A', 'V', 'E'});
    h.insert(h.end(), {'f', 'm', 't', ' '});
    putLe32(h, 16);          // fmt chunk size
    putLe16(h, 1);           // PCM
    putLe16(h, channels);
    putLe32(h, sampleRate);
    putLe32(h, byteRate);
    putLe16(h, blockAlign);
    putLe16(h, bits);
    h.insert(h.end(), {'d', 'a', 't', 'a'});
    putLe32(h, dataSz);
    return h;
}

} // namespace detail

} // namespace tinycdio
