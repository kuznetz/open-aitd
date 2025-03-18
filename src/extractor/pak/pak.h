#pragma once
#include <vector>
#include <string>

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
    vector<pakInfoStruct> headers;

    ~PakFile();
    void open(string fname);
    vector<u8> readBlock(int index);
};

char* loadPak(const char* name, int index);
int LoadPak(const char* name, int index, char* ptr);
int getPakSize(const char* name, int index);
unsigned int PAK_getNumFiles(const char* name);
void dumpPak(const char* name);
