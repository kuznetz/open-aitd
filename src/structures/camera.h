#pragma once
#include "int_types.h"
#include <vector>

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
	std::vector<zoneStruct> rectTests;
};

struct cameraZonePointStruct
{
	s16 x;
	s16 y;
};

struct cameraZoneEntryStruct
{
	std::vector<cameraZonePointStruct> pointTable;
};

struct cameraMaskV1PolyStruct
{
	//x2 points - [x1,y1,x2,y2,...]
	std::vector <s16> points;
};

struct cameraMaskV1Struct
{
	std::vector < zoneStruct> zones;
	std::vector <cameraMaskV1PolyStruct> polys;
};

struct cameraViewedRoomStruct
{
	s16 viewedRoomIdx = 0;
	s16 lightX;
	s16 lightY;
	s16 lightZ;

	//AITD1 masks
	std::vector <cameraMaskV1Struct> overlays_V1;
	//Others masks
	std::vector<cameraMaskV2Struct> V2masks;
	std::vector<cameraZoneEntryStruct> coverZones;
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

	std::vector<cameraViewedRoomStruct> viewedRoomTable;
};
