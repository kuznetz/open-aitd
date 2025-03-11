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

struct cameraPointStruct
{
	s16 x;
	s16 y;
};

struct cameraZoneEntryStruct
{
	std::vector<cameraPointStruct> pointTable;
};

typedef std::vector<cameraPointStruct> cameraOverlayPolygon;
struct cameraOverlayV1Struct
{
	std::vector <zoneStruct> zones;
	std::vector <cameraOverlayPolygon> polygons;
};

struct cameraViewedRoomStruct
{
	s16 viewedRoomIdx = 0;
	s16 lightX;
	s16 lightY;
	s16 lightZ;

	//AITD1 masks
	std::vector <cameraOverlayV1Struct> overlays_V1;
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

	s16 nearDistance;
	s16 fovX;
	s16 fovY;

	std::vector<cameraViewedRoomStruct> viewedRoomTable;
};
