#include <stdio.h>   
#include <stdlib.h>
#include <string>

#include "../structs/int_types.h"
#include "../pak/pak.h"

bool paletteLoaded = false;
vector<u8> palette; //[256 * 30];

void loadPalette() {
    if (paletteLoaded) return;
    PakFile pak("original/ITD_RESS.PAK");
    palette = pak.readBlock(3);
    paletteLoaded = true;
}

u8* getPalColor(u8 idx) {
    loadPalette();
    return &palette.data()[idx*3];
}