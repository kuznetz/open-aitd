#pragma once
#include "int_types.h"
#include <vector>

struct PakModelPrimitive {
	u8 type = 0;
	u8 subType = 0;
	u8 colorIndex = 0;
	vector <u16> vertexIdxs;
	u16 size;
	//triangles
	u16 uvIndex = 0;
	u8 texModel = 0;
	vector<u16> uv;
};

struct ModelBone {
	int fromVertexIdx;
	int vertexCount;
	int rootVertexIdx;
	int parentBone;
	int curBoneIdx;
};

struct PakModel {
	u16 modelFlags;
	ZVStruct bounds;
	std::vector <s16> vertices;
	std::vector <ModelBone> bones;	
	std::vector <PakModelPrimitive> primitives;
};