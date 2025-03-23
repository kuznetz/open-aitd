#pragma once
#include "int_types.h"
#include <vector>

using namespace std;

typedef struct
{
    // 8
    s16 type;
    // A
    s16 delta[3];
    // 10 (AITD2+) if Info_optimise
    s16 rotateDelta[3];
} AniBone;

typedef struct
{
    u16 timestamp;
    s16 offset[3];
    std::vector<AniBone> bones;
} AniFrame;

typedef struct
{
    int id;
    std::vector<AniFrame> frames;
} Animation;
