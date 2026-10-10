# tiny-cdio

Минималистичная **header-only** библиотека (C++17) для чтения образов **CUE/BIN**.

Логика разбора `cue`-файла и чтения секторов повторяет подход libcdio
(`lib/driver/image/bincue.c`, `lib/iso9660/*`), но библиотека полностью
самодостаточна: нужны только заголовки стандартной библиотеки C++, никакой сборки
сторонних зависимостей.

Реализованный функционал:

1. **Чтение только cue/bin** — открытие образа и количество дорожек.
2. **Чтение аудио-дорожек** из mixed-mode CD (сырой PCM 2352 байта/сектор).
3. **Чтение файловой системы** ISO9660 (+ Joliet, если есть) из data-дорожки.
4. **Чтение содержимого файла** из этой файловой системы.

## Файлы

| Файл | Назначение |
|------|------------|
| `tiny_cdio.hpp` | Вся библиотека (класс `tinycdio::Disc` и типы). |
| `example.cpp`  | Пример использования всех четырёх возможностей. |
| `README.md`    | Эта документация. |

## Быстрый старт

```cpp
#include "tiny-cdio/tiny_cdio.hpp"

auto disc = tinycdio::Disc::open("game.cue");

// 1. Дорожки
size_t n = disc.trackCount();                 // сколько дорожек
for (const auto& t : disc.tracks()) { /* ... */ }

// 2. Аудио
for (size_t i : disc.audioTrackIndices()) {
    std::vector<uint8_t> pcm = disc.readAudioTrack(i);   // 2352 байта/сектор
    disc.saveAudioTrackWav(i, "track.wav");              // сразу в WAV
}

// 3. Файловая система
if (disc.hasFilesystem()) {
    for (const auto& e : disc.listDir("/")) { /* ... */ }
}

// 4. Файл
std::vector<uint8_t> data = disc.readFile("AITD.EXE");
```

## API (`tinycdio::Disc`)

### Открытие и общая информация

| Метод | Описание |
|-------|----------|
| `static Disc open(const std::string& cuePath)` | Открыть `.cue`; `.bin` берётся из cue (относительно каталога cue). Бросает `std::runtime_error`. |
| `size_t trackCount()` | Количество дорожек. |
| `const std::vector<TrackInfo>& tracks()` | Список дорожек. |
| `const TrackInfo& track(size_t index)` | Дорожка по 0-based индексу. |
| `int trackIndexByNumber(int number)` | Индекс по номеру из cue (с 1), `-1` если нет. |
| `bool isMixedMode()` | Есть и data-, и audio-дорожки. |
| `int dataTrackIndex()` | Индекс data-дорожки, `-1` если нет. |
| `std::vector<size_t> audioTrackIndices()` | Индексы всех аудио-дорожек. |
| `uint64_t totalSectors()` | Суммарно секторов по файлам образа. |

### Аудио и сырые сектора

| Метод | Описание |
|-------|----------|
| `std::vector<uint8_t> readTrackRaw(size_t idx, uint64_t first = 0, uint64_t count = 0)` | Сырые сектора дорожки (`sectorStride` байт на сектор). |
| `std::vector<uint8_t> readAudioTrack(size_t idx)` | Вся аудио-дорожка как PCM. |
| `std::vector<uint8_t> readAudioSectors(size_t idx, uint64_t first, uint64_t count)` | Диапазон аудио-секторов. |
| `bool saveAudioTrackWav(size_t idx, const std::string& path)` | Записать PCM в WAV (44-байтный заголовок, 16-bit stereo 44100 Hz). |

### Файловая система ISO9660

