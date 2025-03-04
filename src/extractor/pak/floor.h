#ifndef _ROOM_H_
#define _ROOM_H_

#include "common.h"

//ROOM

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


//CAMERA

struct rectTestStruct
{
	s16 zoneX1;
	s16 zoneZ1;
	s16 zoneX2;
	s16 zoneZ2;
};

struct cameraMaskV2Struct
{
	u16 numTestRect = 0;
	rectTestStruct* rectTests;
};

struct cameraZonePointStruct
{
	s16 x;
	s16 y;
};

struct cameraZoneEntryStruct
{
	u16 numPoints = 0;
	cameraZonePointStruct* pointTable;
};

struct cameraMaskV1PolyStruct
{
	u16 pointsCount = 0;
	//x2 points - [x1,y1,x2,y2,...]
	//std::vector<s16> points;
	s16* points = nullptr;
};

struct cameraMaskV1ZoneStruct
{
	u16 polyCount = 0;
	cameraMaskV1PolyStruct* polys;
};

struct cameraMaskV1Struct
{
	u16 numZone = 0;
	cameraMaskV1ZoneStruct* zones;
};

struct cameraViewedRoomStruct
{
	int offset;
	int end;

	s16 viewedRoomIdx = 0;
	s16 lightX;
	s16 lightY;
	s16 lightZ;

	//AITD1 masks
	u16 numV1Mask = 0;
	cameraMaskV1Struct* V1masks;
	//Others masks
	u16 numV2Mask = 0;
	cameraMaskV2Struct* V2masks;
	u16 numCoverZones = 0;
	cameraZoneEntryStruct* coverZones;
};

struct cameraStruct
{
	int offset;
	int end;

	s16 alpha;
	s16 beta;
	s16 gamma;

	s16 x;
	s16 y;
	s16 z;

	s16 focal1;
	s16 focal2;
	s16 focal3;

	u16 numViewedRooms = 0;
	cameraViewedRoomStruct* viewedRoomTable;
};

struct floorStruct
{
	u32 rawDataSize = 0;
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

floorStruct* loadFloor(char* filename);

#endif
