# tiny-cdio

A minimalist **header-only** library (C++17) for reading **CUE/BIN** images.

The logic for parsing the `cue` file and reading sectors follows the libcdio
approach (`lib/driver/image/bincue.c`, `lib/iso9660/*`), but the library is fully
self-contained: it only needs the standard C++ library headers, with no
third-party dependencies to build.

Implemented functionality:

1. **Cue/bin reading only** — opening an image and counting tracks.
2. **Reading audio tracks** from a mixed-mode CD (raw PCM, 2352 bytes/sector).
3. **Reading the file system** ISO9660 (+ Joliet, if present) from the data track.
4. **Reading file contents** from that file system.

## Files

The library is split into layers (headers are included in the order
`types.hpp → disc_base.hpp → disc_iso.hpp`):

| File | Purpose |
|------|---------|
| `types.hpp`     | Sector constants, public types (`TrackMode`, `TrackInfo`, `Entry`), the WAV helper. |
| `disc_base.hpp` | `.cue` parsing, tracks, raw sector and audio reading, WAV (`DiscBase`). |
| `disc_iso.hpp`  | ISO9660 (+ Joliet) file system: `listDir`/`readFile`/`walk` (`DiscIso`). Holds the public class and includes `disc_base.hpp`. |
| `example.cpp`   | An example using all four capabilities. |
| `README.md`     | This documentation. |

Class hierarchy: `DiscBase` (cue/audio) → `DiscIso` (ISO9660). Include
`disc_iso.hpp` in code — it pulls in `disc_base.hpp` and `types.hpp`.

## Quick start

```cpp
#include "tiny-cdio/disc_iso.hpp"

tinycdio::DiscIso disc;
disc.open("game.cue");

// 1. Tracks
size_t n = disc.trackCount();                 // number of tracks
for (const auto& t : disc.tracks()) { /* ... */ }

// 2. Audio
for (size_t i : disc.audioTrackIndices()) {
    std::vector<uint8_t> pcm = disc.readAudioTrack(i);   // 2352 bytes/sector
    disc.saveAudioTrackWav(i, "track.wav");              // straight to WAV
}

// 3. File system
if (disc.hasFilesystem()) {
    for (const auto& e : disc.listDir("/")) { /* ... */ }
}

// 4. File
std::vector<uint8_t> data = disc.readFile("AITD.EXE");
```

## API (`tinycdio::DiscIso`)

### Opening and general information

| Method | Description |
|--------|-------------|
| `void open(const std::string& cuePath)` | Open a `.cue`; the `.bin` is taken from the cue (relative to the cue directory). Throws `std::runtime_error`. |
| `size_t trackCount()` | Number of tracks. |
| `const std::vector<TrackInfo>& tracks()` | Track list. |
| `const TrackInfo& track(size_t index)` | Track by 0-based index. |
| `int trackIndexByNumber(int number)` | Index by the number from the cue (1-based), `-1` if absent. |
| `bool isMixedMode()` | Has both data and audio tracks. |
| `int dataTrackIndex()` | Index of the data track, `-1` if absent. |
| `std::vector<size_t> audioTrackIndices()` | Indices of all audio tracks. |
| `uint64_t totalSectors()` | Total sectors across the image files. |

### Audio and raw sectors

| Method | Description |
|--------|-------------|
| `std::vector<uint8_t> readTrackRaw(size_t idx, uint64_t first = 0, uint64_t count = 0)` | Raw track sectors (`sectorStride` bytes per sector). |
| `std::vector<uint8_t> readAudioTrack(size_t idx)` | The whole audio track as PCM. |
| `std::vector<uint8_t> readAudioSectors(size_t idx, uint64_t first, uint64_t count)` | A range of audio sectors. |
| `bool saveAudioTrackWav(size_t idx, const std::string& path)` | Write the PCM to WAV (44-byte header, 16-bit stereo 44100 Hz). |
| `TrackReader openTrack(size_t idx, uint64_t first = 0, uint64_t count = 0)` | Track read stream (see "Streaming reads"). |
| `TrackReader openAudioTrack(size_t idx)` | Audio track read stream. |

### ISO9660 file system

| Method | Description |
|--------|-------------|
| `bool hasFilesystem()` | Whether ISO9660 is present on the data track. |
| `std::vector<Entry> listDir(const std::string& path = "")` | Directory contents (`""` or `"/"` means the root). |
| `std::optional<Entry> find(const std::string& path)` | Find an entry by path. |
| `bool exists(const std::string& path)` | Check whether a path exists. |
| `std::vector<uint8_t> readFile(const std::string& path)` | Read a whole file. |
| `std::string readFileText(const std::string& path)` | The same, as `std::string`. |
| `FileReader openFile(const std::string& path)` | File read stream (see "Streaming reads"). |
| `std::vector<Entry> walk()` | Recursively all entries of the tree. |
| `std::vector<uint8_t> readIsoBlock(uint32_t lba)` | A single logical block (2048 bytes). |
| `bool readIsoBlocks(uint32_t lba, uint32_t count, uint8_t* out)` | Several logical blocks. |

