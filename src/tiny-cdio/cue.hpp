#pragma once
/*
 * tiny-cdio — разбор CUE и низкоуровневое чтение образа CUE/BIN.
 *
 * Базовый слой библиотеки: DiscBase.
 *   - парсинг .cue (FILE/TRACK/INDEX/PREGAP/FLAGS/CD-TEXT-метаданные);
 *   - дорожки (TrackInfo) и открытые файлы образа;
 *   - чтение сырых секторов и аудио-дорожек;
 *   - сохранение аудио-дорожки в WAV.
 *
 * ISO9660 строится поверх этого слоя (см. iso9660.hpp), публичный фасад —
 * Disc (см. tiny_cdio.hpp). Подключается через "tiny_cdio.hpp".
 */

#include "types.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace tinycdio {

// ---------------------------------------------------------------------------
// Внутренние хелперы разбора CUE
// ---------------------------------------------------------------------------
namespace detail {

struct CueFile {
    std::string name;
};

struct CueToken {
    std::string text;
    int         line = 0;
};

inline std::string toUpper(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

inline std::vector<CueToken> tokenizeCue(const std::string& src) {
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

inline bool parseMsf(const std::string& s, uint64_t& lba) {
    unsigned m = 0, sec = 0, fr = 0;
    if (std::sscanf(s.c_str(), "%u:%u:%u", &m, &sec, &fr) != 3) return false;
    if (sec >= 60 || fr >= 75) return false;
    lba = (static_cast<uint64_t>(m) * 60 + sec) * 75 + fr;
    return true;
}

inline bool applyTrackMode(const std::string& m, TrackInfo& t) {
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

inline std::string readTextFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return {};
    std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return s;
}

inline bool isAbsolutePath(const std::string& p) {
    if (p.empty()) return false;
    if (p[0] == '/' || p[0] == '\\') return true;
    if (p.size() >= 2 && std::isalpha(static_cast<unsigned char>(p[0])) && p[1] == ':')
        return true;
    return false;
}

inline std::string dirName(const std::string& p) {
    size_t s = p.find_last_of("/\\");
    return (s == std::string::npos) ? std::string() : p.substr(0, s);
}

inline std::string joinOsPath(const std::string& dir, const std::string& name) {
    if (name.empty() || isAbsolutePath(name) || dir.empty()) return name;
    char last = dir.back();
    if (last == '/' || last == '\\') return dir + name;
    return dir + "/" + name;
}

} // namespace detail

// ---------------------------------------------------------------------------
// DiscBase — образ CUE/BIN: дорожки, сырые сектора, аудио
// ---------------------------------------------------------------------------
class DiscBase {
protected:
    // FileHandle объявлен до TrackReader: последний хранит указатель на него.
    struct FileHandle {
        std::string   path;
        mutable std::fstream stream;    ///< mutable: чтение идёт из const-методов
        uint32_t      stride      = 0;  ///< байт на сектор в этом файле
        uint64_t      byteSize    = 0;
        uint64_t      sectorCount = 0;
    };

public:
    DiscBase() = default;
    DiscBase(const DiscBase&) = delete;
    DiscBase& operator=(const DiscBase&) = delete;
    DiscBase(DiscBase&&) = default;
    DiscBase& operator=(DiscBase&&) = default;

    // -----------------------------------------------------------------------
    // Потоковое чтение дорожки (без загрузки всей дорожки в память)
    // -----------------------------------------------------------------------
    /**
     * Последовательный/произвольный доступ к дорожке как к потоку байт.
     * Данные подчитываются посекторно по мере вызовов read()/readChunk().
     */
    class TrackReader {
    public:
        TrackReader() = default;

        TrackReader(const DiscBase* disc, size_t trackIndex,
                    uint64_t firstSector, uint64_t count)
            : disc_(disc) {
            const TrackInfo& t = disc->tracks_.at(trackIndex);
            file_ = disc->files_.at(t.fileIndex).get();
            if (t.sectorStride != file_->stride)
                throw std::runtime_error("tiny-cdio: track/file sector stride mismatch");
            stride_ = t.sectorStride;
            start_  = t.startSector + firstSector;
            uint64_t avail = (t.sectorCount > firstSector) ? (t.sectorCount - firstSector) : 0;
            end_ = start_ + (count ? std::min(count, avail) : avail);
            cur_ = start_;
            buf_.resize(stride_);
        }

        /** Была ли ошибка чтения. */
        bool bad() const { return bad_; }

        bool eof() const { return !bad_ && cur_ >= end_ && bufPos_ >= bufLen_; }

        /** Сколько байт ещё не отдано. */
        uint64_t remaining() const {
            uint64_t rest = (cur_ < end_) ? (end_ - cur_) : 0;
            return rest * stride_ + (bufLen_ - bufPos_);
        }

        /** Сколько байт уже отдано от начала диапазона. */
        uint64_t tell() const {
            uint64_t done = (cur_ > start_) ? (cur_ - start_) : 0;
            return done * stride_ - (bufLen_ - bufPos_);
        }

        /**
         * Прочитать не более maxBytes байт в dst. Возвращает число реально
         * прочитанных байт; 0 означает конец потока (или ошибку — см. bad()).
         */
        size_t read(uint8_t* dst, size_t maxBytes) {
            size_t n = 0;
            while (n < maxBytes) {
                if (bufPos_ >= bufLen_) {
                    if (cur_ >= end_) break;
                    if (!fill()) { bad_ = true; break; }
                }
                size_t take = std::min<size_t>(bufLen_ - bufPos_, maxBytes - n);
                std::memcpy(dst + n, buf_.data() + bufPos_, take);
                bufPos_ += take;
                n += take;
            }
            return n;
        }

        /** Прочитать очередной блок размером не более maxBytes (0 = конец). */
        std::vector<uint8_t> readChunk(size_t maxBytes) {
            size_t want = static_cast<size_t>(std::min<uint64_t>(maxBytes, remaining()));
            std::vector<uint8_t> v(want);
            size_t got = read(v.data(), want);
            v.resize(got);
            return v;
        }

        /** Перейти к смещению byteOffset (в байтах) от начала диапазона. */
        bool seek(uint64_t byteOffset) {
            if (stride_ == 0) return false;
            uint64_t half   = byteOffset / stride_;
            uint64_t within = byteOffset % stride_;
            uint64_t sector = start_ + half;
            if (sector > end_) return false;
            cur_ = sector;
            bufPos_ = bufLen_ = 0;
            if (within) {
                if (cur_ >= end_ || !fill()) return false;
                bufPos_ = static_cast<size_t>(within);
            }
            return true;
        }

    private:
        bool fill() {
            if (!file_ || cur_ >= end_) return false;
            if (!disc_->readRawSectors(*file_, cur_, 1, buf_.data())) return false;
            ++cur_;
            bufPos_ = 0;
            bufLen_ = stride_;
            return true;
        }

        const DiscBase*      disc_   = nullptr;
        FileHandle*          file_   = nullptr;
        uint32_t             stride_ = 0;
        uint64_t             start_  = 0;
        uint64_t             cur_    = 0;
        uint64_t             end_    = 0;
        size_t               bufPos_ = 0;
        size_t               bufLen_ = 0;
        bool                 bad_    = false;
        std::vector<uint8_t> buf_;
    };

    /** Открыть поток чтения дорожки (параметры как у readTrackRaw). */
    TrackReader openTrack(size_t trackIndex, uint64_t firstSector = 0,
                          uint64_t count = 0) const {
        return TrackReader(this, trackIndex, firstSector, count);
    }

    /** Открыть поток чтения аудио-дорожки (бросает, если дорожка не аудио). */
    TrackReader openAudioTrack(size_t trackIndex) const {
        const TrackInfo& t = tracks_.at(trackIndex);
        if (!t.isAudio)
            throw std::runtime_error("tiny-cdio: track is not audio");
        return TrackReader(this, trackIndex, 0, 0);
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
        TrackReader r(this, trackIndex, firstSector, count);
        std::vector<uint8_t> out(static_cast<size_t>(r.remaining()));
        size_t got = r.read(out.data(), out.size());
        out.resize(got);
        if (r.bad())
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
        std::vector<uint8_t> header = detail::makeWavHeader(pcm.size());
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

protected:
    // -----------------------------------------------------------------------
    // Внутренние данные
    // -----------------------------------------------------------------------
    std::string                              cuePath_;
    std::vector<TrackInfo>                   tracks_;
    std::vector<detail::CueFile>             cueFiles_;
    std::vector<std::unique_ptr<FileHandle>> files_;
    int                                      dataTrackIndex_ = -1;

    // -----------------------------------------------------------------------
    // Разбор CUE и открытие образа
    // -----------------------------------------------------------------------
    void init(const std::string& cuePath) {
        cuePath_ = cuePath;
        std::string src = detail::readTextFile(cuePath);
        if (src.empty())
            throw std::runtime_error("tiny-cdio: cannot read cue file: " + cuePath);

        auto tok = detail::tokenizeCue(src);
        size_t curFile = 0;
        bool haveFile = false;

        for (size_t i = 0; i < tok.size();) {
            std::string kw = detail::toUpper(tok[i].text);

            if (kw == "FILE") {
                detail::CueFile cf;
                if (i + 1 < tok.size()) cf.name = tok[i + 1].text;
                cueFiles_.push_back(cf);
                curFile = cueFiles_.size() - 1;
                haveFile = true;
                i += 2;
                if (i < tok.size()) {
                    std::string u = detail::toUpper(tok[i].text);
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
                if (!detail::applyTrackMode(detail::toUpper(tok[i + 2].text), t))
                    throw std::runtime_error("tiny-cdio: unknown track mode: " + tok[i + 2].text);
                tracks_.push_back(t);
                i += 3;
                continue;
            }
            if (kw == "INDEX") {
                if (i + 2 >= tok.size()) break;
                int idx = std::atoi(tok[i + 1].text.c_str());
                uint64_t lba = 0;
                if (detail::parseMsf(tok[i + 2].text, lba) && !tracks_.empty()) {
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
                    std::string u = detail::toUpper(tok[i].text);
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
        std::string base = detail::dirName(cuePath);
        files_.clear();
        for (auto& cf : cueFiles_) {
            auto h = std::make_unique<FileHandle>();
            h->path = detail::joinOsPath(base, cf.name);
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

    // -----------------------------------------------------------------------
    // Чтение секторов
    // -----------------------------------------------------------------------
    static bool readRawSectors(FileHandle& f, uint64_t sector, uint64_t count,
                               uint8_t* out) {
        f.stream.clear();
        f.stream.seekg(static_cast<std::streamoff>(sector * f.stride), std::ios::beg);
        if (!f.stream) return false;
        std::streamsize want = static_cast<std::streamsize>(count * f.stride);
        f.stream.read(reinterpret_cast<char*>(out), want);
        return f.stream.gcount() == want;
    }
};

} // namespace tinycdio
