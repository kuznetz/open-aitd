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


inline bool isPointInPoly(Vector2 p, vector<Vector2> polygon)
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

inline BoundingBox NodeToBounds(tinygltf::Node& n)
{
	BoundingBox b;
	//n.translation
	//n.scale
	return b;
}

inline vector<Vector2> loadLineAcc2d(tinygltf::Model& m, int accIdx)
{
	vector<Vector2> res;
	//TODO: Get camera Poly
	return res;
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
				if (isPointInPoly(p, poly)) {
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
		std::ifstream ifs(stageDir + "/stage.json");
		json stageJson = json::parse(ifs);

		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;
		bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, stageDir + "/stage.gltf");

		int roomId = 0;
		while (true) {
			tinygltf::Node* roomN = findNode(model, string("room_") + to_string(roomId));
			if (!roomN) break;
			auto& room = rooms.emplace_back();
			room.position = { (float)roomN->translation[0], (float)roomN->translation[1], (float)roomN->translation[2] };

			int collId = 0;
			while (true) {
				tinygltf::Node* collN = findNode(model, string("coll_") + to_string(roomId) + "_" + to_string(collId));
				if (!collN) break;
				auto& coll = room.colliders.emplace_back();
				coll.isZone = false;
				auto& collJson = stageJson["rooms"][roomId]["colliders"];
			    coll.bounds = NodeToBounds(*collN);
				coll.parameter = collJson["parameter"];
				coll.type = collJson["type"];
				collId++;
			}

			collId = 0;
			while (true) {
				tinygltf::Node* collN = findNode(model, string("zone_") + to_string(roomId) + "_" + to_string(collId));
				if (!collN) break;
				auto& coll = room.colliders.emplace_back();
				coll.isZone = true;
				auto& collJson = stageJson["rooms"][roomId]["zones"];
				coll.bounds = NodeToBounds(*collN);
				coll.parameter = collJson["parameter"];
				coll.type = collJson["type"];
				collId++;
			}
						
			//room.cameraIds = stageJson["rooms"][roomId]["cameras"].get<std::vector<int>>();

			roomId++;
		}

		int cameraId = 0;
		while (true) {
			tinygltf::Node* cameraN = findNode(model, string("camera_") + to_string(cameraId));
			if (!cameraN) break;
			auto& cam = cameras.emplace_back();
			cam.roomIds = stageJson["cameras"][cameraId]["rooms"].get<std::vector<int>>();

			//TODO: Camera settings
			//cam.modelview
			//cam.prespective
			//room.position = { (float)cameraN->translation[0], (float)cameraN->translation[1], (float)cameraN->translation[2] };
			//cameraN->rotation
			//room.position = { (float)cameraN->translation[0], (float)cameraN->translation[1], (float)cameraN->translation[2] };

			for (int r = 0; r < cam.roomIds.size(); r++) {
				int roomId = cam.roomIds[r];			

				int overlayId = 0;
				while (true) {
					int overlayZoneId = 0;
					GCameraOverlay overlay;
					while (true) {
						tinygltf::Node* ovlZN = findNode(model, 
							string("overlay_zone_") + to_string(cameraId) + "_" + to_string(roomId) + "_" +
							to_string(overlayId) + "_" + to_string(overlayZoneId)
						);
						if (!ovlZN) break;
						overlay.bounds.push_back(NodeToBounds(*ovlZN));
						overlayZoneId++;
					}
					if (overlay.bounds.size()) {
						cam.overlays.push_back(overlay);
					}
					else {
						break;
					}
					overlayId++;
				}

				int coverZoneId = 0;
				while (true) {
					tinygltf::Node* coverZoneN = findNode(model,
						string("cam_zone_") + to_string(cameraId) + "_" + to_string(roomId) + "_" +
						to_string(coverZoneId)
					);
					if (!coverZoneN) break;

					int lineAccIdx = model.meshes[coverZoneN->mesh].primitives[0].attributes["POSITION"];
					cam.coverZones.push_back(loadLineAcc2d(model, lineAccIdx));

					coverZoneId++;
				}

			}
			cameraId++;
		}

	}

}