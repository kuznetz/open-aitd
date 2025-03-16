#include <stdio.h>   
#include <stdlib.h>
#include <string>

#include "background_extractor.h"
#include "save_png.h"
#include "structs/int_types.h"
#include "palette.h"
#include "pak/pak.h"

void extractBackground(char* filename, int camera, char* outPng) {
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
        auto col = getPalColor(idx);
        data[i * 3 + 0] = col[0];
        data[i * 3 + 1] = col[1];
        data[i * 3 + 2] = col[2];
    }

    savePng( outPng, 320, 200, data );

    delete[] data;
    delete[] img;
}


