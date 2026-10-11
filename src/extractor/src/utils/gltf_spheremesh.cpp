#pragma once
#include "my_gltf.h"

namespace AITDExtractor {

	const Vector3 cubeVecs[6][3] = {
		{{-1, 1,-1}, { 0, 0, 2}, { 2, 0, 0}}, //Top
		{{-1,-1,-1}, { 0, 0, 2}, { 2, 0, 0}}, //Bot
		{{-1,-1, 1}, { 2, 0, 0}, { 0, 2, 0}}, //Front
		{{-1,-1,-1}, { 2, 0, 0}, { 0, 2, 0}}, //Back
		{{-1,-1,-1}, { 0, 0, 2}, { 0, 2, 0}}, //Left
		{{ 1,-1,-1 },{ 0, 0, 2}, { 0, 2, 0}}, //Right
	};

	void setSphereMeshSide(vector<Vector3>& vertexes, Vector3 start, Vector3 vx, Vector3 vy, float radius, int vertCount, int offs) {
		for (int i = 0; i < vertCount; i++) {
			for (int j = 0; j < vertCount; j++) {
				auto v = Vector3Add(start, Vector3Add(
					Vector3Scale(vy, (float)i / (vertCount-1)),
					Vector3Scale(vx, (float)j / (vertCount-1))
				));
				float l = Vector3Length(v);
				vertexes[offs + (i * vertCount) + j] = {
					v.x * radius / l,
					v.y * radius / l,
					v.z * radius / l,
				};
			}
		}
	}

	void setSphereMeshFace(vector<unsigned int>& faceIdxs, int vertCount, int offs, int vertOffs, bool rev) {
		int k = offs;
		for (int i = 0; i < vertCount-1; i++) {
			for (int j = 0; j < vertCount-1; j++) {
				if (!rev) {
					faceIdxs[k++] = vertOffs + (i * vertCount) + j;
					faceIdxs[k++] = vertOffs + (i * vertCount) + j + 1;
					faceIdxs[k++] = vertOffs + ((i + 1) * vertCount) + j;
					faceIdxs[k++] = vertOffs + (i * vertCount) + j + 1;
					faceIdxs[k++] = vertOffs + ((i + 1) * vertCount) + j + 1;
					faceIdxs[k++] = vertOffs + ((i + 1) * vertCount) + j;
				}
				else {
					faceIdxs[k++] = vertOffs + (i * vertCount) + j;
					faceIdxs[k++] = vertOffs + ((i + 1) * vertCount) + j;
					faceIdxs[k++] = vertOffs + (i * vertCount) + j + 1;
					faceIdxs[k++] = vertOffs + (i * vertCount) + j + 1;
					faceIdxs[k++] = vertOffs + ((i + 1) * vertCount) + j;
					faceIdxs[k++] = vertOffs + ((i + 1) * vertCount) + j + 1;
				}
			}
		}
	}

	tinygltf::Primitive createSpherePrim(tinygltf::Model& m, float radius, int vertCount, Vector3 pos, int material, float uvScale) {
		int sideVCount = vertCount * vertCount;
		int sideFaceICount = (vertCount-1) * (vertCount-1) * 2 * 3;
		vector<Vector3> vertexes(sideVCount * 6);
		vector<unsigned int> faces(sideFaceICount * 6);
		for (int i = 0; i < 6; i++) {
			setSphereMeshSide(vertexes, cubeVecs[i][0], cubeVecs[i][1], cubeVecs[i][2], radius, vertCount, sideVCount * i);
			setSphereMeshFace(faces, vertCount, sideFaceICount * i, sideVCount * i, i % 2);
		}
		for (int i = 0; i < vertexes.size(); i++) {
			vertexes[i] = Vector3Add(vertexes[i], pos);
		}
		
		int vertAccIdx = createVertexes(m, vertexes);
		auto prim = createPolyPrimitive(m, faces, vertAccIdx, material);

		// Noise spheres (polyType 1) carry a noise texture. The reference loader
		// maps SphereMesh.uv scaled by uvScale = noisesize * size / 200. This
		// quad-sphere is built from 6 cube sides, so each side gets its own 0..1
		// UV grid (the equivalent of the sphere mesh unwrap).
		if (uvScale != 0.0f) {
			vector<Vector2> uvs(vertexes.size());
			for (int i = 0; i < 6; i++) {
				for (int y = 0; y < vertCount; y++) {
					for (int x = 0; x < vertCount; x++) {
						int idx = sideVCount * i + y * vertCount + x;
						uvs[idx] = {
							(float)x / (vertCount - 1) * uvScale,
							(float)y / (vertCount - 1) * uvScale
						};
					}
				}
			}
			prim.attributes["TEXCOORD_0"] = createVec2Accessor(m, uvs);
		}
		return prim;
	}

}