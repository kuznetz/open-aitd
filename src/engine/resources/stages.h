#pragma once
//#include <raymath.h>
#include <algorithm>
#include <fstream>
#include <vector>

#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>
#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

#include "../raylib.h"
#include "bounds.h"

using nlohmann::json;
using namespace raylib;
using namespace std;
namespace openAITD {

	/*inline int isInPoly(int x1, int x2, int z1, int z2, cameraViewedRoomStruct* pCameraZoneDef)
	{
		int xMid = (x1 + x2) / 2;
		int zMid = (z1 + z2) / 2;

		int i;

		for (i = 0; i < pCameraZoneDef->numCoverZones; i++)
		{
			int j;
			int flag = 0;

			for (j = 0; j < pCameraZoneDef->coverZones[i].numPoints; j++)
			{
				int zoneX1;
				int zoneZ1;
				int zoneX2;
				int zoneZ2;

				zoneX1 = pCameraZoneDef->coverZones[i].pointTable[j].x;
				zoneZ1 = pCameraZoneDef->coverZones[i].pointTable[j].y;
				zoneX2 = pCameraZoneDef->coverZones[i].pointTable[j + 1].x;
				zoneZ2 = pCameraZoneDef->coverZones[i].pointTable[j + 1].y;

				if (testCrossProduct(xMid, zMid, xMid - 10000, zMid, zoneX1, zoneZ1, zoneX2, zoneZ2))
				{
					flag |= 1;
				}

				if (testCrossProduct(xMid, zMid, xMid + 10000, zMid, zoneX1, zoneZ1, zoneX2, zoneZ2))
				{
					flag |= 2;
				}
			}

			if (flag == 3)
			{
				return(1);
			}
		}

		return(0);
	}*/

	/*
	Not works
	inline bool isPointInPoly(const Vector2 p, const vector<Vector2>& polygon)
	{
		float minX = polygon[0].x;
		float maxX = polygon[0].x;
		float minY = polygon[0].y;
		float maxY = polygon[0].y;
		for (int i = 1; i < polygon.size(); i++)
		{
			auto& q = polygon[i];
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
	*/

	/*
	inline bool isPointInPoly(const Vector2 p, const vector<Vector2>& polygon)
	{
		char flag;
		Vector2 p1;
		Vector2 p2 = polygon[polygon.size() - 1];
		for (int j = 0; j < polygon.size(); j++) {
			p1 = p2;
			p2 = polygon[j];
		}
		if (flag == 3)
		{
			return true;
		}
		return false;
	}
	*/

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
		int objectLink = -1;
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
		Vector3 position;
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
			room.position = { (float)roomN->translation[0], (float)roomN->translation[1], (float)roomN->translation[2] };

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
						zone[z] += {room.position.x, room.position.z};
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

}