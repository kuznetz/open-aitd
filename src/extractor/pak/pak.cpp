// seg 55

#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include "../structs/int_types.h"
#include "unpack.h"
#include "pak.h"

#ifdef WIN32
#include <direct.h>
#endif

PakFile::~PakFile() 
{
    if (this->fileHandle != 0) {
        fclose(fileHandle);
    }
}

PakFile::PakFile(string fname)
{
    this->open(fname);
}

void PakFile::open(string fname)
{
    this->fileHandle = fopen(fname.c_str(), "rb");

    //u32 start;
    //fread(&start, 4, 1, fileHandle);
    //start = READ_LE_U32(&start);
    fseek(this->fileHandle, 4, SEEK_CUR);

    u32 fileOffset;
    fread(&fileOffset, 4, 1, fileHandle);
    fileOffset = READ_LE_U32(&fileOffset);
    int blockcount = (fileOffset - 4) / 4;
    this->headers.resize(blockcount);
    this->headers[0].headerOffset = fileOffset;
    for (int i = 1; i < blockcount; i++) {
        fread(&fileOffset, 4, 1, fileHandle);
        fileOffset = READ_LE_U32(&fileOffset);
        this->headers[i].headerOffset = fileOffset;
    }

    const int bufLen = 4 + 4 + 1 + 1 + 2;
    u8 headerBuf[bufLen];
    for (int i = 0; i < blockcount; i++) {
        auto& h = this->headers[i];
        fseek(this->fileHandle, h.headerOffset, SEEK_SET);

        u32 additionalDescriptorSize;
        fread(&additionalDescriptorSize, 4, 1, fileHandle);
        additionalDescriptorSize = READ_LE_U32(&additionalDescriptorSize);
        if (additionalDescriptorSize)
        {
            additionalDescriptorSize -= 4;
            //h.descriptor.resize(additionalDescriptorSize, ' ');
            //fread((char*)h.additional.c_str(), additionalDescriptorSize, 1, this->fileHandle);
            fseek(this->fileHandle, additionalDescriptorSize, SEEK_CUR);
        }

        fread(headerBuf, 1, bufLen, this->fileHandle);
        h.discSize = READ_LE_U32(headerBuf);
        h.uncompressedSize = READ_LE_U32(headerBuf + 4);
        h.compressionFlag = READ_LE_U8(headerBuf + 8);
        h.info5 = READ_LE_U8(headerBuf + 9);
        s16 offset = READ_LE_U16(headerBuf + 10);
        if (offset)
        {
            h.name.resize(offset, ' ');
            fread((char*)h.name.c_str(), offset, 1, this->fileHandle);
        }
        //h.dataOffset = h.headerOffset + 4 + additionalDescriptorSize + bufLen + offset;
        h.dataOffset = ftell(this->fileHandle);
    }

}

vector<u8> PakFile::readBlock(int index)
{
    auto& pakInfo = this->headers[index];
    vector<u8> data(pakInfo.discSize);
    fseek(this->fileHandle, pakInfo.dataOffset, SEEK_SET);
    fread(data.data(), pakInfo.discSize, 1, fileHandle);
    switch (pakInfo.compressionFlag)
    {
    case 0:
    {
        return data;
        break;
    }
    case 1:
    {
        vector<u8> extrData(pakInfo.uncompressedSize);
        PAK_explode((unsigned char*)data.data(), (unsigned char*)extrData.data(), pakInfo.discSize, pakInfo.uncompressedSize, pakInfo.info5);
        return extrData;
        break;
    }
    case 4:
    {
        vector<u8> extrData(pakInfo.uncompressedSize);
        PAK_deflate((unsigned char*)data.data(), (unsigned char*)extrData.data(), pakInfo.discSize, pakInfo.uncompressedSize);
        return extrData;
        break;
    }
    default:
        throw new exception("Unknown compression");
        break;
    }
}
