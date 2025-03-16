#pragma once
#include <string>
#include "structs/model.h"
#include "pak/pak.h"

int loadModelBones(PakModel &body, char* data) {
	int i = 0;
	int bonesCount = READ_LE_U16(data + 0);
	i += 2;
	//??? - Skip Offsets
	i += bonesCount * 2;
	body.bones.resize(bonesCount);
	for (int n = 0; n < bonesCount; n++)
	{
		auto& b = body.bones[n];
		b.fromVertexIdx = READ_LE_U16(data + i + 0);
		b.vertexCount   = READ_LE_U16(data + i + 2);
		b.rootVertexIdx = READ_LE_U16(data + i + 4);
		b.parentBone    = READ_LE_U8(data + i + 6);
		b.curBoneIdx    = READ_LE_U8(data + i + 7);
		i += 8;

		if ((body.modelFlags & 8) == 8)
		{
			//??? - Skip
			i += 16;
		}
		else
		{
			//??? - Skip
			i += 8;
		}
	}
	return i;
}

void loadModelPrimitives(PakModel& body, char* data) {
	int i = 0;
	int primCount = READ_LE_U16(data + 0);
	i += 2;
	body.primitives.resize(primCount);
	for (int primIdx = 0; primIdx < primCount; primIdx++) {
		auto& prim = body.primitives[primIdx];
		prim.type		= READ_LE_U8(data + i);
		i += 1;
		switch (prim.type)
		{
		//line
		case 0:
		{
			int numPoints = READ_LE_U8(data + i);
			prim.subType = READ_LE_U8(data + i + 1);
			prim.colorIndex = READ_LE_U8(data + i + 2);
			i += 3;

			prim.vertexIdxs.resize(2);
			prim.vertexIdxs[0] = READ_LE_U16(data + i);
			prim.vertexIdxs[1] = READ_LE_U16(data + i + 2);
			i += 4;
			break;
		}
		//polygon
		case 1:
		{
			int numPoints = READ_LE_U8(data + i);
			prim.subType = READ_LE_U8(data + i + 1);
			prim.colorIndex = READ_LE_U8(data + i + 2);
			i += 3;

			prim.vertexIdxs.resize(numPoints);
			for (int m = 0; m < numPoints; m++)
			{
				prim.vertexIdxs[m] = READ_LE_U16(data + i);
				i += 2;
			}
			break;
		}
		//sphere
		case 3:
		{
			prim.subType = READ_LE_U8(data + i + 0);
			i++;
			prim.colorIndex = READ_LE_U8(data + i);
			//???
			i += 2;
			prim.size = READ_LE_S16(data + i);
			i += 2;
			prim.vertexIdxs.resize(1);
			prim.vertexIdxs[0] = READ_LE_U16(data + i);
			i += 2;
			break;
		}
		case 2: //1x1 pixel
		case 6: //2x2 square
		case 7: //NxN square, size depends projected z-value
		{
			prim.subType = READ_LE_U8(data + i + 0);
			prim.colorIndex = READ_LE_U8(data + i + 1);
			i += 3;
			prim.vertexIdxs.resize(1);
			prim.vertexIdxs[0] = READ_LE_U16(data + i);
			i += 2;
			break;
		}
		//triangle
		case 8:  //texture
		case 9:  //normals
		case 10: //normals + texture
		{
			if (prim.type == 8 || prim.type == 10) {
				//uvOffset = uvStart + buffer.ReadUnsignedShort(i + 1) / 16 * 3;
				u16 uvOffset = READ_LE_U16(data + i);
			}
			prim.colorIndex = READ_LE_U8(data + i + 2);
			i += 3;

			prim.vertexIdxs.resize(3);
			for (int k = 0; k < 3; k++)
			{
				i += 2;
				prim.vertexIdxs[k] = READ_LE_U16(data + i + 0);
				if (prim.type == 8 || prim.type == 10)
				{
					/*uv.Add(new Vector2(
						buffer.ReadShort(uvIndex + 0) / 256.0f,
						buffer.ReadShort(uvIndex + 2) / textureHeight));
					uvIndex += 4;*/
				}
			}
			//??? - skip 3 bytes
			i += 3;
			if (prim.type == 9 || prim.type == 10)
			{
				i += 6; //normals
			}
		}
		break;

		case 4:
		case 5: //should be ignored
			break;

		default:
			auto m = std::string("unknown primitive ") + std::to_string(prim.type) + " at " + std::to_string(i);
			throw new std::exception(m.c_str());
		}

	}
}

PakModel loadModel(char* data, int size) {
	//header
	PakModel body;
	body.modelFlags = READ_LE_U16(data);	
	body.bounds.ZVX1 = READ_LE_S16(data + 2);
	body.bounds.ZVX2 = READ_LE_S16(data + 4);
	body.bounds.ZVY1 = READ_LE_S16(data + 6);
	body.bounds.ZVY2 = READ_LE_S16(data + 8);
    body.bounds.ZVZ1 = READ_LE_S16(data + 10);
	body.bounds.ZVZ2 = READ_LE_S16(data + 12);
	int vertsOffs = READ_LE_S16(data + 14) + 16;

	int vertsCount = READ_LE_S16(data + vertsOffs);
	int i = vertsOffs+2;
	body.vertices.resize(vertsCount*3);
	for (int j = 0; j < vertsCount; j++)
	{
		body.vertices[j * 3 + 0] = READ_LE_S16(data + i + 0);
		body.vertices[j * 3 + 1] = READ_LE_S16(data + i + 2);
		body.vertices[j * 3 + 2] = READ_LE_S16(data + i + 4);
		i += 6;
	}

	//check if model has bones
	if ((body.modelFlags & 2) == 2)
	{
		i += loadModelBones(body, data + i);
	}
	loadModelPrimitives(body, data + i);
	return body;
}
