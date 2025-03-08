#include <stdio.h>   
#include <stdlib.h>
#include <string>

#include "background_extractor.h"
#include "save_png.h"
#include "../structures/int_types.h"
#include "pak/pak.h"

unsigned char* palette = 0;

void freeExtractor() {
    if (palette != 0) delete[] palette;
}

void loadPalette() {
    if (palette != 0) return;
    palette = new unsigned char[256 * 30];
    if (!LoadPak("original/ITD_RESS", 3, (char*)palette))
    {
        printf("Failed to load palette\n");
        return;
    }
}

void extractBackground(char* filename, int camera, char* outPng) {
    loadPalette();
    //unsigned char* img = new unsigned char[320 * 200];
    auto sz = getPakSize(filename, camera);
    auto img = loadPak(filename, camera);
    if (!img)
    {
        printf("Failed to load background\n");
        return;
    }

    unsigned char* data = new unsigned char[320 * 200 * 3];
    for (int i = 0; i < (320 * 200); i++) {
        u8 idx = ((u8*)img)[i];
        data[i * 3 + 0] = palette[idx * 3 + 0];
        data[i * 3 + 1] = palette[idx * 3 + 1];
        data[i * 3 + 2] = palette[idx * 3 + 2];
    }

    savePng( outPng, 320, 200, data );

    delete[] data;
    delete[] img;
}


