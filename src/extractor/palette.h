#pragma once
#include <stdio.h>   
#include <stdlib.h>
#include <string>

#include "structs/int_types.h"
#include "pak/pak.h"

inline u8* paletteLoaded = false;
inline u8 palette[256 * 30];

inline void loadPalette() {
    if (paletteLoaded) return;
    if (!LoadPak("original/ITD_RESS", 3, (char*)palette))
    {
        printf("Failed to load palette\n");
        return;
    }
    paletteLoaded = true;
}

inline u8* getPalColor(u8 idx) {
    loadPalette();
    return &palette[idx*3];
}