#pragma once
#include <stdio.h>   
#include <stdlib.h>
#include <string>

#include "../utils/save_png.h"
#include "../structs/int_types.h"
#include "../loaders/loaders.h"
#include "../pak/pak.h"

namespace AITDExtractor {

    void extractBackground(u8* img, const char* outPng, const Pallete& pallete) {
        //unsigned char* img = new unsigned char[320 * 200];
        unsigned char* data = new unsigned char[320 * 200 * 3];
        for (int i = 0; i < (320 * 200); i++) {
            u8 idx = ((u8*)img)[i];
            auto& col = pallete[idx];
            data[i * 3 + 0] = col[0];
            data[i * 3 + 1] = col[1];
            data[i * 3 + 2] = col[2];
        }

        savePng( outPng, 320, 200, data );

        delete[] data;
    }

    // Extracts a 320x200 background whose palette is embedded in the data itself.
    // Matches PAKExtract Background.GetBackground "case 64770: //ITD_RESS":
    // the palette starts at offset 2 and the indexed image starts at offset 770.
    void extractPalletteBackground(const u8* data, size_t size, const char* outPng,
                                   size_t palleteOffset = 2, size_t imageOffset = 770) {
        const size_t pixelCount = 320 * 200;
        if (size < imageOffset + pixelCount ||
            size < palleteOffset + PALETTE_SIZE * 3) {
            return; // data too small for an embedded-palette background
        }

        Pallete pallete = loadPalleteFromData(data, palleteOffset);

        unsigned char* rgb = new unsigned char[pixelCount * 3];
        for (size_t i = 0; i < pixelCount; i++) {
            u8 idx = data[imageOffset + i];
            auto& col = pallete[idx];
            rgb[i * 3 + 0] = col[0];
            rgb[i * 3 + 1] = col[1];
            rgb[i * 3 + 2] = col[2];
        }

        savePng(outPng, 320, 200, rgb);

        delete[] rgb;
    }

}