#pragma once
#include "int_types.h"
#include <vector>

struct ZVStruct
{
	s32 ZVX1;
	s32 ZVX2;
	s32 ZVY1;
	s32 ZVY2;
	s32 ZVZ1;
	s32 ZVZ2;
};

struct hardColStruct
{
	ZVStruct zv;
	u32 type;
	u32 parameter;
};

struct sceZoneStruct
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
	std::vector<sceZoneStruct> sceZoneTable;
	std::vector<u16> cameraIdxTable;
};
