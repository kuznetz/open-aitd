#pragma once
#include "int_types.h"

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
	int offset;
	int end;

	s16 worldX;
	s16 worldY;
	s16 worldZ;
	u32 numHardCol = 0;
	hardColStruct* hardColTable;
	u32 numSceZone = 0;
	sceZoneStruct* sceZoneTable;
	u32 numCameraInRoom = 0;
	u16* cameraIdxTable;
};
