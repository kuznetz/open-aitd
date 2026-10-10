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
 * Пример использования:
 *   #include "tiny-cdio/tiny_cdio.hpp"
 *   auto disc = tinycdio::Disc::open("game.cue");
 *   for (size_t i = 0; i < disc.trackCount(); ++i) { ... }
 *   auto pcm = disc.readAudioTrack(2);                 // сырой PCM
 *   disc.saveAudioTrackWav(2, "track02.wav");          // PCM -> WAV
 *   for (auto& e : disc.listDir("/")) { ... }          // корень ISO9660
 *   auto bytes = disc.readFile("AITD.EXE");            // содержимое файла
 */

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cctype>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <iterator>
#include <optional>
#include <functional>
#include <algorithm>
#include <stdexcept>

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
// Класс Disc — образ CUE/BIN + доступ к дорожкам и ISO9660
// ---------------------------------------------------------------------------
class Disc {
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

    // --- общая информация ---------------------------------------------------

    const std::string& cuePath() const { return cuePath_; }

    size_t trackCount() const { return tracks_.size(); }
    const std::vector<TrackInfo>& tracks() const { return tracks_; }
    const TrackInfo& track(size_t index) const { return tracks_.at(index); }

    /** Индекс дорожки по её номеру из .cue (с 1); -1 если нет. */
    int trackIndexByNumber(int number) const {
        for (size_t i = 0; i < tracks_.size(); ++i)
            if (tracks_[i].number == number) return static_cast<int>(i);
        return -1;
    }

    /** true, если образ содержит и данные, и аудио (mixed-mode). */
    bool isMixedMode() const {
        bool data = false, audio = false;
        for (auto& t : tracks_) { if (t.isAudio) audio = true; else data = true; }
        return data && audio;
    }

    /** Индекс первой дорожки с данными или -1. */
    int dataTrackIndex() const { return dataTrackIndex_; }

    /** Индексы всех аудио-дорожек. */
    std::vector<size_t> audioTrackIndices() const {
        std::vector<size_t> v;
        for (size_t i = 0; i < tracks_.size(); ++i) if (tracks_[i].isAudio) v.push_back(i);
        return v;
    }

    /** Суммарное число секторов (сырых) по всем файлам образа. */
    uint64_t totalSectors() const {
        uint64_t n = 0;
        for (auto& f : files_) n += f->sectorCount;
        return n;
    }

    // --- чтение дорожек -----------------------------------------------------

    /** Прочитать сырые сектора дорожки (sectorStride байт на сектор). */
    std::vector<uint8_t> readTrackRaw(size_t trackIndex, uint64_t firstSector = 0,
                                      uint64_t count = 0) const {
        const TrackInfo& t = tracks_.at(trackIndex);
        FileHandle& f = *files_.at(t.fileIndex);
        if (t.sectorStride != f.stride)
            throw std::runtime_error("tiny-cdio: track/file sector stride mismatch");
        uint64_t n = count ? count : (t.sectorCount > firstSector ? t.sectorCount - firstSector : 0);
        std::vector<uint8_t> out(static_cast<size_t>(n) * t.sectorStride);
        if (n && !readRawSectors(f, t.startSector + firstSector, n, out.data()))
            throw std::runtime_error("tiny-cdio: read error (raw track)");
        return out;
    }

    /**
     * Прочитать аудио-дорожку целиком как сырой PCM (2352 байта на сектор,
     * 16-bit stereo 44100 Hz).
     * @throws std::runtime_error если дорожка не аудио.
     */
    std::vector<uint8_t> readAudioTrack(size_t trackIndex) const {
        const TrackInfo& t = tracks_.at(trackIndex);
        if (!t.isAudio)
            throw std::runtime_error("tiny-cdio: track is not audio");
        return readTrackRaw(trackIndex);
    }

