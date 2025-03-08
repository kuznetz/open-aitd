#include "png.h"
#include <exception>

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
    
    unsigned char byteCount;
    switch (color_type) {
    case PNG_COLOR_TYPE_RGB:
        byteCount = 3;
        break;
    case PNG_COLOR_TYPE_GRAY:
        byteCount = 1;
        break;
    case PNG_COLOR_TYPE_RGBA:
        byteCount = 4;
        break;
    default:
        throw new std::exception("Invalid PNG_COLOR_TYPE");
    }

    for (int k = 0; k < height; k++) {
        //k = each line, k*width = pixels till first pixel of like k, *3 = RGB values
        rows[k] = ((png_bytep)data) + k * width * byteCount;
    }
    png_write_image(png, rows);
    delete[] rows;
    png_write_end(png, NULL);

    fclose(fp);
    png_destroy_write_struct(&png, &info);
}