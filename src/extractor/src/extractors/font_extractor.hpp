#pragma once
#include "../structs/int_types.h"
#include "../utils/save_png.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Get width of glyph for character ch (in pixels)
static int GetCharWidth(int ch, const u8* fontVar5) {
    u16 entry = (u16)((fontVar5[ch * 2] << 8) | fontVar5[ch * 2 + 1]);
    return (entry >> 12) & 0xF;
}

// Render a single glyph into the image buffer (RGBA, white color, opaque)
static void RenderGlyph(const u8* fontVar4, const u8* fontVar5,
                        int charSize, int fontHeight,
                        int ch, int x, int y,
                        u8* img, int imgWidth, int imgHeight) {
    int w = GetCharWidth(ch, fontVar5);
    if (w == 0) return;

    u16 entry = (u16)((fontVar5[ch * 2] << 8) | fontVar5[ch * 2 + 1]);
    int offsetBits = entry & 0xFFF;
    const u8* base = fontVar4 + (offsetBits >> 3);
    int startBit = offsetBits & 7;

    for (int row = 0; row < fontHeight; ++row) {
        const u8* srcRow = base + row * charSize;
        int bit = startBit;
        int byteIdx = 0;
        u8 curByte = srcRow[byteIdx];

        for (int col = 0; col < w; ++col) {
            if (curByte & (0x80 >> bit)) {
                int px = x + col;
                int py = y + row;
                if (px >= 0 && px < imgWidth && py >= 0 && py < imgHeight) {
                    size_t idx = ((size_t)py * imgWidth + px) * 4;
                    img[idx + 0] = 255; // R
                    img[idx + 1] = 255; // G
                    img[idx + 2] = 255; // B
                    img[idx + 3] = 255; // A
                }
            }
            if (++bit == 8) {
                bit = 0;
                curByte = srcRow[++byteIdx];
            }
        }
    }
}

// Draw a string of characters with letter and word spacing.
// Returns the number of pixels advanced in X.
static int DrawString(const u8* fontVar4, const u8* fontVar5,
                      int charSize, int fontHeight,
                      const char* str, int x, int y,
                      u8* img, int imgWidth, int imgHeight,
                      int letterSpace, int wordSpace) {
    int curX = x;
    while (*str) {
        u8 ch = (u8)*str;
        int w = GetCharWidth(ch, fontVar5);
        if (w > 0) {
            RenderGlyph(fontVar4, fontVar5, charSize, fontHeight,
                        ch, curX, y, img, imgWidth, imgHeight);
            curX += w + letterSpace;
        } else {
            curX += wordSpace;   // space or other invisible character
        }
        ++str;
    }
    return curX - x;
}

// Measure the width of a string (without drawing)
static int MeasureString(const char* str, const u8* fontVar5,
                         int letterSpace, int wordSpace) {
    int width = 0;
    while (*str) {
        u8 ch = (u8)*str;
        int w = GetCharWidth(ch, fontVar5);
        if (w > 0) {
            width += w + letterSpace;
        } else {
            width += wordSpace;
        }
        ++str;
    }
    return width;
}