    /** Прочитать count аудио-секторов начиная с firstSector дорожки. */
    std::vector<uint8_t> readAudioSectors(size_t trackIndex, uint64_t firstSector,
                                          uint64_t count) const {
        const TrackInfo& t = tracks_.at(trackIndex);
        if (!t.isAudio)
            throw std::runtime_error("tiny-cdio: track is not audio");
        return readTrackRaw(trackIndex, firstSector, count);
    }

    /** Сохранить аудио-дорожку в WAV (44-байтный заголовок + PCM). */
    bool saveAudioTrackWav(size_t trackIndex, const std::string& outPath) const {
        auto pcm = readAudioTrack(trackIndex);
        std::vector<uint8_t> header = makeWavHeader(pcm.size());
        std::ofstream out(outPath, std::ios::binary);
        if (!out) return false;
        out.write(reinterpret_cast<const char*>(header.data()),
                  static_cast<std::streamsize>(header.size()));
        out.write(reinterpret_cast<const char*>(pcm.data()),
                  static_cast<std::streamsize>(pcm.size()));
        return out.good();
    }

    // --- чтение данных (ISO-секторов) --------------------------------------

    /** Прочитать один логический блок ISO9660 (2048 байт полезных данных). */
    std::vector<uint8_t> readIsoBlock(uint32_t lba) const {
        std::vector<uint8_t> out(kIsoBlockSize);
        if (!readIsoBlocks(lba, 1, out.data()))
            throw std::runtime_error("tiny-cdio: read error (iso block)");
        return out;
    }

    // --- файловая система ISO9660 ------------------------------------------

    /** Есть ли доступная файловая система ISO9660 в data-дорожке. */
    bool hasFilesystem() const { return ensureIso(); }

    /** Вывести содержимое каталога (path: "" или "/" — корень). */
    std::vector<Entry> listDir(const std::string& path = std::string()) const {
        if (!ensureIso()) return {};
        auto dir = resolve(path);
        if (!dir || !dir->isDirectory) return {};
        return listDirOf(*dir);
    }

    /** Найти запись по пути. */
    std::optional<Entry> find(const std::string& path) const {
        if (!ensureIso()) return std::nullopt;
        return resolve(path);
    }

    bool exists(const std::string& path) const {
        auto e = find(path);
        return e.has_value();
    }

    /** Прочитать содержимое файла по пути. */
    std::vector<uint8_t> readFile(const std::string& path) const {
        auto e = find(path);
        if (!e) throw std::runtime_error("tiny-cdio: file not found: " + path);
        if (e->isDirectory) throw std::runtime_error("tiny-cdio: is a directory: " + path);
        return readExtent(e->extentLba, e->size);
    }

    /** Прочитать содержимое файла в строку. */
    std::string readFileText(const std::string& path) const {
        auto bytes = readFile(path);
        return std::string(bytes.begin(), bytes.end());
    }

    /** Рекурсивно обойти всё дерево; возвращает записи файлов и каталогов. */
    std::vector<Entry> walk() const {
        std::vector<Entry> out;
        if (!ensureIso()) return out;
        Entry root = makeRootEntry();
        std::function<void(const Entry&)> rec = [&](const Entry& d) {
            for (auto& e : listDirOf(d)) {
                out.push_back(e);
                if (e.isDirectory) rec(e);
            }
        };
        rec(root);
        return out;
    }

    // --- низкоуровневый доступ ---------------------------------------------

    /** Прочитать count логических блоков ISO9660 (2048 байт каждый) в out. */
    bool readIsoBlocks(uint32_t lba, uint32_t count, uint8_t* out) const {
        if (dataTrackIndex_ < 0) return false;
        const TrackInfo& t = tracks_[dataTrackIndex_];
        if (t.dataOffset + kIsoBlockSize > t.sectorStride) return false;
        FileHandle& f = *files_[t.fileIndex];
        std::vector<uint8_t> raw(static_cast<size_t>(count) * t.sectorStride);
        if (!readRawSectors(f, t.startSector + lba, count, raw.data())) return false;
        for (uint32_t i = 0; i < count; ++i)
            std::memcpy(out + static_cast<size_t>(i) * kIsoBlockSize,
                        raw.data() + static_cast<size_t>(i) * t.sectorStride + t.dataOffset,
                        kIsoBlockSize);
        return true;
    }

private:
    // -----------------------------------------------------------------------
    // Внутренние структуры/хелперы
    // -----------------------------------------------------------------------
    struct FileHandle {
        std::string   path;
        mutable std::fstream stream;    ///< mutable: чтение идёт из const-методов
        uint32_t      stride      = 0;  ///< байт на сектор в этом файле
        uint64_t      byteSize    = 0;
        uint64_t      sectorCount = 0;
    };

