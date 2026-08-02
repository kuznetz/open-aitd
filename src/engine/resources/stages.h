#pragma once
#include <algorithm>
#include <fstream>
#include <vector>

#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>
#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

#include "../../common/raylib_cpp.hpp"
#include "../../common/metrics.hpp"
#include "bounds.h"

using nlohmann::json;
using namespace raylib;
using namespace std;
namespace openAITD {

	inline bool isPointInPoly(const Vector2 p, const vector<Vector2>& polygon) {
		size_t n = polygon.size();
		bool result = false;
		for (size_t i = 0; i < n; ++i) {
			size_t j = (i + 1) % n;
			if (
				// Does p0.y lies in half open y range of edge.
				// N.B., horizontal edges never contribute
				((polygon[j].y <= p.y && p.y < polygon[i].y) ||
					(polygon[i].y <= p.y && p.y < polygon[j].y)) &&
				// is p to the left of edge?
				(p.x < polygon[j].x + (polygon[i].x - polygon[j].x) * (p.y - polygon[j].y) /
					(polygon[i].y - polygon[j].y))
				)
				result = !result;
		}
		return result;
	}

	inline tinygltf::Node* findNode(tinygltf::Model& m, string name)
	{
		for (int i = 0; i < m.nodes.size(); i++) {
			if (m.nodes[i].name == name) return &m.nodes[i];
		}
		return 0;
	}

	inline Bounds NodeToBounds(tinygltf::Node& n)
	{
		auto t = n.translation;
		auto s = n.scale;
		if (s[0] < 0) {	s[0] = -s[0]; t[0] -= s[0];	}
		if (s[1] < 0) {	s[1] = -s[1]; t[1] -= s[1]; }
		if (s[2] < 0) {	s[2] = -s[2]; t[2] -= s[2];	}
		Bounds b(
			{ (float)t[0], (float)t[1], (float)t[2] },
			{ (float)(t[0] + s[0]), (float)(t[1] + s[1]), (float)(t[2] + s[2])}
		);
		b.correctBounds();
		return b;
	}

	inline vector<Vector2> loadLineAcc2d(tinygltf::Model& m, int accIdx)
	{
		vector<Vector2> res;		
		auto& acc = m.accessors[accIdx];
		auto& bufVW = m.bufferViews[acc.bufferView];
		char* data = (char*)m.buffers[bufVW.buffer].data.data() + bufVW.byteOffset;
		float* dataf = (float*)data;
		for (int i = 0; i < acc.count; i++) {
			res.push_back({ dataf[0], dataf[2] });
			dataf += 3;
		}
		return res;
	}

	struct RoomCollider
	{
		Bounds bounds;
		int type; // 1 - simple, 3 - climbing,  9 - linked
		int parameter;
		int linkedObjectId = -1;
	};

	enum class RoomZoneType {
		ChangeRoom = 0,
		Trigger = 9,
		ChangeStage = 10
	};

	struct RoomZone
	{
		Bounds bounds;
		RoomZoneType type;
		int parameter;
	};

	struct Room {
		Vector3 origPosition;
		vector<RoomCollider> colliders;
		vector<RoomZone> zones;
	};

	struct GCameraOverlay {
		vector<Bounds> bounds;
	};

	struct GCameraRoom {
		int roomId;
		vector<GCameraOverlay> overlays;
		//vector<vector<Vector2>> coverZones;
	};

	class WCamera {
	public:
		//For test
		Vector3 position;
		Vector4 rotation;
		tinygltf::PerspectiveCamera pers;

		vector<GCameraRoom> rooms;
		vector<vector<Vector2>> coverZones;
		Matrix modelview;
		Matrix perspective;

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
		vector<Room> rooms;
		vector<WCamera> cameras;

		void load(string stageDir);
		bool pointInCamera(const Vector2 p, WCamera& camera);
		int closestCamera(Vector3 p);
		//int centredCamera(Vector3 p);

		Vector3 VectorChangeRoom(const Vector3 v, int fromRoomId, int toRoomId);
		Bounds BoundsChangeRoom(const Bounds b, int fromRoomId, int toRoomId);
	};

