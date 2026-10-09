#pragma once
#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>
#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>
#include <vector>
#include <cstring>
#include "../../../common/raylib_cpp.hpp"

namespace AITDExtractor {

	using namespace std;
	using namespace raylib;

	inline tinygltf::Value makePairExtras(int type, int parameter) {
		tinygltf::Value::Object obj;
		obj["type"] = tinygltf::Value(type);
		obj["parameter"] = tinygltf::Value(parameter);
		return tinygltf::Value(obj);
	}

	inline tinygltf::Value makeRoomsExtras(const vector<int>& rooms) {
		tinygltf::Value::Array arr;
		arr.reserve(rooms.size());
		for (size_t i = 0; i < rooms.size(); i++) {
			arr.push_back(tinygltf::Value(rooms[i]));
		}
		tinygltf::Value::Object obj;
		obj["rooms"] = tinygltf::Value(arr);
		return tinygltf::Value(obj);
	}

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
	tinygltf::Primitive createCubePrim(tinygltf::Model& m, Vector3 center, Vector3 size, int material);

}