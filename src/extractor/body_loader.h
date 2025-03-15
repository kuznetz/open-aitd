#pragma once
#include <string>
#include "structs/body.h"
#include "pak/pak.h"

int loadBodyBones(PakBody &body, char* data) {
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

void loadBodyPrimitives(PakBody& body, char* data) {
	int i = 0;
	int primCount = READ_LE_U16(data + 0);
	i += 2;
	body.primitives.resize(primCount);
	for (int n = 0; n < primCount; n++) {
		auto& prim = body.primitives[n];
		prim.type = data[i + 0];
		i++;
		switch (prim.type)
		{
		/*
		//line
		case 0:
		{
			i++;
			int colorIndex = buffer[i + 0];
			i += 2;

			Color32 color = paletteColors[colorIndex];
			int pointIndexA = buffer.ReadShort(i + 0) / 6;
			int pointIndexB = buffer.ReadShort(i + 2) / 6;
			Vector3 directionVector = vertices[pointIndexA] - vertices[pointIndexB];
			Vector3 middle = (vertices[pointIndexA] + vertices[pointIndexB]) / 2.0f;
			Quaternion rotation = Quaternion.LookRotation(directionVector);

			uv.AddRange(CubeMesh.uv);
			uvDepth.AddRange(CubeMesh.vertices.Select(x = > Vector2.zero));
			indices[0].AddRange(CubeMesh.triangles.Select(x = > x + allVertices.Count));
			allVertices.AddRange(CubeMesh.vertices.Select(x = >
				rotation * Vector3.Scale(x, new Vector3(linesize, linesize, directionVector.magnitude))
				+ middle));
			colors.AddRange(CubeMesh.vertices.Select(x = > color));
			colorsRaw.AddRange(CubeMesh.vertices.Select(x = > color));
			boneWeights.AddRange(CubeMesh.vertices.Select(x = > new BoneWeight(){ boneIndex0 = bonesPerVertex[x.z > 0 ? pointIndexA : pointIndexB], weight0 = 1 }));

			i += 4;
			break;
		}
		*/
		//polygon
		case 1:
		{
			int numPoints = READ_LE_U8(data + i);
			prim.polyType = READ_LE_U8(data + i + 1);
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
		/*
		//sphere
		case 3:
		{
			int polyType = buffer[i];
			i++;
			int colorIndex = buffer[i];
			Color32 color = Palette.GetPaletteColor(paletteColors, colorIndex, polyType, DetailsLevel.BoolValue);
			Color32 colorRaw = Palette.GetRawPaletteColor(paletteColors, colorIndex, polyType);
			List<int> triangleList = indices[GetTriangleListIndex(polyType)];

			i += 2;

			int size = buffer.ReadShort(i + 0);
			i += 2;
			int pointSphereIndex = buffer.ReadShort(i + 0) / 6;
			i += 2;

			Vector3 position = vertices[pointSphereIndex];
			float scale = size / 500.0f;
			float uvScale = noisesize * size / 200.0f;

			if ((polyType == 3 || polyType == 4 || polyType == 5 || polyType == 6) && DetailsLevel.BoolValue)
			{
				gradientPolygonType.Add(polyType);
				gradientPolygonList.Add(Enumerable.Range(allVertices.Count, SphereMesh.vertices.Length).ToList());
			}

			uv.AddRange(SphereMesh.uv.Select(x = > x * uvScale));
			uvDepth.AddRange(SphereMesh.vertices.Select(x = > Vector2.zero));
			triangleList.AddRange(SphereMesh.triangles.Select(x = > x + allVertices.Count));
			allVertices.AddRange(SphereMesh.vertices.Select(x = > x * scale + position));
			colors.AddRange(SphereMesh.vertices.Select(x = > color));
			colorsRaw.AddRange(SphereMesh.vertices.Select(x = > colorRaw));
			boneWeights.AddRange(SphereMesh.vertices.Select(x = > new BoneWeight(){ boneIndex0 = bonesPerVertex[pointSphereIndex], weight0 = 1 }));
			break;
		}
		*/
		/*
		case 2: //1x1 pixel
		case 6: //2x2 square
		case 7: //NxN square, size depends projected z-value
		{
			i++;
			int colorIndex = buffer[i];
			i += 2;
			int cubeIndex = buffer.ReadShort(i + 0) / 6;
			i += 2;

			Color32 color = paletteColors[colorIndex];
			Vector3 position = vertices[cubeIndex];

			float pointsize = linesize;
			switch (prim.type)
			{
			case 6:
				pointsize = linesize * 2.5f;
				break;
			case 7:
				pointsize = linesize * 5.0f;
				break;
			}

			uv.AddRange(CubeMesh.uv);
			uvDepth.AddRange(CubeMesh.vertices.Select(x = > Vector2.zero));
			indices[0].AddRange(CubeMesh.triangles.Select(x = > x + allVertices.Count));
			allVertices.AddRange(CubeMesh.vertices.Select(x = > x * pointsize + position));
			colors.AddRange(CubeMesh.vertices.Select(x = > color));
			colorsRaw.AddRange(CubeMesh.vertices.Select(x = > color));
			boneWeights.AddRange(CubeMesh.vertices.Select(x = > new BoneWeight(){ boneIndex0 = bonesPerVertex[cubeIndex], weight0 = 1 }));
			break;
		}
		*/
		
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

void loadBody(char* data, int size) {
	//header
	PakBody body;
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
		body.vertices[j + 0] = READ_LE_S16(data + i + 0);
		body.vertices[j + 1] = READ_LE_S16(data + i + 2);
		body.vertices[j + 2] = READ_LE_S16(data + i + 4);
		i += 6;
	}

	//check if model has bones
	if ((body.modelFlags & 2) == 2)
	{
		i += loadBodyBones(body, data + i);
	}
	loadBodyPrimitives(body, data + i);
}