	void Stage::load(string stageDir) {
		this->stageDir = stageDir;
		ifstream ifs(stageDir + "/stage.json");
		json stageJson = json::parse(ifs);

		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		string err;
		string warn;
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
			room.origPosition = { (float)roomN->translation[0], (float)roomN->translation[1], (float)roomN->translation[2] };

			int collId = 0;
			while (true) {
				tinygltf::Node* collN = findNode(model, string("coll_") + to_string(roomId) + "_" + to_string(collId));
				if (!collN) break;
				auto& coll = room.colliders.emplace_back();
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
				auto& zone = room.zones.emplace_back();
				auto& zoneJson = stageJson["rooms"][roomId]["zones"][collId];
				zone.bounds = NodeToBounds(*collN);
				zone.parameter = zoneJson["parameter"];
				zone.type = zoneJson["type"];
				collId++;
			}

			//room.cameraIds = stageJson["rooms"][roomId]["cameras"].get<vector<int>>();

			roomId++;
		}
	}

	void Stage::loadCameras(tinygltf::Model& model, json& stageJson) {
		int cameraId = 0;
		while (true) {
			tinygltf::Node* cameraN = findNode(model, string("camera_") + to_string(cameraId));
			if (!cameraN) break;
			auto& cam = cameras.emplace_back();
			auto roomIds = stageJson["cameras"][cameraId]["rooms"].get<vector<int>>();
			auto& camPers = model.cameras[cameraN->camera].perspective;

			cam.pers = camPers;
			//cam.perspective = MatrixPerspective(camPers.yfov, camPers.aspectRatio, camPers.znear, camPers.zfar);

			auto& r = cameraN->rotation;
			auto& t = cameraN->translation;
			cam.position = { (float)t[0], (float)t[1], (float)t[2] };
			cam.rotation = { (float)r[0], (float)r[1], (float)r[2], (float)r[3] };
			Matrix m1 = QuaternionToMatrix({ (float)r[0], (float)r[1], (float)r[2], (float)r[3] });
		    //Matrix m1 = MatrixRotateY( 0*PI );
			Matrix m2 = MatrixTranslate((float)t[0], (float)t[1], (float)t[2]);
			cam.modelview = MatrixMultiply( m2, m1 );

			//cam.modelview
			//room.position = { (float)cameraN->translation[0], (float)cameraN->translation[1], (float)cameraN->translation[2] };
			//cameraN->rotation
			//room.position = { (float)cameraN->translation[0], (float)cameraN->translation[1], (float)cameraN->translation[2] };

			for (int r = 0; r < roomIds.size(); r++) {
				auto& camRoom = cam.rooms.emplace_back();
				camRoom.roomId = roomIds[r];
				auto& room = rooms[camRoom.roomId];
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
						auto b = NodeToBounds(*ovlZN);
						b.max.y = b.min.y + 1;
						overlay.bounds.push_back(b);
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
					auto zone = loadLineAcc2d(model, lineAccIdx);
					for (int z = 0; z < zone.size(); z++) {
						zone[z] += {room.origPosition.x, room.origPosition.z};
					}
					cam.coverZones.push_back(zone);
					coverZoneId++;
				}

			}
			cameraId++;
		}

	}

	bool Stage::pointInCamera(const Vector2 p, WCamera& camera)
	{
		for (int i = 0; i < camera.coverZones.size(); i++) {
			if (isPointInPoly(p, camera.coverZones[i])) {
				return true;
			}
		}
		/*for (int r = 0; r < camera.rooms.size(); r++) {
			auto& camRoom = camera.rooms[r];
			for (int i = 0; i < camRoom.coverZones.size(); i++) {
				if (isPointInPoly(p, camRoom.coverZones[i])) {
					return true;
				}
			}
		}*/
		return false;		
	}

	int Stage::closestCamera(Vector3 p)
	{
		int result = -1;
		float sqrDist = 0;
		for (int cId = 0; cId < cameras.size(); cId++) {
			auto& cam = cameras[cId];
			if (!pointInCamera({p.x, p.z}, cam)) continue;
			float curDist = Vector3DistanceSqr(p, cam.position);
			if (sqrDist == 0 || sqrDist > curDist) {
				sqrDist = curDist;
				result = cId;
			}
		}
		return result;
	}

	Vector3 Stage::VectorChangeRoom(const Vector3 v, int fromRoomId, int toRoomId) {
		if (fromRoomId == toRoomId) return v;
		auto& roomFrom = rooms[fromRoomId];
		auto& roomTo   = rooms[toRoomId];
		return Vector3Subtract( Vector3Add(v, roomFrom.origPosition), roomTo.origPosition);
	}

	Bounds Stage::BoundsChangeRoom(const Bounds b, int fromRoomId, int toRoomId) {
		if (fromRoomId == toRoomId) return b;
		auto& roomFrom = rooms[fromRoomId].origPosition;
		auto& roomTo = rooms[toRoomId].origPosition;
		return {
			Vector3Subtract(Vector3Add(b.min, roomFrom), roomTo),
			Vector3Subtract(Vector3Add(b.max, roomFrom), roomTo)
		};
	}	

}