inline void ExtractFontGlyphs(const u8* fontData, const char* outputFile, int padding) {
    // -------- Read font header (fully matches ExtSetFont) --------
    const u8* p = fontData;

    int16_t tempDx = (int16_t)(p[0] | (p[1] << 8));
    p += 2;

    int fontHeight = p[0];
    p += 1;

    int charSize = p[0];
    p += 1;
    if (charSize == 0) {
        charSize = p[0] | (p[1] << 8);   // Little‑Endian
        p += 2;
    } else {
        p += 2;   // skip reserved bytes
    }

    int16_t tempAxFlip = (int16_t)((p[0] << 8) | p[1]);   // Big‑Endian
    p += 2;

    const u8* fontVar4 = p;                           // bitmap data
    const u8* fontVar1 = fontData;
    ptrdiff_t offsetTable = (ptrdiff_t)tempAxFlip - (ptrdiff_t)(tempDx & 0xFF) * 2;
    const u8* fontVar5 = fontVar1 + offsetTable;      // character table

    // -------- Character spacing parameters --------
    const int letterSpace = 1;   // g_fontInterLetterSpace
    const int wordSpace   = 2;   // g_fontInterWordSpace

    // -------- Table parameters --------
    const int rows = 16;
    const int cols = 16;
    const int innerPad = 2;          // inner padding inside cell
    const int verticalGap = 2;       // gap between code and glyph inside cell

    // -------- Calculate cell dimensions --------
    // Maximum glyph width
    int maxGlyphWidth = 0;
    for (int c = 0; c < 256; ++c) {
        int w = GetCharWidth(c, fontVar5);
        if (w > maxGlyphWidth) maxGlyphWidth = w;
    }

    // Width of three decimal digits (e.g., "888" – the widest digits)
    char sampleStr[] = "888";
    int codeWidth = MeasureString(sampleStr, fontVar5, letterSpace, wordSpace);

    // Cell size
    int cellWidth  = (maxGlyphWidth > codeWidth ? maxGlyphWidth : codeWidth) + 2 * innerPad;
    int cellHeight = fontHeight + verticalGap + fontHeight + 2 * innerPad; // code on top, glyph below

    // Total image dimensions (including padding between cells)
    int totalWidth  = cols * (cellWidth  + padding) - padding;
    int totalHeight = rows * (cellHeight + padding) - padding;

    // -------- Create image buffer (RGBA) --------
    size_t pixelCount = (size_t)totalWidth * totalHeight;
    u8* imageData = (u8*)malloc(pixelCount * 4);
    if (!imageData) return;
    memset(imageData, 0, pixelCount * 4);
    for (size_t i = 0; i < pixelCount; ++i) {
        imageData[i * 4 + 3] = 255;
    }    

    // -------- Render table --------
    char codeStr[8]; // enough for "%d" (up to 3 chars + null)
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int code = row * cols + col;
            int x = col * (cellWidth  + padding);
            int y = row * (cellHeight + padding);

            // --- Draw decimal code (horizontally centered) ---
            sprintf(codeStr, "%d", code);
            int actualCodeWidth = MeasureString(codeStr, fontVar5, letterSpace, wordSpace);
            int codeX = x + (cellWidth - actualCodeWidth) / 2;
            int codeY = y + innerPad;
            DrawString(fontVar4, fontVar5, charSize, fontHeight,
                       codeStr, codeX, codeY,
                       imageData, totalWidth, totalHeight,
                       letterSpace, wordSpace);

            // --- Draw glyph (if it exists) ---
            int glyphWidth = GetCharWidth(code, fontVar5);
            if (glyphWidth > 0) {
                int glyphX = x + (cellWidth - glyphWidth) / 2;
                int glyphY = y + innerPad + fontHeight + verticalGap;
                RenderGlyph(fontVar4, fontVar5, charSize, fontHeight,
                            code, glyphX, glyphY,
                            imageData, totalWidth, totalHeight);
            }
        }
    }

    // -------- Save to PNG --------
    savePng(outputFile, totalWidth, totalHeight, imageData, PNG_COLOR_TYPE_RGBA);
    free(imageData);
}

//***************  CHARSET ***********************

size_t utf8_char_length(unsigned char first_byte) {
    if ((first_byte & 0x80) == 0) return 1;
    if ((first_byte & 0xE0) == 0xC0) {
        if (first_byte < 0xC2) throw std::runtime_error("Invalid UTF-8 start byte (overlong or reserved)");
        return 2;
    }
    if ((first_byte & 0xF0) == 0xE0) {
        return 3;
    }
    if ((first_byte & 0xF8) == 0xF0) {
        if (first_byte > 0xF4) throw std::runtime_error("Invalid UTF-8 start byte (beyond U+10FFFF)");
        return 4;
    }
    throw std::runtime_error("Invalid UTF-8 start byte");
}

int charsetToCpp(const std::string filename, const std::string out) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot open input file: " << filename << std::endl;
        return 1;
    }

    size_t file_size = std::filesystem::file_size(filename);
    std::vector<char> buffer(file_size);
    file.read(buffer.data(), file_size);
    file.close();

    std::vector<std::string> symbols;
    size_t pos = 0;

    while (pos < buffer.size() && symbols.size() < 128) {
        unsigned char c = static_cast<unsigned char>(buffer[pos]);

        // Skip spaces
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            ++pos;
            continue;
        }

        // Skip U+2060 (WORD JOINER) – E2 81 A0
        if (pos + 2 < buffer.size() &&
            static_cast<unsigned char>(buffer[pos]) == 0xE2 &&
            static_cast<unsigned char>(buffer[pos + 1]) == 0x81 &&
            static_cast<unsigned char>(buffer[pos + 2]) == 0xA0) {
            pos += 3;
            continue;
        }

        size_t len = utf8_char_length(c);
        if (len == 0 || pos + len > buffer.size()) {
            std::cerr << "Invalid or truncated UTF-8 sequence at position " << pos << std::endl;
            return 1;
        }
        symbols.emplace_back(buffer.data() + pos, len);
        pos += len;
    }

    if (symbols.size() != 128) {
        std::cerr << "Expected 128 symbols, but got " << symbols.size() << std::endl;
        return 1;
    }

    std::ofstream outfile(out);
    if (!outfile.is_open()) {
        std::cerr << "Cannot open output file: " << out << std::endl;
        return 1;
    }

    outfile << "const std::array<std::string, 128> CHARSET = {{\n";

    for (size_t i = 0; i < symbols.size(); ++i) {
        const std::string& sym = symbols[i];

        outfile << "    \"";
        for (unsigned char byte : sym) {
            outfile << "\\x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
                    << static_cast<int>(byte);
        }
        outfile << "\"";

        if (i != symbols.size() - 1) outfile << ",";

        if ((i + 1) % 8 == 0)
            outfile << "\n";
        else
            outfile << " ";
    }

    outfile << "}};\n";
    outfile.close();
    return 0;
}