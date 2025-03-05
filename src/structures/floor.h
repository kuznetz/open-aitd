#pragma once

#include "int_types.h"
#include "room.h"
#include "camera.h"

struct floorStruct
{
	u32 cameraCount = 0;
	s16 roomCount = 0;
	roomStruct* rooms;
	cameraStruct* cameras;
};

//PAK
//struct roomDefStruct
//{
//	s16 offsetToCameraDef; // 0
//	s16 offsetToPosDef; // 2
//	s16 worldX;//4
//	s16 worldY;//6
//	s16 worldZ;//8
//	s16 numCameraInRoom;//0xA
//};


//struct roomDataStruct
//{
//  u32 numCameraInRoom;
//
//  u32 numHardCol;
//  hardColStruct* hardColTable;
//
//  u32 numSceZone;
//  sceZoneStruct* sceZoneTable;
//
//  s32 worldX;
//  s32 worldY;
//  s32 worldZ;
//
//  u16* cameraIdxTable;
//};
//typedef struct roomDataStruct roomDataStruct;

//extern cameraDataStruct* cameraDataTable[NUM_MAX_CAMERA_IN_ROOM];
//extern cameraViewedRoomStruct* currentCameraZoneList[NUM_MAX_CAMERA_IN_ROOM];