### Streaming reads (without loading everything)

Instead of reading a whole track/file into a `std::vector`, you can obtain a
stream and process the data in chunks.

`DiscBase::TrackReader` (raw sectors/audio):

| Method | Description |
|--------|-------------|
| `size_t read(uint8_t* dst, size_t maxBytes)` | Read at most `maxBytes`; return the number of bytes (0 = end). |
| `std::vector<uint8_t> readChunk(size_t maxBytes)` | The next chunk (empty = end). |
| `bool seek(uint64_t byteOffset)` | Seek to a byte offset from the start of the range. |
| `uint64_t remaining()` / `uint64_t tell()` | Bytes remaining / already delivered. |
| `bool eof()` / `bool bad()` | End of stream / read error. |

`DiscIso::FileReader` (an ISO9660 file): the same `read`/`readChunk`/`seek`/`tell`/
`remaining`/`eof`/`bad`, plus `uint64_t size()`.

```cpp
// File stream in 4 KiB chunks.
auto fr = disc.openFile("AITD.EXE");
std::vector<uint8_t> chunk;
while (!(chunk = fr.readChunk(4096)).empty()) { /* ... */ }

// Audio stream, sector by sector.
auto ar = disc.openAudioTrack(2);
while (ar.remaining()) { auto pcm = ar.readChunk(2352); /* ... */ }
```

`readFile()`, `readAudioTrack()` and `readTrackRaw()` are implemented on top of
these streams, so the code is not duplicated.

### Types

```cpp
struct TrackInfo {
    int         number;        // track number from the cue (1-based)
    TrackMode   mode;          // Audio / Mode1_2352 / Mode2_2352 / ...
    bool        isAudio;
    uint32_t    sectorStride;  // sector size in the file, bytes
    uint32_t    dataOffset;    // offset of the payload within the sector
    uint32_t    dataSize;      // payload bytes per sector (2048 for data)
    uint64_t    startSector;   // INDEX 01 (0-based, within the file)
    uint64_t    endSector;     // exclusive end
    uint64_t    sectorCount;
    bool        hasPregapIndex;
    uint64_t    index00Sector;
    size_t      fileIndex;
    std::string file;
};

struct Entry {
    std::string name;       // without the ";1" version suffix
    std::string fullPath;   // full path, '/' separator
    bool        isDirectory;
    uint32_t    extentLba;
    uint64_t    size;       // bytes
};
```

## How it works

- **CUE** is tokenized with awareness of quotes and individual lines; `FILE`,
  `TRACK <n> <MODE>`, `INDEX 00/01 mm:ss:ff` are supported, while metadata
  (`REM`, `TITLE`, `CATALOG`, `ISRC`, `FLAGS`, `PREGAP`, ...) is skipped.
  Lines like `FLAGS DCP INDEX 01 00:00:00` are parsed correctly.
- **Track boundaries**: the start is `INDEX 01`, the end is the next track's
  `INDEX 00` (or its `INDEX 01`), otherwise the end of the file. The pregap is
  taken into account (as in `bincue.c`).
- **Reading data sectors**: the useful 2048 bytes are carved out of the raw
  sector (2352/2336 bytes) using `dataOffset`, which depends on the mode
  (MODE1/2352 → 16, MODE2/2352 → 24, MODE2/2336 → 8).
- **ISO9660**: the primary volume descriptor at LBA 16 (`CD001`) gives the root
  directory record; if a Joliet SVD is present (type 2, escape
  `%/@`|`%/C`|`%/E`), Joliet names are used (UCS-2BE → UTF-8). Directory and
  extent traversal is implemented manually, without Rock Ridge.

## Limitations

- Only **BIN/CUE** images (not NRG/TOC/ISO).
- The main scenario is "raw" 2352-byte sectors; MODE1/2048, MODE2/2048 and
  MODE2/2336 are supported, but different sector sizes cannot be mixed within a
  single file (an exception is thrown).
- Rock Ridge / long POSIX names are not parsed.
- Files with multiple extents (flag `0x80`) are not supported.

## Integration

The library is header-only: just add the `src/` directory to the include paths
and include `tiny-cdio/disc_iso.hpp`. In this project the `src/` directory is
already part of `ALL_INCLUDES` (see `CMakeLists.txt`), so no extra build
configuration is required — the header can be included as
`#include "tiny-cdio/disc_iso.hpp"`.

The example (`example.cpp`) is intentionally not added to `CMakeLists.txt`; it
can be compiled manually:

```bat
cl /std:c++17 /EHsc /I.. example.cpp
```

```bash
g++ -std=c++17 -I.. example.cpp -o tiny_cdio_example
```
