#pragma once
#include "int_types.h"

//CAMERA

struct zoneStruct
{
	s16 zoneX1;
	s16 zoneZ1;
	s16 zoneX2;
	s16 zoneZ2;
};

struct cameraMaskV2Struct
{
	u16 numTestRect = 0;
	zoneStruct* rectTests;
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
	s16 pointsCount = 0;
	//x2 points - [x1,y1,x2,y2,...]
	//std::vector<s16> points;
	s16* points = nullptr;
};

struct cameraMaskV1Struct
{
	u16 numZone = 0;
	zoneStruct* zones;
	u16 numPolys = 0;
	cameraMaskV1PolyStruct* polys;
};

struct cameraViewedRoomStruct
{
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
