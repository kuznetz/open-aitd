#pragma once
#include <vector>
#include <fstream>
#include <string>
#include "../structs/int_types.h"

namespace AITDExtractor {

    using namespace std;

    typedef struct pakInfoStruct
    {
        int headerOffset;
        int dataOffset;
        string name;
        s32 discSize;
        s32 uncompressedSize;
        char compressionFlag;
        char info5;
    } pakInfoStruct;

    class PakFile {
    public:
        ifstream file;
        vector<pakInfoStruct> headers = {};

        PakFile();
        PakFile(string fname);
        void open(string fname, int blockcount = 0);
        vector<u8> readBlock(int index);
    private:
        int PAK_deflate(u8* srcBuffer, u8* dstBuffer, unsigned int compressedSize, unsigned int uncompressedSize);
        int PAK_explode(u8* srcBuffer, u8* dstBuffer, unsigned int compressedSize, unsigned int uncompressedSize, unsigned short flags);
    };

}