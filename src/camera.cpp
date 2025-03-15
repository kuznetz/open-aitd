#include <stdio.h>   
#include <stdlib.h>
#include <string.h>

#include "pak/pak.h"
#include "raylib.h"
#include "png.h"

void savePng(char* filename, int width, int height, char* data)
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
    png_set_IHDR(
        png,
        info,
        width, height,
        8,
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);

    // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
    // Use png_set_filler().
    //png_set_filler(png, 0, PNG_FILLER_AFTER);

    if (!data) abort();

    png_bytepp rows = new png_bytep[height];
    for (int k = 0; k < height; k++) {
        //k = each line, k*width = pixels till first pixel of like k, *3 = RGB values
        rows[k] = (png_bytep)(data + k * width * 3);
    }
    png_write_image(png, rows);
    delete[] rows;
    png_write_end(png, NULL);

    fclose(fp);
    png_destroy_write_struct(&png, &info);
}

void computePalette(unsigned char* inPalette, unsigned char* outPalette, int coef)
{
    int i;

    for (i = 0; i < 256; i++)
    {
        *(outPalette++) = ((*(inPalette++)) * coef) >> 8;
        *(outPalette++) = ((*(inPalette++)) * coef) >> 8;
        *(outPalette++) = ((*(inPalette++)) * coef) >> 8;
    }
}


Image loadBackground() {
    Image result;

    unsigned char palette[256 * 30];
    //unsigned char* palette = (unsigned char*)malloc(256 * 256);
    if (!LoadPak("ITD_RESS", 3, (char*)palette))
    {
        printf("Failed to load palette\n");
        return result;
    }

    //for (int i = 0; i < 256; i++)
    //{
    //    palette[i * 3 + 0] = palette[(255 - i) * 3 + 0];
    //    palette[i * 3 + 1] = palette[(255 - i) * 3 + 1];
    //    palette[i * 3 + 2] = palette[(255 - i) * 3 + 2];
    //}

    //unsigned char pal[256*3];
    //computePalette(palette, &pal[0], 240);
    //memcpy(&x, palette, 768);
    
    unsigned char* img = (unsigned char*)malloc(320 * 200);
    if (!LoadPak("CAMERA00", 0, (char*)img))
    {
        printf("Failed to load background\n");
        return result;
    }

    result.mipmaps = 1;
    result.width = 320;
    result.height = 200;
    result.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    result.data = malloc(320 * 200 * 3);

    unsigned char* c = (unsigned char*)result.data;
    for (int i = 0; i < (320 * 200); i++) {
        c[i * 3 + 0] = palette[img[i] * 3 + 0];
        c[i * 3 + 1] = palette[img[i] * 3 + 1];
        c[i * 3 + 2] = palette[img[i] * 3 + 2];
    }

    //free(palette);
    free(img);

    return result;
}