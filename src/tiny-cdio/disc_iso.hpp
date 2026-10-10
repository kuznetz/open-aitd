#pragma once
/*
 * tiny-cdio — ISO9660 (+ Joliet) file system.
 *
 * Layered on top of DiscBase (see disc_base.hpp): reads logical blocks via
 * readIsoBlocks() and parses:
 *   - the primary volume descriptor (LBA 16, "CD001");
 *   - the optional Joliet SVD (type 2, escape "%/@", "%/C", "%/E");
 *   - directories, extents, file reads.
 */

#include "disc_base.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace tinycdio {

// ---------------------------------------------------------------------------
// DiscIso — access to the ISO9660 file system on the data track
// ---------------------------------------------------------------------------
class DiscIso : public DiscBase {
public:
    DiscIso() = default;
    DiscIso(const DiscIso&) = delete;
    DiscIso& operator=(const DiscIso&) = delete;
    DiscIso(DiscIso&&) = default;
    DiscIso& operator=(DiscIso&&) = default;

    // -----------------------------------------------------------------------
    // Streaming file reading (without loading the whole file into memory)
    // -----------------------------------------------------------------------
    /**
     * Sequential/random access to an ISO9660 file as a byte stream.
     * 2048-byte blocks are read on demand as read()/readChunk() are called.
     */
    class FileReader {
    public:
        FileReader() = default;

        FileReader(const DiscIso* disc, uint32_t lba, uint64_t size)
            : disc_(disc), lba_(lba), size_(size) {}

        /** Whether a read error occurred. */
        bool bad() const { return bad_; }

        bool eof() const { return pos_ >= size_; }

        uint64_t size() const { return size_; }
        uint64_t tell() const { return pos_; }
        uint64_t remaining() const { return (pos_ < size_) ? (size_ - pos_) : 0; }

        /**
         * Read at most maxBytes bytes into dst. Returns the number of bytes
         * actually read; 0 means end of file (or an error — see bad()).
         */
        size_t read(uint8_t* dst, size_t maxBytes) {
            size_t n = 0;
            while (n < maxBytes && pos_ < size_) {
                uint32_t block  = static_cast<uint32_t>(lba_ + pos_ / kIsoBlockSize);
                uint32_t within = static_cast<uint32_t>(pos_ % kIsoBlockSize);
                if (block != cached_) {
                    if (!disc_->readIsoBlocks(block, 1, blk_.data())) { bad_ = true; break; }
                    cached_ = block;
                }
                uint64_t want = std::min<uint64_t>(kIsoBlockSize - within, size_ - pos_);
                want = std::min<uint64_t>(want, maxBytes - n);
                std::memcpy(dst + n, blk_.data() + within, static_cast<size_t>(want));
                pos_ += want;
                n    += static_cast<size_t>(want);
            }
            return n;
        }

        /** Read the next chunk of at most maxBytes bytes (0 = end). */
        std::vector<uint8_t> readChunk(size_t maxBytes) {
            size_t want = static_cast<size_t>(std::min<uint64_t>(maxBytes, remaining()));
            std::vector<uint8_t> v(want);
            size_t got = read(v.data(), want);
            v.resize(got);
            return v;
        }

        /** Seek to byteOffset (in bytes) from the start of the file. */
        bool seek(uint64_t byteOffset) {
            if (byteOffset > size_) return false;
            pos_ = byteOffset;
            return true;
        }

    private:
        const DiscIso*       disc_   = nullptr;
        uint32_t             lba_    = 0;
        uint64_t             size_   = 0;
        uint64_t             pos_    = 0;
        bool                 bad_    = false;
        uint32_t             cached_ = 0xFFFFFFFFu;
        std::vector<uint8_t> blk_    = std::vector<uint8_t>(kIsoBlockSize);
    };

    // --- ISO9660 file system -----------------------------------------------

    /** Open a file read stream by path (throws if missing or a directory). */
    FileReader openFile(const std::string& path) const {
        auto e = find(path);
        if (!e) throw std::runtime_error("tiny-cdio: file not found: " + path);
        if (e->isDirectory) throw std::runtime_error("tiny-cdio: is a directory: " + path);
        return FileReader(this, e->extentLba, e->size);
    }

    /** Whether an accessible ISO9660 file system is present on the data track. */
    bool hasFilesystem() const { return ensureIso(); }

    /** List the contents of a directory (path: "" or "/" means the root). */
    std::vector<Entry> listDir(const std::string& path = std::string()) const {
        if (!ensureIso()) return {};
        auto dir = resolve(path);
        if (!dir || !dir->isDirectory) return {};
        return listDirOf(*dir);
    }

    /** Find an entry by path. */
    std::optional<Entry> find(const std::string& path) const {
        if (!ensureIso()) return std::nullopt;
        return resolve(path);
    }

    bool exists(const std::string& path) const {
        auto e = find(path);
        return e.has_value();
    }

    /** Read the whole contents of a file by path (a wrapper over openFile). */
    std::vector<uint8_t> readFile(const std::string& path) const {
        FileReader r = openFile(path);
        std::vector<uint8_t> out(static_cast<size_t>(r.size()));
        size_t got = r.read(out.data(), out.size());
        out.resize(got);
        if (r.bad())
            throw std::runtime_error("tiny-cdio: read error while reading file extent");
        return out;
    }

    /** Read the contents of a file into a string. */
    std::string readFileText(const std::string& path) const {
        auto bytes = readFile(path);
        return std::string(bytes.begin(), bytes.end());
    }

    /** Recursively traverse the whole tree; returns file and directory entries. */
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

private:
    // --- ISO9660 cache ------------------------------------------------------
    mutable bool     isoTried_    = false;
    mutable bool     isoOk_       = false;
    mutable bool     joliet_      = false;
    mutable uint32_t blockSize_   = kIsoBlockSize;
    mutable uint32_t rootExtent_  = 0;
    mutable uint64_t rootSize_    = 0;

    // --- little-endian ------------------------------------------------------
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

        // Look for a Joliet SVD (type 2, escape "%/@"|"%/C"|"%/E").
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

    // --- names/paths --------------------------------------------------------
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

    // --- traversal/reading --------------------------------------------------
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
                if (rlen == 0) break;                       // padding to the end of the sector
                if (off + rlen > kIsoBlockSize) break;      // corrupted record
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

};

} // namespace tinycdio
