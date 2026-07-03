#include <fstream>
#include <string>
#include <vector>
#include <exception>
#include "../structs/int_types.h"
#include "pak.h"

namespace AITDExtractor {
    using namespace std;

    PakFile::PakFile() {
    }

    PakFile::PakFile(string fname) {
        this->open(fname);
    }

    void PakFile::open(string fname, int blockcount) {
        file.open(fname, ios::binary);
        if (!file.is_open()) {
            throw runtime_error("Cannot open file: " + fname);
        }

        u32 start;
        file.read(reinterpret_cast<char*>(&start), 4);
        start = READ_LE_U32(&start);

        u32 fileOffset;
        file.read(reinterpret_cast<char*>(&fileOffset), 4);
        fileOffset = READ_LE_U32(&fileOffset);

        if (blockcount == 0) {
            blockcount = (fileOffset - 4) / 4;
        }
        this->headers.resize(blockcount);
        this->headers[0].headerOffset = fileOffset;
        for (int i = 1; i < blockcount; i++) {
            file.read(reinterpret_cast<char*>(&fileOffset), 4);
            fileOffset = READ_LE_U32(&fileOffset);
            this->headers[i].headerOffset = fileOffset;
        }

        const int bufLen = 4 + 4 + 1 + 1 + 2;
        u8 headerBuf[bufLen];
        for (int i = 0; i < blockcount; i++) {
            auto& h = this->headers[i];
            file.seekg(h.headerOffset, ios::beg);

            u32 additionalDescriptorSize;
            file.read(reinterpret_cast<char*>(&additionalDescriptorSize), 4);
            additionalDescriptorSize = READ_LE_U32(&additionalDescriptorSize);
            if (additionalDescriptorSize) {
                additionalDescriptorSize -= 4;
                file.seekg(additionalDescriptorSize, ios::cur);
            }

            file.read(reinterpret_cast<char*>(headerBuf), bufLen);
            h.discSize = READ_LE_U32(headerBuf);
            h.uncompressedSize = READ_LE_U32(headerBuf + 4);
            h.compressionFlag = READ_LE_U8(headerBuf + 8);
            h.info5 = READ_LE_U8(headerBuf + 9);
            s16 offset = READ_LE_U16(headerBuf + 10);
            if (offset) {
                h.name.resize(offset);
                file.read(&h.name[0], offset);
            }
            h.dataOffset = static_cast<u32>(file.tellg());
        }
    }

    vector<u8> PakFile::readBlock(int index) {
        auto& pakInfo = this->headers[index];
        vector<u8> data(pakInfo.discSize);
        file.seekg(pakInfo.dataOffset, ios::beg);
        file.read(reinterpret_cast<char*>(data.data()), pakInfo.discSize);
        switch (pakInfo.compressionFlag) {
        case 0:
            return data;
        case 1: {
            vector<u8> extrData(pakInfo.uncompressedSize);
            PAK_explode(data.data(), extrData.data(), pakInfo.discSize, pakInfo.uncompressedSize, pakInfo.info5);
            return extrData;
        }
        case 4: {
            vector<u8> extrData(pakInfo.uncompressedSize);
            PAK_deflate(data.data(), extrData.data(), pakInfo.discSize, pakInfo.uncompressedSize);
            return extrData;
        }
        default:
            throw new exception("Unknown compression");
        }
    }
}