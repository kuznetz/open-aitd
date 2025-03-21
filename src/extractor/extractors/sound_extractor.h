#pragma once
#include <cmath>
#include <fstream>
#include <iostream>
#include "../structs/int_types.h"
using namespace std;

struct VOCSample {
    char* data;
    int size;
    int rate;
};

VOCSample loadVoc(char* samplePtr, int size)
{
    VOCSample result;
    //assert(samplePtr[26] == 1); //assert first block is of sound data type
    int sampleSize = (READ_LE_U32(samplePtr + 26) >> 8) - 2;
    int frequencyDiv = *(unsigned char*)(samplePtr + 30);
    int codecId = samplePtr[31];
    char* sampleData = samplePtr + 32;
    int sampleRate = 1000000 / (256 - frequencyDiv);


    result.data = sampleData;
    result.size = sampleSize - 1;
    result.rate = sampleRate;
    //format = SF_FORMAT_VOC | SF_FORMAT_PCM_U8;
    return result;
}

typedef struct wav_header {
    // RIFF Header
    char riff_header[4]; // Contains "RIFF"
    int wav_size; // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
    char wave_header[4]; // Contains "WAVE"

    // Format Header
    char fmt_header[4]; // Contains "fmt " (includes trailing space)
    int fmt_chunk_size; // Should be 16 for PCM
    short audio_format; // Should be 1 for PCM. 3 for IEEE Float
    short num_channels;
    int sample_rate;
    int byte_rate; // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
    short sample_alignment; // num_channels * Bytes Per Sample
    short bit_depth; // Number of bits per sample

    // Data
    char data_header[4]; // Contains "data"
    int data_bytes; // Number of bytes in data. Number of samples * num_channels * sample byte size
    // uint8_t bytes[]; // Remainder of wave file is bytes
} wav_header;

void writeWav(VOCSample* voc ,string filename)
{
    wav_header h = {
        {'R','I','F','F'},
        voc->size + sizeof(wav_header) - 8,
        {'W','A','V','E'},
        {'f','m','t',' '},
        16,
        1,
        1,
        voc->rate,
        voc->rate,
        1,
        8,
        {'d','a','t','a'},
        voc->size
    };
    ofstream f(filename, ios::binary);
    f.write((char*)&h, sizeof(wav_header));
    f.write(voc->data, voc->size);
}
