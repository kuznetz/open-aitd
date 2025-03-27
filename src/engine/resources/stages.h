#pragma once
//#include <raymath.h>
#include <algorithm>
#include <fstream>

#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>
#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

#include "../raylib.h"

using nlohmann::json;
using namespace std;
namespace openAITD {

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
		auto& t = n.translation;
		auto& s = n.scale;
		BoundingBox b = {
			{ t[0], t[1], t[2] },
			{ t[0] + s[0], t[1] + s[1], t[1] + s[1]}
		};
		return b;
	}

	inline vector<Vector2> loadLineAcc2d(tinygltf::Model& m, int accIdx)
	{
		vector<Vector2> res;
		//TODO: Get camera Poly
		return res;
	}

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

	struct GCameraRoom {
		int roomId;
		vector<GCameraOverlay> overlays;
		vector<vector<Vector2>> coverZones;
	};

	class WCamera {
	public:
		std::vector<GCameraRoom> rooms;
		vector<vector<Vector2>> coverZones;
		Matrix modelview;
		Matrix prespective;

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
	private:
		void loadRooms(tinygltf::Model& model, json& stageJson);
		void loadCameras(tinygltf::Model& model, json& stageJson);

	public:
		string stageDir;
		std::vector<Room> rooms;
		std::vector<WCamera> cameras;

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


		loadRooms(model, stageJson);
		loadCameras(model, stageJson);
	}

	void Stage::loadRooms(tinygltf::Model& model, json& stageJson) {
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
				auto& collJson = stageJson["rooms"][roomId]["colliders"][collId];
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
				auto& collJson = stageJson["rooms"][roomId]["zones"][collId];
				coll.bounds = NodeToBounds(*collN);
				coll.parameter = collJson["parameter"];
				coll.type = collJson["type"];
				collId++;
			}

			//room.cameraIds = stageJson["rooms"][roomId]["cameras"].get<std::vector<int>>();

			roomId++;
		}
	}

	void Stage::loadCameras(tinygltf::Model& model, json& stageJson) {
		int cameraId = 0;
		while (true) {
			tinygltf::Node* cameraN = findNode(model, string("camera_") + to_string(cameraId));
			if (!cameraN) break;
			auto& cam = cameras.emplace_back();
			auto roomIds = stageJson["cameras"][cameraId]["rooms"].get<std::vector<int>>();
			auto& camPers = model.cameras[cameraN->camera].perspective;

			//TODO: Camera settings
			cam.prespective = MatrixPerspective(camPers.yfov, camPers.aspectRatio, camPers.znear, 100000);
			
			auto& r = cameraN->rotation;
			auto& t = cameraN->translation;
			Matrix m1 = QuaternionToMatrix({ (float)r[0], (float)r[1], (float)r[2], (float)r[3] });
			//m1.m12 = (float)t[0];
			//m1.m13 = (float)t[1];
			//m1.m14 = (float)t[2];
			cam.modelview = m1;
			
			//Matrix m2 = MatrixTranslate((float)t[0], (float)t[1], (float)t[2]);
			//cam.modelview = m2;

			//cam.modelview
			//room.position = { (float)cameraN->translation[0], (float)cameraN->translation[1], (float)cameraN->translation[2] };
			//cameraN->rotation
			//room.position = { (float)cameraN->translation[0], (float)cameraN->translation[1], (float)cameraN->translation[2] };

			for (int r = 0; r < roomIds.size(); r++) {
				auto& camRoom = cam.rooms.emplace_back();
				camRoom.roomId = roomIds[r];
				int overlayId = 0;
				while (true) {
					int overlayZoneId = 0;
					GCameraOverlay overlay;
					while (true) {
						tinygltf::Node* ovlZN = findNode(model, 
							string("overlay_zone_") + to_string(cameraId) + "_" + to_string(camRoom.roomId) + "_" +
							to_string(overlayId) + "_" + to_string(overlayZoneId)
						);
						if (!ovlZN) break;						
						overlay.bounds.push_back(NodeToBounds(*ovlZN));
						overlayZoneId++;
					}
					if (overlay.bounds.size()) {
						camRoom.overlays.push_back(overlay);
					}
					else {
						break;
					}
					overlayId++;
				}

				int coverZoneId = 0;
				while (true) {
					tinygltf::Node* coverZoneN = findNode(model,
						string("cam_zone_") + to_string(cameraId) + "_" + to_string(camRoom.roomId) + "_" +
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