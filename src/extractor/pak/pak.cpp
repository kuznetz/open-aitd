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

void PakFile::open(string fname)
{
    this->fileHandle = fopen(fname.c_str(), "rb");

    u32 start;
    fread(&start, 4, 1, fileHandle);
    start = READ_LE_U32(&start);
    //fseek(this->fileHandle, 4, SEEK_CUR);

    u32 fileOffset;
    fread(&fileOffset, 4, 1, fileHandle);
    fileOffset = READ_LE_U32(&fileOffset);
    int blockcount = (fileOffset - 8) / 4;
    this->headers.resize(blockcount);
    this->headers[0].headerOffset = fileOffset;
    for (int i = 1; i < blockcount; i++) {
        this->headers[i].headerOffset = fileOffset;
        fread(&fileOffset, 4, 1, fileHandle);
        fileOffset = READ_LE_U32(&fileOffset);
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

//#define USE_UNPACKED_DATA

void readPakInfo(pakInfoStruct* pPakInfo, FILE* fileHandle)
{
    fread(&pPakInfo->discSize,4,1,fileHandle);
    fread(&pPakInfo->uncompressedSize,4,1,fileHandle);
    fread(&pPakInfo->compressionFlag,1,1,fileHandle);
    fread(&pPakInfo->info5,1,1,fileHandle);
    fread(&pPakInfo->dataOffset,2,1,fileHandle);

    pPakInfo->discSize = READ_LE_U32(&pPakInfo->discSize);
    pPakInfo->uncompressedSize = READ_LE_U32(&pPakInfo->uncompressedSize);
    pPakInfo->dataOffset = READ_LE_U16(&pPakInfo->dataOffset);
}

unsigned int PAK_getNumFiles(const char* name)
{
    char bufferName[512] = "";
    FILE* fileHandle;
    u32 fileOffset;

    strcat(bufferName, name); // temporary until makeExtention is coded
    strcat(bufferName,".PAK");

    fileHandle = fopen(bufferName,"rb");

    if (!fileHandle)
        return 0;

    fseek(fileHandle,4,SEEK_CUR);
    fread(&fileOffset,4,1,fileHandle);
#ifdef MACOSX
    fileOffset = READ_LE_U32(&fileOffset);
#endif
    fclose(fileHandle);

    return((fileOffset/4)-2);
}

int LoadPak(const char* name, int index, char* ptr)
{
#ifdef USE_UNPACKED_DATA
    char buffer[256];
    FILE* fHandle;
    int size;

    sprintf(buffer,"%s/%04X.OUT",name,index);

    fHandle = fopen(buffer,"rb");

    if(!fHandle)
        return(0);

    fseek(fHandle,0L,SEEK_END);
    size = ftell(fHandle);
    fseek(fHandle,0L,SEEK_SET);

    fread(ptr,size,1,fHandle);
    fclose(fHandle);

    return(1);
#else
    char* lptr;

    lptr = loadPak(name,index);

    memcpy(ptr,lptr,getPakSize(name,index));

    free(lptr);

    return(1);
#endif
}

int getPakSize(const char* name, int index)
{
#ifdef USE_UNPACKED_DATA
    char buffer[256];
    FILE* fHandle;
    int size;

    sprintf(buffer,"%s/%04X.OUT",name,index);

    fHandle = fopen(buffer,"rb");

    if(!fHandle)
        return(0);

    fseek(fHandle,0L,SEEK_END);
    size = ftell(fHandle);
    fseek(fHandle,0L,SEEK_SET);

    fclose(fHandle);

    return (size);
#else
    char bufferName[512] = "";
    FILE* fileHandle;
    s32 fileOffset;
    s32 additionalDescriptorSize;
    pakInfoStruct pakInfo;
    s32 size=0;

    strcat(bufferName, name); // temporary until makeExtention is coded
    strcat(bufferName,".PAK");

    fileHandle = fopen(bufferName,"rb");

    if(fileHandle) // a bit stupid, should return NULL right away
    {
        fseek(fileHandle,(index+1)*4,SEEK_SET);

        fread(&fileOffset,4,1,fileHandle);
#ifdef MACOSX
        fileOffset = READ_LE_U32(&fileOffset);
#endif
        fseek(fileHandle,fileOffset,SEEK_SET);

        fread(&additionalDescriptorSize,4,1,fileHandle);
#ifdef MACOSX
        additionalDescriptorSize = READ_LE_U32(&additionalDescriptorSize);
#endif

        readPakInfo(&pakInfo,fileHandle);

        fseek(fileHandle,pakInfo.dataOffset,SEEK_CUR);

        if(pakInfo.compressionFlag == 0) // uncompressed
        {
            size = pakInfo.discSize;
        }
        else if(pakInfo.compressionFlag == 1) // compressed
        {
            size = pakInfo.uncompressedSize;
        }
        else if(pakInfo.compressionFlag == 4)
        {
            size = pakInfo.uncompressedSize;
        }

        fclose(fileHandle);
    }

    return size;
#endif
}

char* loadPak(const char* name, int index)
{
    if(PAK_getNumFiles(name) < index)
        return NULL;
    
    
    //dumpPak(name);
#ifdef USE_UNPACKED_DATA
    char buffer[256];
    FILE* fHandle;
    int size;
    char* ptr;

    sprintf(buffer,"%s/%04X.OUT",name,index);

    fHandle = fopen(buffer,"rb");

    if(!fHandle)
        return NULL;

    fseek(fHandle,0L,SEEK_END);
    size = ftell(fHandle);
    fseek(fHandle,0L,SEEK_SET);

    ptr = (char*)malloc(size);

    fread(ptr,size,1,fHandle);
    fclose(fHandle);

    return ptr;
#else
    char bufferName[512] = "";
    FILE* fileHandle;
    u32 fileOffset;
    u32 additionalDescriptorSize;
    pakInfoStruct pakInfo;
    char* ptr=0;


    //makeExtention(bufferName, name, ".PAK");
    strcat(bufferName, name); // temporary until makeExtention is coded
    strcat(bufferName,".PAK");

    fileHandle = fopen(bufferName,"rb");

    if(fileHandle) // a bit stupid, should return NULL right away
    {
        char nameBuffer[256] = "";

        fseek(fileHandle,(index+1)*4,SEEK_SET);

        fread(&fileOffset,4,1,fileHandle);

#ifdef MACOSX
        fileOffset = READ_LE_U32(&fileOffset);
#endif

        fseek(fileHandle,fileOffset,SEEK_SET);

        fread(&additionalDescriptorSize,4,1,fileHandle);

#ifdef MACOSX
        additionalDescriptorSize = READ_LE_U32(&additionalDescriptorSize);
#endif
		if(additionalDescriptorSize)
		{
			fseek(fileHandle, additionalDescriptorSize-4, SEEK_CUR);
		}

		readPakInfo(&pakInfo,fileHandle);

		if(pakInfo.dataOffset)
		{
			fread(nameBuffer,pakInfo.dataOffset,1,fileHandle);
#ifdef FITD_DEBUGGER
			printf("Loading %s/%s\n", name,nameBuffer+2);
#endif
		}
		else
		{
			fseek(fileHandle,pakInfo.dataOffset,SEEK_CUR);
		}

		switch(pakInfo.compressionFlag)
		{
		case 0:
			{
				ptr = (char*)malloc(pakInfo.discSize);
				fread(ptr,pakInfo.discSize,1,fileHandle);
				break;
			}
		case 1:
			{
				char * compressedDataPtr = (char *) malloc(pakInfo.discSize);
				fread(compressedDataPtr, pakInfo.discSize, 1, fileHandle);
				ptr = (char *) malloc(pakInfo.uncompressedSize);

                PAK_explode((unsigned char*)compressedDataPtr, (unsigned char*)ptr, pakInfo.discSize, pakInfo.uncompressedSize, pakInfo.info5);

                free(compressedDataPtr);
                break;
            }
        case 4:
            {
                char * compressedDataPtr = (char *) malloc(pakInfo.discSize);
                fread(compressedDataPtr, pakInfo.discSize, 1, fileHandle);
                ptr = (char *) malloc(pakInfo.uncompressedSize);

                PAK_deflate((unsigned char*)compressedDataPtr, (unsigned char*)ptr, pakInfo.discSize, pakInfo.uncompressedSize);

                free(compressedDataPtr);
                break;
            }
        default:
            break;
        }
        fclose(fileHandle);
    }

    return ptr;
#endif
}

void dumpPak(const char* name)
{
#ifdef WIN32 
    unsigned int numEntries = PAK_getNumFiles(name);

    for (unsigned int index = 0; index < numEntries; index++)
    {
        char bufferName[512] = "";
        FILE* fileHandle;
        u32 fileOffset;
        u32 additionalDescriptorSize;
        pakInfoStruct pakInfo;
        char* ptr = 0;


        //makeExtention(bufferName, name, ".PAK");
        strcat(bufferName, name); // temporary until makeExtention is coded
        strcat(bufferName, ".PAK");

        fileHandle = fopen(bufferName, "rb");

        if (fileHandle) // a bit stupid, should return NULL right away
        {
            char nameBuffer[256] = "";

            fseek(fileHandle, (index + 1) * 4, SEEK_SET);

            fread(&fileOffset, 4, 1, fileHandle);

#ifdef MACOSX
            fileOffset = READ_LE_U32(&fileOffset);
#endif

            fseek(fileHandle, fileOffset, SEEK_SET);

            fread(&additionalDescriptorSize, 4, 1, fileHandle);

#ifdef MACOSX
            additionalDescriptorSize = READ_LE_U32(&additionalDescriptorSize);
#endif
            if (additionalDescriptorSize)
            {
                fseek(fileHandle, additionalDescriptorSize - 4, SEEK_CUR);
            }

            readPakInfo(&pakInfo, fileHandle);

            if (pakInfo.dataOffset)
            {
                fread(nameBuffer, pakInfo.dataOffset, 1, fileHandle);
#ifdef FITD_DEBUGGER
                printf("Loading %s/%s\n", name, nameBuffer + 2);
#endif
            }
            else
            {
                fseek(fileHandle, pakInfo.dataOffset, SEEK_CUR);
            }

            switch (pakInfo.compressionFlag)
            {
            case 0:
            {
                ptr = (char*)malloc(pakInfo.discSize);
                fread(ptr, pakInfo.discSize, 1, fileHandle);
                break;
            }
            case 1:
            {
                char * compressedDataPtr = (char *)malloc(pakInfo.discSize);
                fread(compressedDataPtr, pakInfo.discSize, 1, fileHandle);
                ptr = (char *)malloc(pakInfo.uncompressedSize);

                PAK_explode((unsigned char*)compressedDataPtr, (unsigned char*)ptr, pakInfo.discSize, pakInfo.uncompressedSize, pakInfo.info5);

                free(compressedDataPtr);
                break;
            }
            case 4:
            {
                char * compressedDataPtr = (char *)malloc(pakInfo.discSize);
                fread(compressedDataPtr, pakInfo.discSize, 1, fileHandle);
                ptr = (char *)malloc(pakInfo.uncompressedSize);

                PAK_deflate((unsigned char*)compressedDataPtr, (unsigned char*)ptr, pakInfo.discSize, pakInfo.uncompressedSize);

                free(compressedDataPtr);
                break;
            }
            default:
                break;
            }
            fclose(fileHandle);

            {
                mkdir(name);
                char outputName[256];
                sprintf(outputName, "%s/%02d_%s", name, index, nameBuffer + 2);
                FILE* foutputHandle = fopen(outputName, "wb+");
                if (foutputHandle)
                {
                    fwrite(ptr, pakInfo.uncompressedSize, 1, foutputHandle);
                    fclose(foutputHandle);
                }
            }
        }
    }
#endif
}
