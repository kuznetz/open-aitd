#pragma once
#include "int_types.h"
#include <vector>

struct hardColStruct
{
	ZVStruct zv;
	u32 type;
	u32 parameter;
};

struct roomStruct
{
	s16 worldX;
	s16 worldY;
	s16 worldZ;
	std::vector<hardColStruct> hardColTable;
	std::vector<hardColStruct> sceZoneTable;
	std::vector<u16> cameraIdxTable;
};
