#pragma once
/*
 * tiny-cdio — types and constants.
 *
 * Part of the header-only tiny-cdio library (C++17); included through
 * This file holds:
 *   - sector constants (modelled after libcdio: cdio/sector.h, cdio/iso9660.h);
 *   - public types TrackMode, TrackInfo, Entry;
 *   - small standalone helpers (little-endian writing, WAV header).
 */

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace tinycdio {

// ---------------------------------------------------------------------------
// Constants (semantically identical to libcdio: cdio/sector.h, cdio/iso9660.h)
// ---------------------------------------------------------------------------
constexpr uint32_t kRawSectorSize   = 2352;   // CDIO_CD_FRAMESIZE_RAW
constexpr uint32_t kIsoBlockSize    = 2048;   // ISO_BLOCKSIZE / CDIO_CD_FRAMESIZE
constexpr uint32_t kSyncSize        = 12;     // CDIO_CD_SYNC_SIZE
constexpr uint32_t kHeaderSize      = 4;      // CDIO_CD_HEADER_SIZE
constexpr uint32_t kSubheaderSize   = 8;      // CDIO_CD_SUBHEADER_SIZE
constexpr int      kPregapSectors   = 150;    // CDIO_PREGAP_SECTORS
constexpr uint64_t kUndefinedSector = ~static_cast<uint64_t>(0);

// ---------------------------------------------------------------------------
// Public types
// ---------------------------------------------------------------------------

/** Track sector type (corresponds to TRACK ... MODE records in .cue). */
enum class TrackMode {
    Audio,        ///< AUDIO (Red Book, 2352 bytes per sector)
    Mode1_2048,   ///< MODE1/2048 (cooked)
    Mode1_2352,   ///< MODE1/2352 (raw, data at offset 16)
    Mode2_2048,   ///< MODE2/2048 (cooked)
    Mode2_2336,   ///< MODE2/2336 (data at offset 8)
    Mode2_2352,   ///< MODE2/2352 (XA, data at offset 24)
    Unknown       ///< unrecognized
};

/** Image track description. */
struct TrackInfo {
    int        number        = 0;                ///< track number from .cue (1-based)
    TrackMode  mode          = TrackMode::Unknown;
    bool       isAudio       = false;            ///< true for AUDIO tracks
    uint32_t   sectorStride  = kRawSectorSize;   ///< sector size in the file, bytes
    uint32_t   dataOffset    = 0;                ///< offset of payload data within the sector
    uint32_t   dataSize      = kRawSectorSize;   ///< payload bytes per sector
    uint64_t   startSector   = 0;                ///< INDEX 01 (0-based, within the file)
    uint64_t   endSector     = 0;                ///< exclusive end of the track
    uint64_t   sectorCount   = 0;                ///< number of sectors in the track
    bool       hasPregapIndex= false;            ///< whether INDEX 00 is present
    uint64_t   index00Sector = 0;                ///< INDEX 00 (if present)
    size_t     fileIndex     = 0;                ///< index of the source file (files_)
    std::string file;                            ///< file name as given in .cue
};

/** ISO9660 directory entry. */
struct Entry {
    std::string name;        ///< name (without the ";1" version suffix)
    std::string fullPath;    ///< full path from the root, using '/'
    bool        isDirectory  = false;
    uint32_t    extentLba    = 0;   ///< ISO LBA of the data start
    uint64_t    size         = 0;   ///< data size, bytes
};

// ---------------------------------------------------------------------------
// Small standalone helpers
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

/** Build a 44-byte WAV header (PCM) for the given data size. */
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
