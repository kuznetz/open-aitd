#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <array>
#include <stdexcept>
#include "../structs/int_types.h"

namespace AITDExtractor {

    typedef std::array<std::string, 128> AitdCharset;

    inline const AitdCharset AITD1_CHARSET = {{
        "\xC3\x87","\xC3\xBC","\xC3\xA9","\xC3\xA2","\xC3\xA4","\xC3\xA0","\xC3\xA5","\xC3\xA7",
        "\xC3\xAA","\xC3\xAB","\xC3\xA8","\xC3\xAF","\xC3\xAE","\xC3\xAC","\xC3\x84","\xC3\x85",
        "\xC3\x89","\xC3\xA6","\xC3\x86","\xC3\xB4","\xC3\xB6","\xC3\xB2","\xC3\xBB","\xC3\xB9",
        "\xC3\xBF","\xC3\x96","\xC3\x9C","\xC2\xA2","\xC2\xA3","\xC2\xA5","\x3F"    ,"\xC2\xA7",
        "\xC3\xA1","\xC3\xAD","\xC3\xB3","\xC3\xBA","\xC3\xB1","\xC3\x91","\xC2\xAA","\xC2\xBA",
        "\xC2\xBF","\xC2\xA9","\xC2\xAA","\xC2\xBD","\xC2\xBC","\xC3\xA4","\xC2\xAE","\x69",
        "\xC2\xB0","\xC2\xB1","\xC2\xB2","\xC2\xB3","\xC2\xB4","\xC2\xB5","\xC2\xB6","\xC2\xB7",
        "\xC2\xB8","\xC2\xB9","\xC2\xBA","\xC2\xBB","\xC2\xBC","\xC2\xBD","\xC2\xBE","\xC2\xBF",
        "\xC3\x80","\xC3\x81","\xC3\x82","\xC3\x83","\xC3\x84","\xC3\x85","\xC3\x86","\xC3\x87",
        "\xC3\x89","\xC3\x88","\xC3\x8A","\xC3\x8B","\xC3\x8C","\xC3\x8D","\xC3\x8E","\xC3\x8F",
        "\xC3\x90","\xC3\x91","\xC3\x92","\xC3\x93","\xC3\x94","\xC3\x95","\xC3\x96","\xC3\x97",
        "\xC3\x98","\xC3\x99","\xC3\x9A","\xC3\x9B","\xC3\x9C","\xC3\x9D","\xC3\x9E","\xC3\x9F",
        "\xC3\x9E","\xC3\x9F","\xC3\xA2","\xC3\xA3","\xC3\xA4","\xC3\xA5","\xC3\xA6","\xC3\xA7",
        "\xC3\xA8","\xC3\xA9","\xC3\xAA","\xC3\xAB","\xC3\xAC","\xC3\xAD","\xC3\xAE","\xC3\xAF",
        "\xC3\xB0","\xC3\xB1","\xC3\xB2","\xC3\xB3","\xC3\xB4","\xC3\xB5","\xC3\xB6","\xC3\xB7",
        "\xC3\xB8","\xC3\xB9","\xC3\xBA","\xC3\xBB","\xC3\xBC","\xC2\xB2","\xC3\xBE","\xC3\xBF"
    }};


    std::string convert_text(const char* input, size_t length, const AitdCharset& charset) {
        std::string output;
        output.reserve(length * 4);

        for (size_t i = 0; i < length; ++i) {
            unsigned char c = static_cast<unsigned char>(input[i]);
            if (c < 128) {
                output.push_back(static_cast<char>(c));
            } else {
                size_t idx = static_cast<size_t>(c) - 128;
                if (idx >= charset.size()) {
                    throw std::runtime_error("Invalid byte value in input");
                }
                output.append(charset[idx]);
            }
        }

        return output;
    }

    std::string charset_test(const AitdCharset& table) {
        std::string output;
        output.reserve(128 * 4);
        for (int i=0; i<128; i++) {
            output.append(string("[")+to_string(i+128)+"]"+table[i]+" ");
        }
        return output;
    }    

    inline void extractText(const std::vector<u8>& data, const std::string& outPath) {
        size_t endPos = data.size();
        for (size_t i = 0; i < data.size(); ++i) {
            if (data[i] == 0x1A) {
                endPos = i;
                break;
            }
        }

        std::vector<u8> cleanData;
        cleanData.reserve(endPos);
        for (size_t i = 0; i < endPos; ++i) {
            u8 c = data[i];
            if (c == 0x0D) {
                cleanData.push_back('\n');
                if (i + 1 < endPos && data[i + 1] == 0x0A) {
                    ++i;
                }
                continue;
            }
            if (c < 0x20 && c != '\t' && c != '\n') {
                continue;
            }
            cleanData.push_back(c);
        }

        std::string utf8Text = convert_text(
            reinterpret_cast<const char*>(cleanData.data()),
            cleanData.size(),
            AITD1_CHARSET
        );

        std::ofstream out(outPath, std::ios::binary | std::ios::trunc);
        if (!out) {
            std::cerr << "Could not open file: " << outPath << std::endl;
            return;
        }
        out.write(utf8Text.c_str(), utf8Text.size());
        out.close();
    }

} // namespace AITDExtractor