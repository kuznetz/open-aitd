#pragma once
#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>
#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>
#include <vector>
#include <raymath.h>
#include <cstring>

using namespace std;

extern const uint8_t cubeIndices[];
extern const float cubeVertices[];
extern const int cubeVertSize;

typedef struct VertexSkin {
	unsigned long long jointsAccIdx;
	unsigned long long weightsAccIdx;
} VertexSkin;

int addDataToBuffer(tinygltf::Model& m, void* data, int dataSize);
int createBufferAndView(tinygltf::Model& m, void* data, int size, int vwTarget);
int createCubeMesh(tinygltf::Model& m);
int createLineMesh(tinygltf::Model& m, const vector<float>& line);
int createPolyMesh(tinygltf::Model& m, const vector<Vector3>& vertexes, const vector<unsigned int>& indices);
int createVertexes(tinygltf::Model& m, const vector<Vector3>& vertexes);
const VertexSkin addVertexSkin(tinygltf::Model& m, vector<unsigned char> vecBoneAffect);
tinygltf::Primitive createPolyPrimitive(tinygltf::Model& m, const vector<unsigned int>& indices, int vertAccIdx, int material);
tinygltf::Primitive createSpherePrim(tinygltf::Model& m, float radius, int vertCount, Vector3 pos, int material);
tinygltf::Primitive createPipePrim(tinygltf::Model& m, Vector3 points[], float radius, int sides, int material);