    struct CueFile {
        std::string name;
    };

    struct CueToken {
        std::string text;
        int         line = 0;
    };

    std::string cuePath_;
    std::vector<TrackInfo>                  tracks_;
    std::vector<CueFile>                    cueFiles_;
    std::vector<std::unique_ptr<FileHandle>> files_;
    int dataTrackIndex_ = -1;

    // Кэш ISO9660
    mutable bool     isoTried_    = false;
    mutable bool     isoOk_       = false;
    mutable bool     joliet_      = false;
    mutable uint32_t blockSize_   = kIsoBlockSize;
    mutable uint32_t rootExtent_  = 0;
    mutable uint64_t rootSize_    = 0;

    // --- разбор CUE --------------------------------------------------------

    static std::string toUpper(std::string s) {
        for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return s;
    }

    static std::vector<CueToken> tokenizeCue(const std::string& src) {
        std::vector<CueToken> out;
        int line = 1;
        size_t i = 0, n = src.size();
        while (i < n) {
            char c = src[i];
            if (c == '\n') { ++line; ++i; continue; }
            if (c == '\r' || c == ' ' || c == '\t') { ++i; continue; }
            if (c == '"') {
                ++i;
                std::string t;
                while (i < n && src[i] != '"') { if (src[i] == '\n') ++line; t += src[i++]; }
                if (i < n) ++i;
                out.push_back({t, line});
                continue;
            }
            std::string t;
            while (i < n) {
                char d = src[i];
                if (d == ' ' || d == '\t' || d == '\r' || d == '\n') break;
                t += d; ++i;
            }
            out.push_back({t, line});
        }
        return out;
    }

    static bool parseMsf(const std::string& s, uint64_t& lba) {
        unsigned m = 0, sec = 0, fr = 0;
        if (std::sscanf(s.c_str(), "%u:%u:%u", &m, &sec, &fr) != 3) return false;
        if (sec >= 60 || fr >= 75) return false;
        lba = (static_cast<uint64_t>(m) * 60 + sec) * 75 + fr;
        return true;
    }

    static bool applyTrackMode(const std::string& m, TrackInfo& t) {
        if (m == "AUDIO") {
            t.mode = TrackMode::Audio; t.isAudio = true;
            t.sectorStride = kRawSectorSize; t.dataOffset = 0; t.dataSize = kRawSectorSize;
            return true;
        }
        if (m == "MODE1/2048") {
            t.mode = TrackMode::Mode1_2048; t.sectorStride = 2048; t.dataOffset = 0; t.dataSize = 2048;
            return true;
        }
        if (m == "MODE1/2352") {
            t.mode = TrackMode::Mode1_2352; t.sectorStride = kRawSectorSize;
            t.dataOffset = kSyncSize + kHeaderSize; t.dataSize = kIsoBlockSize;
            return true;
        }
        if (m == "MODE2/2048") {
            t.mode = TrackMode::Mode2_2048; t.sectorStride = 2048; t.dataOffset = 0; t.dataSize = 2048;
            return true;
        }
        if (m == "MODE2/2336") {
            t.mode = TrackMode::Mode2_2336; t.sectorStride = 2336;
            t.dataOffset = kSubheaderSize; t.dataSize = kIsoBlockSize;
            return true;
        }
        if (m == "MODE2/2324") { // best-effort: XA form2
            t.mode = TrackMode::Mode2_2352; t.sectorStride = kRawSectorSize;
            t.dataOffset = kSubheaderSize; t.dataSize = 2324;
            return true;
        }
        if (m == "MODE2/2352") {
            t.mode = TrackMode::Mode2_2352; t.sectorStride = kRawSectorSize;
            t.dataOffset = kSyncSize + kHeaderSize + kSubheaderSize; t.dataSize = kIsoBlockSize;
            return true;
        }
        t.mode = TrackMode::Unknown;
        return false;
    }

