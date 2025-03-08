#pragma once
#include "png.h"
#include <exception>

void savePng(const char* filename, int width, int height, void* data, int color_type = PNG_COLOR_TYPE_RGB);