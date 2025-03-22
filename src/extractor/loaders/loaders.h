#pragma once
#include "../pak/pak.h"
#include "../structs/model.h"
#include "../structs/int_types.h"
#include "../structs/game_objects.h"
#include "../structs/floor.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>
#include <vector>
using namespace std;

u8* getPalColor(u8 idx);
vector<gameObjectStruct> loadGameObjects(string from);
PakModel loadModel(char* data, int size);
floorStruct loadFloorPak(string filename);