    static std::string readTextFile(const std::string& path) {
        std::ifstream in(path, std::ios::binary);
        if (!in) return {};
        std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        return s;
    }

    void init(const std::string& cuePath) {
        cuePath_ = cuePath;
        std::string src = readTextFile(cuePath);
        if (src.empty())
            throw std::runtime_error("tiny-cdio: cannot read cue file: " + cuePath);

        auto tok = tokenizeCue(src);
        size_t curFile = 0;
        bool haveFile = false;

        for (size_t i = 0; i < tok.size();) {
            std::string kw = toUpper(tok[i].text);

            if (kw == "FILE") {
                CueFile cf;
                if (i + 1 < tok.size()) cf.name = tok[i + 1].text;
                cueFiles_.push_back(cf);
                curFile = cueFiles_.size() - 1;
                haveFile = true;
                i += 2;
                if (i < tok.size()) {
                    std::string u = toUpper(tok[i].text);
                    if (u == "BINARY" || u == "MOTOROLA" || u == "WAVE" ||
                        u == "AUDIO" || u == "MP3" || u == "AIFF")
                        ++i;
                }
                continue;
            }
            if (kw == "TRACK") {
                if (i + 2 >= tok.size()) break;
                TrackInfo t;
                t.number = std::atoi(tok[i + 1].text.c_str());
                t.fileIndex = curFile;
                t.startSector = kUndefinedSector;   // пока не найден INDEX 01
                if (!applyTrackMode(toUpper(tok[i + 2].text), t))
                    throw std::runtime_error("tiny-cdio: unknown track mode: " + tok[i + 2].text);
                tracks_.push_back(t);
                i += 3;
                continue;
            }
            if (kw == "INDEX") {
                if (i + 2 >= tok.size()) break;
                int idx = std::atoi(tok[i + 1].text.c_str());
                uint64_t lba = 0;
                if (parseMsf(tok[i + 2].text, lba) && !tracks_.empty()) {
                    TrackInfo& t = tracks_.back();
                    if (idx == 1) {
                        t.startSector = lba;
                    } else if (idx == 0) {
                        t.hasPregapIndex = true;
                        t.index00Sector = lba;
                    }
                }
                i += 3;
                continue;
            }
            if (kw == "PREGAP" || kw == "POSTGAP") { i += 2; continue; }

            // REM — комментарий: пропускаем всю строку целиком.
            if (kw == "REM") {
                int ln = tok[i].line;
                while (i < tok.size() && tok[i].line == ln) ++i;
                continue;
            }

            // FLAGS <PRE|DCP|4CH|SCMS>... — пропускаем только сами флаги,
            // чтобы поддержать строки вида "FLAGS DCP INDEX 01 00:00:00".
            if (kw == "FLAGS") {
                ++i;
                while (i < tok.size()) {
                    std::string u = toUpper(tok[i].text);
                    if (u == "PRE" || u == "DCP" || u == "4CH" || u == "SCMS") { ++i; continue; }
                    break;
                }
                continue;
            }

            // CD-TEXT/прочие метаданные: ключевое слово + один аргумент.
            if (kw == "CATALOG" || kw == "CDTEXTFILE" || kw == "ISRC" ||
                kw == "TITLE" || kw == "PERFORMER" || kw == "SONGWRITER" ||
                kw == "MESSAGE" || kw == "ARRANGER" || kw == "COMPOSER") {
                i += 2;
                continue;
            }

            // Неизвестное слово — пропускаем один токен.
            ++i;
        }

        if (!haveFile) throw std::runtime_error("tiny-cdio: no FILE statement in cue");
        if (tracks_.empty()) throw std::runtime_error("tiny-cdio: no TRACK statement in cue");

        // Имя файла дорожки + проверка индексов.
        for (auto& t : tracks_) {
            if (t.fileIndex >= cueFiles_.size())
                throw std::runtime_error("tiny-cdio: TRACK before FILE in cue");
            t.file = cueFiles_[t.fileIndex].name;
            if (t.startSector == kUndefinedSector)
                throw std::runtime_error("tiny-cdio: track without INDEX 01");
        }

        // Открываем файлы образа.
        std::string base = dirName(cuePath);
        files_.clear();
        for (auto& cf : cueFiles_) {
            auto h = std::make_unique<FileHandle>();
            h->path = joinOsPath(base, cf.name);
            h->stream.open(h->path, std::ios::in | std::ios::binary);
            if (!h->stream)
                throw std::runtime_error("tiny-cdio: cannot open image file: " + h->path);
            h->stream.seekg(0, std::ios::end);
            std::streamoff sz = h->stream.tellg();
            h->stream.seekg(0, std::ios::beg);
            h->byteSize = (sz > 0) ? static_cast<uint64_t>(sz) : 0;
            files_.push_back(std::move(h));
        }

        // Stride файла берём из первой ссылающейся дорожки; проверяем единство.
        for (auto& t : tracks_) {
            FileHandle& f = *files_[t.fileIndex];
            if (f.stride == 0) f.stride = t.sectorStride;
            else if (f.stride != t.sectorStride)
                throw std::runtime_error("tiny-cdio: inconsistent sector size in image file");
        }
        for (auto& f : files_) {
            if (f->stride == 0) f->stride = kRawSectorSize;
            f->sectorCount = f->byteSize / f->stride;
        }

        // Границы/длины дорожек.
        for (size_t i = 0; i < tracks_.size(); ++i) {
            TrackInfo& t = tracks_[i];
            uint64_t end;
            if (i + 1 < tracks_.size() && tracks_[i + 1].fileIndex == t.fileIndex) {
                const TrackInfo& nx = tracks_[i + 1];
                end = (nx.hasPregapIndex && nx.index00Sector >= t.startSector)
                          ? nx.index00Sector : nx.startSector;
            } else {
                end = files_[t.fileIndex]->sectorCount;
            }
            if (end < t.startSector) end = t.startSector;
            t.endSector = end;
            t.sectorCount = end - t.startSector;
        }

        // Первая дорожка с данными.
        dataTrackIndex_ = -1;
        for (size_t i = 0; i < tracks_.size(); ++i) {
            if (!tracks_[i].isAudio) { dataTrackIndex_ = static_cast<int>(i); break; }
        }
    }

