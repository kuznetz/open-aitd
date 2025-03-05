#include <stdio.h>   
#include <stdlib.h>
#include <string>

#include "background_extractor.h"
#include "pak/pak.h"
#include "png.h"

unsigned char* palette = 0;

void freeExtractor() {
    if (palette != 0) delete[] palette;
}

void savePng(const char* filename, int width, int height, void* data, int color_type = PNG_COLOR_TYPE_RGB)
{
    FILE* fp = fopen(filename, "wb");
    if (!fp) abort();

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) abort();

    png_infop info = png_create_info_struct(png);
    if (!info) abort();

    if (setjmp(png_jmpbuf(png))) abort();

    png_init_io(png, fp);

    // Output is 8bit depth, RGBA format.
    //PNG_COLOR_TYPE_GRAY
    png_set_IHDR(
        png,
        info,
        width, height,
        8,
        color_type,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);
    //png_set_filler(png, 0, PNG_FILLER_AFTER);

    if (!data) abort();

    png_bytepp rows = new png_bytep[height];
    for (int k = 0; k < height; k++) {
        //k = each line, k*width = pixels till first pixel of like k, *3 = RGB values
        rows[k] = ((png_bytep)data) + k * width * 3;
    }
    png_write_image(png, rows);
    delete[] rows;
    png_write_end(png, NULL);

    fclose(fp);
    png_destroy_write_struct(&png, &info);
}

void loadPalette() {
    if (palette != 0) return;
    palette = new unsigned char[256 * 30];
    if (!LoadPak("ITD_RESS", 3, (char*)palette))
    {
        printf("Failed to load palette\n");
        return;
    }
}

void extractBackground(char* filename, int camera) {
    loadPalette();
    unsigned char* img = new unsigned char[320 * 200];
    if (!LoadPak(filename, camera, (char*)img))
    {
        printf("Failed to load background\n");
        return;
    }

    unsigned char* data = new unsigned char[320 * 200 * 3];
    for (int i = 0; i < (320 * 200); i++) {
        data[i * 3 + 0] = palette[img[i] * 3 + 0];
        data[i * 3 + 1] = palette[img[i] * 3 + 1];
        data[i * 3 + 2] = palette[img[i] * 3 + 2];
    }

    std::string s = std::string("extracted/") + filename + "_" + std::to_string(camera) + ".png";
    savePng( s.c_str(), 320, 200, data );

    delete[] data;
    delete[] img;
}


