#pragma once
#include "my_gltf.h"

const Vector3 cubeVecs[6][3] = {
	{{-1, 1,-1}, {0, 0, 2}, {2, 0, 0}}, //Top
	{{-1,-1,-1}, {0, 0, 2}, {2, 0, 0}}, //Bot
	{{-1,-1, 1}, {2, 0, 0}, {0, 2, 0}}, //Front
	{{-1,-1,-1}, {2, 0, 0}, {0, 2, 0}}, //Back
	{{ 1,-1,-1}, {0, 0, 2}, {0, 2, 0}}, //Right
	{{-1,-1,-1}, {0, 0, 2}, {0, 2, 0}}  //Left
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

void setSphereMeshFace(vector<unsigned int>& faceIdxs, int vertCount, int offs, int vertOffs) {
	int k = offs;
	for (int i = 0; i < vertCount-1; i++) {
		for (int j = 0; j < vertCount-1; j++) {
			faceIdxs[k++] = vertOffs + (i * vertCount) + j;
			faceIdxs[k++] = vertOffs + (i * vertCount) + j + 1;
			faceIdxs[k++] = vertOffs + ((i + 1) * vertCount) + j;
			faceIdxs[k++] = vertOffs + (i * vertCount) + j + 1;
			faceIdxs[k++] = vertOffs + ((i + 1) * vertCount) + j + 1;
			faceIdxs[k++] = vertOffs + ((i + 1) * vertCount) + j;
		}
	}
}

tinygltf::Primitive createSpherePrim(tinygltf::Model& m, float radius, int vertCount, Vector3 pos, int material) {
	int sideVCount = vertCount * vertCount;
	int sideFaceICount = (vertCount-1) * (vertCount-1) * 2 * 3;
	vector<Vector3> vertexes(sideVCount * 6);
	vector<unsigned int> faces(sideFaceICount * 6);
	for (int i = 0; i < 6; i++) {
		setSphereMeshSide(vertexes, cubeVecs[i][0], cubeVecs[i][1], cubeVecs[i][2], radius, vertCount, sideVCount * i);
		setSphereMeshFace(faces, vertCount, sideFaceICount * i, sideVCount * i);
	}
	for (int i = 0; i < vertexes.size(); i++) {
		vertexes[i] = Vector3Add(vertexes[i], pos);
	}
	
	int vertAccIdx = createVertexes(m, vertexes);
	return createPolyPrimitive(m, faces, vertAccIdx, material);
}