    // --- чтение секторов ---------------------------------------------------

    static bool readRawSectors(FileHandle& f, uint64_t sector, uint64_t count,
                               uint8_t* out) {
        f.stream.clear();
        f.stream.seekg(static_cast<std::streamoff>(sector * f.stride), std::ios::beg);
        if (!f.stream) return false;
        std::streamsize want = static_cast<std::streamsize>(count * f.stride);
        f.stream.read(reinterpret_cast<char*>(out), want);
        return f.stream.gcount() == want;
    }

    // --- хелперы путей ------------------------------------------------------

    static bool isAbsolutePath(const std::string& p) {
        if (p.empty()) return false;
        if (p[0] == '/' || p[0] == '\\') return true;
        if (p.size() >= 2 && std::isalpha(static_cast<unsigned char>(p[0])) && p[1] == ':')
            return true;
        return false;
    }

    static std::string dirName(const std::string& p) {
        size_t s = p.find_last_of("/\\");
        return (s == std::string::npos) ? std::string() : p.substr(0, s);
    }

    static std::string joinOsPath(const std::string& dir, const std::string& name) {
        if (name.empty() || isAbsolutePath(name) || dir.empty()) return name;
        char last = dir.back();
        if (last == '/' || last == '\\') return dir + name;
        return dir + "/" + name;
    }

    // --- ISO9660 -----------------------------------------------------------