| Метод | Описание |
|-------|----------|
| `bool hasFilesystem()` | Есть ли ISO9660 в data-дорожке. |
| `std::vector<Entry> listDir(const std::string& path = "")` | Содержимое каталога (`""` или `"/"` — корень). |
| `std::optional<Entry> find(const std::string& path)` | Найти запись по пути. |
| `bool exists(const std::string& path)` | Проверить наличие пути. |
| `std::vector<uint8_t> readFile(const std::string& path)` | Прочитать файл целиком. |
| `std::string readFileText(const std::string& path)` | То же, в `std::string`. |
| `std::vector<Entry> walk()` | Рекурсивно все записи дерева. |
| `std::vector<uint8_t> readIsoBlock(uint32_t lba)` | Один логический блок (2048 байт). |
| `bool readIsoBlocks(uint32_t lba, uint32_t count, uint8_t* out)` | Несколько логических блоков. |

### Типы

```cpp
struct TrackInfo {
    int         number;        // номер дорожки из cue (с 1)
    TrackMode   mode;          // Audio / Mode1_2352 / Mode2_2352 / ...
    bool        isAudio;
    uint32_t    sectorStride;  // размер сектора в файле, байт
    uint32_t    dataOffset;    // смещение полезных данных в секторе
    uint32_t    dataSize;      // полезных байт в секторе (2048 для data)
    uint64_t    startSector;   // INDEX 01 (0-based, внутри файла)
    uint64_t    endSector;     // эксклюзивный конец
    uint64_t    sectorCount;
    bool        hasPregapIndex;
    uint64_t    index00Sector;
    size_t      fileIndex;
    std::string file;
};

struct Entry {
    std::string name;       // без версии ";1"
    std::string fullPath;   // полный путь, разделитель '/'
    bool        isDirectory;
    uint32_t    extentLba;
    uint64_t    size;       // байт
};
```

## Как это работает

- **CUE** токенизируется с учётом кавычек и отдельных строк; поддержаны
  `FILE`, `TRACK <n> <MODE>`, `INDEX 00/01 mm:ss:ff`, а метаданные
  (`REM`, `TITLE`, `CATALOG`, `ISRC`, `FLAGS`, `PREGAP`, ...) пропускаются.
  Корректно разбираются строки вида `FLAGS DCP INDEX 01 00:00:00`.
- **Границы дорожек**: начало — `INDEX 01`, конец — `INDEX 00` следующей
  дорожки (или её `INDEX 01`), либо конец файла. Учитывается pregap
  (как в `bincue.c`).
- **Чтение data-секторов**: из сырого сектора (2352/2336 байт) вырезаются
  полезные 2048 байт по `dataOffset`, зависящему от режима
  (MODE1/2352 → 16, MODE2/2352 → 24, MODE2/2336 → 8).
- **ISO9660**: primary volume descriptor в LBA 16 (`CD001`), от него — корневой
  directory record; при наличии Joliet SVD (type 2, escape `%/@`|`%/C`|`%/E`)
  используются Joliet-имена (UCS-2BE → UTF-8). Обход каталогов и extents
  реализован вручную, без Rock Ridge.

## Ограничения

- Только образы **BIN/CUE** (не NRG/TOC/ISO).
- Основной сценарий — «сырые» сектора 2352 байта; MODE1/2048, MODE2/2048,
  MODE2/2336 поддержаны, но смешивать разные размеры секторов в одном файле
  нельзя (бросается исключение).
- Rock Ridge / длинные POSIX-имена не разбираются.
- Файлы с несколькими extents (flag `0x80`) не поддерживаются.

## Интеграция

Библиотека header-only: достаточно добавить каталог `src/` в include-пути и
подключить `tiny-cdio/tiny_cdio.hpp`. В этом проекте каталог `src/` уже входит
в `ALL_INCLUDES` (см. `CMakeLists.txt`), поэтому дополнительная настройка сборки
не требуется — заголовок можно включать как `#include "tiny-cdio/tiny_cdio.hpp"`.

Пример (`example.cpp`) намеренно не добавлен в `CMakeLists.txt`; скомпилировать
его можно вручную:

```bat
cl /std:c++17 /EHsc /I.. example.cpp
```

```bash
g++ -std=c++17 -I.. example.cpp -o tiny_cdio_example
```
