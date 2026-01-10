#pragma once
#include <vector>
#include <string>
#include "../structs/int_types.h"

using namespace std;

typedef struct pakInfoStruct
{
    int headerOffset;
    int dataOffset;
    string name;
    //string additional;
    s32 discSize;
    s32 uncompressedSize;
    char compressionFlag;
    char info5;
    //s16 offset;
} pakInfoStruct;

class PakFile {
public:
    FILE* fileHandle = 0;
    vector<pakInfoStruct> headers = {};

    PakFile();
    PakFile(string fname);
    ~PakFile();
    void open(string fname, int blockcount = 0);
    vector<u8> readBlock(int index);
};