    static uint16_t le16(const uint8_t* p) {
        return static_cast<uint16_t>(p[0] | (p[1] << 8));
    }
    static uint32_t le32(const uint8_t* p) {
        return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
               (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
    }

    Entry makeRootEntry() const {
        Entry r;
        r.name = "/";
        r.fullPath = "/";
        r.isDirectory = true;
        r.extentLba = rootExtent_;
        r.size = rootSize_;
        return r;
    }

    bool ensureIso() const {
        if (isoTried_) return isoOk_;
        isoTried_ = true;
        if (dataTrackIndex_ < 0) return false;

        auto pvd = readIsoBlockSafe(16);
        if (pvd.size() < 2048) return false;
        if (pvd[0] != 1 || std::memcmp(&pvd[1], "CD001", 5) != 0) return false;

        blockSize_ = le16(&pvd[128]);
        if (blockSize_ == 0) blockSize_ = kIsoBlockSize;
        rootExtent_ = le32(&pvd[156 + 2]);
        rootSize_   = le32(&pvd[156 + 10]);
        joliet_     = false;

        // Ищем Joliet SVD (type 2, escape "%/@"|"%/C"|"%/E").
        for (uint32_t l = 17; l < 17 + 32; ++l) {
            auto d = readIsoBlockSafe(l);
            if (d.size() < 2048) break;
            if (d[0] == 255) break;
            if (d[0] == 2 && std::memcmp(&d[1], "CD001", 5) == 0 &&
                d[88] == 0x25 && d[89] == 0x2F) {
                joliet_ = true;
                blockSize_ = le16(&d[128]); if (blockSize_ == 0) blockSize_ = kIsoBlockSize;
                rootExtent_ = le32(&d[156 + 2]);
                rootSize_   = le32(&d[156 + 10]);
                break;
            }
        }

        isoOk_ = true;
        return true;
    }

    std::vector<uint8_t> readIsoBlockSafe(uint32_t lba) const {
        std::vector<uint8_t> out(kIsoBlockSize);
        if (!readIsoBlocks(lba, 1, out.data())) out.clear();
        return out;
    }

    static std::string toLowerStr(std::string s) {
        for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    static bool iequals(const std::string& a, const std::string& b) {
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i)
            if (std::tolower(static_cast<unsigned char>(a[i])) !=
                std::tolower(static_cast<unsigned char>(b[i])))
                return false;
        return true;
    }

    static void appendUtf8(std::string& s, uint32_t cp) {
        if (cp < 0x80) {
            s += static_cast<char>(cp);
        } else if (cp < 0x800) {
            s += static_cast<char>(0xC0 | (cp >> 6));
            s += static_cast<char>(0x80 | (cp & 0x3F));
        } else {
            s += static_cast<char>(0xE0 | (cp >> 12));
            s += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            s += static_cast<char>(0x80 | (cp & 0x3F));
        }
    }

    static std::string decodeName(const uint8_t* p, int len, bool joliet) {
        std::string s;
        if (joliet) {
            for (int i = 0; i + 1 < len; i += 2) {
                uint32_t cp = (static_cast<uint32_t>(p[i]) << 8) | p[i + 1];
                if (cp == 0) continue;
                appendUtf8(s, cp);
            }
        } else {
            for (int i = 0; i < len; ++i) {
                char c = static_cast<char>(p[i]);
                if (c == ';') break;           // "FILE.EXE;1" -> "FILE.EXE"
                s += c;
            }
            while (!s.empty() && s.back() == '.') s.pop_back();
        }
        return s;
    }

    static std::string joinIsoPath(const std::string& dir, const std::string& name) {
        if (dir.empty() || dir == "/") return "/" + name;
        if (dir.back() == '/') return dir + name;
        return dir + "/" + name;
    }

    static std::vector<std::string> splitPath(const std::string& p) {
        std::vector<std::string> parts;
        std::string cur;
        for (char c : p) {
            if (c == '/' || c == '\\') {
                if (!cur.empty()) { parts.push_back(cur); cur.clear(); }
            } else {
                cur += c;
            }
        }
        if (!cur.empty()) parts.push_back(cur);
        return parts;
    }

    std::optional<Entry> resolve(const std::string& path) const {
        if (!ensureIso()) return std::nullopt;
        Entry cur = makeRootEntry();
        for (auto& part : splitPath(path)) {
            if (!cur.isDirectory) return std::nullopt;
            bool found = false;
            for (auto& e : listDirOf(cur)) {
                if (iequals(e.name, part)) { cur = e; found = true; break; }
            }
            if (!found) return std::nullopt;
        }
        return cur;
    }

    std::vector<Entry> listDirOf(const Entry& dir) const {
        std::vector<Entry> res;
        uint64_t size = dir.size;
        uint32_t lba  = dir.extentLba;
        uint32_t blocks = static_cast<uint32_t>((size + kIsoBlockSize - 1) / kIsoBlockSize);
        std::vector<uint8_t> blk(kIsoBlockSize);

        for (uint32_t b = 0; b < blocks; ++b) {
            if (!readIsoBlocks(lba + b, 1, blk.data())) break;
            uint32_t off = 0;
            while (off + 33 <= kIsoBlockSize) {
                uint8_t rlen = blk[off];
                if (rlen == 0) break;                       // padding до конца сектора
                if (off + rlen > kIsoBlockSize) break;      // повреждённая запись
                uint8_t flags   = blk[off + 25];
                uint8_t nameLen = blk[off + 32];
                if (nameLen == 0 || off + 33 + nameLen > kIsoBlockSize) { off += rlen; continue; }
                const uint8_t* namePtr = blk.data() + off + 33;
                if (nameLen == 1 && (namePtr[0] == 0 || namePtr[0] == 1)) { off += rlen; continue; }

                Entry e;
                e.isDirectory = (flags & 0x02) != 0;
                e.extentLba   = le32(blk.data() + off + 2);
                e.size        = le32(blk.data() + off + 10);
                e.name        = decodeName(namePtr, nameLen, joliet_);
                e.fullPath    = joinIsoPath(dir.fullPath, e.name);
                res.push_back(std::move(e));
                off += rlen;
            }
        }

        std::sort(res.begin(), res.end(), [](const Entry& a, const Entry& b) {
            if (a.isDirectory != b.isDirectory) return a.isDirectory > b.isDirectory;
            return toLowerStr(a.name) < toLowerStr(b.name);
        });
        return res;
    }

    std::vector<uint8_t> readExtent(uint32_t lba, uint64_t size) const {
        std::vector<uint8_t> out;
        out.reserve(static_cast<size_t>(size));
        std::vector<uint8_t> blk(kIsoBlockSize);
        uint64_t remaining = size;
        uint32_t cur = lba;
        while (remaining > 0) {
            uint32_t chunk = static_cast<uint32_t>(std::min<uint64_t>(remaining, kIsoBlockSize));
            if (!readIsoBlocks(cur, 1, blk.data()))
                throw std::runtime_error("tiny-cdio: read error while reading file extent");
            out.insert(out.end(), blk.begin(), blk.begin() + chunk);
            remaining -= chunk;
            ++cur;
        }
        return out;
    }

    // --- WAV ---------------------------------------------------------------

    static void putLe16(std::vector<uint8_t>& v, uint16_t x) {
        v.push_back(static_cast<uint8_t>(x & 0xFF));
        v.push_back(static_cast<uint8_t>((x >> 8) & 0xFF));
    }
    static void putLe32(std::vector<uint8_t>& v, uint32_t x) {
        v.push_back(static_cast<uint8_t>(x & 0xFF));
        v.push_back(static_cast<uint8_t>((x >> 8) & 0xFF));
        v.push_back(static_cast<uint8_t>((x >> 16) & 0xFF));
        v.push_back(static_cast<uint8_t>((x >> 24) & 0xFF));
    }

    static std::vector<uint8_t> makeWavHeader(uint64_t dataBytes,
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
};

} // namespace tinycdio
