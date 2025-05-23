﻿#pragma once
#include "int_types.h"
#include <vector>

using namespace std;

typedef enum ModelPrimitiveType {
	Line = 0,
	Polygon = 1,
	Pixel = 2,
	Sphere = 3,
	Ign1 = 4,
	Ign2 = 5,
	Square2x2 = 6,
	SquareNxN = 7,
	TriTex = 8,
	TriNorm = 9,
	TriNormTex = 10,
} ModelPrimitiveType;

typedef struct PakModelPrimitive {
	u8 type = 0;
	u8 subType = 0;
	u8 colorIndex = 0;
	vector<u16> vertexIdxs;
	u16 size;
	//triangles
	u16 uvIndex = 0;
	u8 texModel = 0;
	vector<u16> uv;
} PakModelPrimitive;

typedef struct ModelBone {
	int fromVertexIdx;
	int vertexCount;
	int rootVertexIdx;
	int parentBoneIdx;
	int boneIdx;
} ModelBone;

typedef struct PakModel {
	u16 modelFlags;
	ZVStruct bounds;
	std::vector <s16> vertices;
	std::vector <ModelBone> bones;	
	std::vector <PakModelPrimitive> primitives;
} PakModel;
