#pragma once
#include <raylib.h>
#include <algorithm>
#include <fstream>

#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>
using nlohmann::json;
using namespace std;
#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>


inline bool IsPointInPoly(Vector2 p, vector<Vector2> polygon)
{
	float minX = polygon[0].x;
	float maxX = polygon[0].x;
	float minY = polygon[0].y;
	float maxY = polygon[0].y;
	for (int i = 1; i < polygon.size(); i++)
	{
		Vector2& q = polygon[i];
		minX = min(q.x, minX);
		maxX = max(q.x, maxX);
		minY = min(q.y, minY);
		maxY = max(q.y, maxY);
	}

	if (p.x < minX || p.x > maxX || p.y < minY || p.y > maxY)
	{
		return false;
	}

	// https://wrf.ecse.rpi.edu/Research/Short_Notes/pnpoly.html
	bool inside = false;
	int j = polygon.size() - 1;
	for (int i = 0; i < polygon.size(); i++)
	{
		if ((polygon[i].y > p.y) != (polygon[j].y > p.y) &&
			p.x < (polygon[j].x - polygon[i].x) * (p.y - polygon[i].y) / (polygon[j].y - polygon[i].y) + polygon[i].x)
		{
			inside = !inside;
		}
		j = i;
	}

	return inside;
}

inline tinygltf::Node* findNode(tinygltf::Model& m, string name)
{
	for (int i = 0; i < m.nodes.size(); i++) {
		if (m.nodes[i].name == name) return &m.nodes[i];
	}
	return 0;
}


using namespace std;
namespace openAITD {

	struct RoomCollider
	{
		BoundingBox bounds;
		//TODO: enums
		bool isZone;
		int type;
		int parameter;
	};

	struct Room {
		Vector3 position;
		std::vector<RoomCollider> colliders;
		std::vector<int> cameraIds;
	};

	struct GCameraOverlay {
		vector<BoundingBox> bounds;
	};

	class GCamera {
	public:
		std::vector<int> roomIds;
		vector<GCameraOverlay> overlays;
		vector<vector<Vector2>> coverZones;
		Matrix modelview;
		Matrix prespective;
		//MatrixPerspective(fovRes.fov * testFovK * DEG2RAD, cameraAspect, nearDist, CAMERA_CULL_DISTANCE_FAR);

		bool IsPointInCamera(Vector2 p)
		{
			for (int i = 0; i < coverZones.size(); i++) {
				auto& poly = coverZones[i];
				if (IsPointInPoly(p, poly)) {
					return true;
				}
			}
			return false;
		}

		Vector2 WorldToScreen(Vector3 p)
		{
			return { 0,0 };
		}
	};

	//Store static data in game
	class Stage {
	public:
		string stageDir;
		std::vector<Room> rooms;
		std::vector<GCamera> cameras;

		void load(string stageDir);
		//int centredCamera(Vector3 p);
		//int closestCamera(Vector3 p);
	};

	void Stage::load(string stageDir) {
		this->stageDir = stageDir;
		std::ifstream ifs(stageDir + "/scene.json");
		json stageJson = json::parse(ifs);

		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;
		bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, stageDir + "/scene.gltf");

		this->cameras.resize(stageJson["cameras"].size());
		for (int i = 0; i < stageJson["cameras"].size(); i++) {
			auto n = findNode(model, string("camera_") + to_string(i));

			
			//auto& cam = model.cameras[i];

		}

		this->rooms.resize(stageJson["rooms"].size());
		for (int i = 0; i < stageJson["rooms"].size(); i++) {
			auto n = findNode(model, string("room_") + to_string(i));
			//auto& cam = model.cameras[i];

		}


	}

}