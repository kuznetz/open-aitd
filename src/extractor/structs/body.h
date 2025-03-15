#pragma once
#include "int_types.h"
#include <vector>

struct PakBodyPrimitive {
	u8 type;
	vector <u16> vertexIdxs;
	//poly
	u8 polyType;
	//triangles
	u16 uvIndex;
	u8 texModel;
	u16 colorIndex;
	vector<u16> uv;
};

struct PakBody {
	u16 modelFlags;
	ZVStruct bounds;
	std::vector <s16> vertices;
	std::vector <PakBodyPrimitive> primitives;
};