#pragma once
#include <stdio.h>   
#include <stdlib.h>
#include <string>

#include "structs/int_types.h"
#include "pak/pak.h"

inline bool paletteLoaded = false;
inline vector<u8> palette; //[256 * 30];

inline void loadPalette() {
    if (paletteLoaded) return;
    PakFile pak("original/ITD_RESS.PAK");
    palette = pak.readBlock(3);
    paletteLoaded = true;
}

inline u8* getPalColor(u8 idx) {
    loadPalette();
    return &palette.data()[idx*3];
}