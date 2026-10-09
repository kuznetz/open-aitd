#pragma once
#include <algorithm>
#include <cstring>
#include <fstream>
#include <vector>

#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

#include "../../common/raylib_cpp.hpp"
#include "../../common/metrics.hpp"
#include "bounds.h"

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

	inline int findNodeIndex(const tinygltf::Model& m, const string& name)
	{
		for (int i = 0; i < (int)m.nodes.size(); i++) {
			if (m.nodes[i].name == name) return i;
		}
		return -1;
	}

	// Builds node local transform as T * R * S, matching the transform order
	// used by the rest of the engine (e.g. camera modelview).
	inline Matrix nodeLocalMatrix(const tinygltf::Node& n)
	{
		if (n.matrix.size() == 16) {
			// glTF stores node.matrix in the same column-major layout raylib uses.
			return Matrix{
				(float)n.matrix[0],  (float)n.matrix[1],  (float)n.matrix[2],  (float)n.matrix[3],
				(float)n.matrix[4],  (float)n.matrix[5],  (float)n.matrix[6],  (float)n.matrix[7],
				(float)n.matrix[8],  (float)n.matrix[9],  (float)n.matrix[10], (float)n.matrix[11],
				(float)n.matrix[12], (float)n.matrix[13], (float)n.matrix[14], (float)n.matrix[15]
			};
		}
		// tinygltf only fills TRS vectors that are present in the file, so an
		// absent component stays empty: fall back to identity values instead of
		// indexing out of range.
		Vector3 t = { 0, 0, 0 };
		Vector3 s = { 1, 1, 1 };
		Quaternion q = { 0, 0, 0, 1 };
		if (n.translation.size() >= 3) {
			t = { (float)n.translation[0], (float)n.translation[1], (float)n.translation[2] };
		}
		if (n.scale.size() >= 3) {
			s = { (float)n.scale[0], (float)n.scale[1], (float)n.scale[2] };
		}
		if (n.rotation.size() >= 4) {
			q = { (float)n.rotation[0], (float)n.rotation[1], (float)n.rotation[2], (float)n.rotation[3] };
		}
		// raylib composes with MatrixMultiply(A, B) == B * A, exactly like
		// DrawModelEx does: MatrixMultiply(MatrixMultiply(S, R), T) yields the
		// classic T * R * S transform (scale first, then rotate, then translate).
		return MatrixMultiply(
			MatrixMultiply(MatrixScale(s.x, s.y, s.z), QuaternionToMatrix(q)),
			MatrixTranslate(t.x, t.y, t.z)
		);
	}

	// Local AABB of a VEC3 accessor: min/max when present, otherwise raw buffer
	// read respecting byteOffset/byteStride.
	inline bool accessorVec3Bounds(const tinygltf::Model& m, const tinygltf::Accessor& acc, Bounds& out)
	{
		if (acc.type != TINYGLTF_TYPE_VEC3) return false;

		if (acc.minValues.size() >= 3 && acc.maxValues.size() >= 3) {
			out.min = { (float)acc.minValues[0], (float)acc.minValues[1], (float)acc.minValues[2] };
			out.max = { (float)acc.maxValues[0], (float)acc.maxValues[1], (float)acc.maxValues[2] };
			return true;
		}

		if (acc.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) return false;
		if (acc.sparse.isSparse) return false;
		if (acc.bufferView < 0 || acc.bufferView >= (int)m.bufferViews.size()) return false;

		const auto& vw = m.bufferViews[acc.bufferView];
		if (vw.buffer < 0 || vw.buffer >= (int)m.buffers.size()) return false;

		const auto& data = m.buffers[vw.buffer].data;
		const size_t elementSize = 3 * sizeof(float);
		const size_t stride = vw.byteStride ? (size_t)vw.byteStride : elementSize;
		const size_t base = (size_t)vw.byteOffset + (size_t)acc.byteOffset;
		if (acc.count <= 0) return false;
		if (base + (size_t)(acc.count - 1) * stride + elementSize > data.size()) return false;

		const uint8_t* ptr = data.data() + base;
		for (int i = 0; i < acc.count; i++) {
			float f[3];
			memcpy(f, ptr + (size_t)i * stride, sizeof(f));
			Vector3 p = { f[0], f[1], f[2] };
			if (i == 0) { out.min = out.max = p; }
			else { out.min = Vector3Min(out.min, p); out.max = Vector3Max(out.max, p); }
		}
		return true;
	}

	inline bool meshLocalBounds(const tinygltf::Model& m, int meshIdx, Bounds& out)
	{
		if (meshIdx < 0 || meshIdx >= (int)m.meshes.size()) return false;
		const auto& mesh = m.meshes[meshIdx];
		bool any = false;
		for (size_t p = 0; p < mesh.primitives.size(); p++) {
			const auto& prim = mesh.primitives[p];
			auto posIt = prim.attributes.find("POSITION");
			if (posIt == prim.attributes.end()) continue;
			const int accIdx = posIt->second;
			if (accIdx < 0 || accIdx >= (int)m.accessors.size()) continue;
			Bounds pb;
			if (!accessorVec3Bounds(m, m.accessors[accIdx], pb)) continue;
			if (!any) { out = pb; any = true; }
			else { out.min = Vector3Min(out.min, pb.min); out.max = Vector3Max(out.max, pb.max); }
		}
		return any;
	}

	inline Bounds transformBounds(const Bounds& b, const Matrix& transform)
	{
		auto corners = b.getCorners();
		Vector3 p = Vector3Transform(corners[0], transform);
		Bounds r(p, p);
		for (int i = 1; i < 8; i++) {
			p = Vector3Transform(corners[i], transform);
			r.min = Vector3Min(r.min, p);
			r.max = Vector3Max(r.max, p);
		}
		return r;
	}

	// Collects geometry bounds of a node subtree: every mesh of the subtree is
	// measured and transformed by its accumulated transform. Returns false when
	// the subtree contains no readable geometry at all.
	inline bool nodeGeometryBounds(const tinygltf::Model& m, int nodeIdx, const Matrix& parent, Bounds& out, int depth)
	{
		if (nodeIdx < 0 || nodeIdx >= (int)m.nodes.size()) return false;
		if (depth > 32) return false;

		const auto& n = m.nodes[nodeIdx];
		// Accumulate as MatrixMultiply(local, parent) == parent * local, so the
		// child transform is applied first and the parent transform after it.
		const Matrix local = MatrixMultiply(nodeLocalMatrix(n), parent);

		bool any = false;
		if (n.mesh >= 0) {
			Bounds mb;
			if (meshLocalBounds(m, n.mesh, mb)) {
				out = transformBounds(mb, local);
				any = true;
			}
		}

		for (size_t c = 0; c < n.children.size(); c++) {
			Bounds cb;
			if (!nodeGeometryBounds(m, n.children[c], local, cb, depth + 1)) continue;
			if (!any) { out = cb; any = true; }
			else { out.min = Vector3Min(out.min, cb.min); out.max = Vector3Max(out.max, cb.max); }
		}
		return any;
	}

	// Geometry based bounds: AABB of the node subtree mesh geometry. A unit cube
	// scaled/translated (legacy stages produced by floor_extractor_2) yields
	// exactly the same box as the old translation/scale math, including negative
	// scale, so no format versioning or markers are needed.
	inline Bounds NodeToBounds(const tinygltf::Model& m, int nodeIdx, const Matrix& parent = MatrixIdentity())
	{
		Bounds b;
		if (!nodeGeometryBounds(m, nodeIdx, parent, b, 0)) {
			const char* name = (nodeIdx >= 0 && nodeIdx < (int)m.nodes.size()) ? m.nodes[nodeIdx].name.c_str() : "?";
			TraceLog(LOG_WARNING, "NodeToBounds: no readable geometry for node %d '%s'", nodeIdx, name);
			return Bounds({ 0, 0, 0 }, { 0, 0, 0 });
		}
		return b;
	}

	// Reads {type, parameter} extras of collider/zone nodes; keeps defaults when
	// the node carries no extras object at all.
	inline void readPairExtras(const tinygltf::Value& extras, int& type, int& parameter)
	{
		if (!extras.IsObject()) return;
		if (extras.Has("type")) type = extras.Get("type").GetNumberAsInt();
		if (extras.Has("parameter")) parameter = extras.Get("parameter").GetNumberAsInt();
	}

	inline vector<Vector2> loadLineAcc2d(const tinygltf::Model& m, int accIdx)
	{
		vector<Vector2> res;
		if (accIdx < 0 || accIdx >= (int)m.accessors.size()) return res;
		auto& acc = m.accessors[accIdx];
		if (acc.type != TINYGLTF_TYPE_VEC3) return res;
		if (acc.bufferView < 0 || acc.bufferView >= (int)m.bufferViews.size()) return res;
		auto& bufVW = m.bufferViews[acc.bufferView];
		if (bufVW.buffer < 0 || bufVW.buffer >= (int)m.buffers.size()) return res;
		const auto& data = m.buffers[bufVW.buffer].data;
		const size_t elementSize = 3 * sizeof(float);
		const size_t stride = bufVW.byteStride ? (size_t)bufVW.byteStride : elementSize;
		const size_t base = (size_t)bufVW.byteOffset + (size_t)acc.byteOffset;
		if (acc.count <= 0) return res;
		if (base + (size_t)(acc.count - 1) * stride + elementSize > data.size()) return res;
		const uint8_t* ptr = data.data() + base;
		for (int i = 0; i < acc.count; i++) {
			float f[3];
			memcpy(f, ptr + (size_t)i * stride, sizeof(f));
			res.push_back({ f[0], f[2] });
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
		void loadRooms(tinygltf::Model& model);
		void loadCameras(tinygltf::Model& model);

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

		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		string err;
		string warn;
		bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, stageDir + "/stage.gltf");


		loadRooms(model);
		loadCameras(model);
	}

	void Stage::loadRooms(tinygltf::Model& model) {
		int roomId = 0;
		while (true) {
			tinygltf::Node* roomN = findNode(model, string("room_") + to_string(roomId));
			if (!roomN) break;
			auto& room = rooms.emplace_back();
			room.origPosition = { (float)roomN->translation[0], (float)roomN->translation[1], (float)roomN->translation[2] };

			int collId = 0;
			while (true) {
				int collIdx = findNodeIndex(model, string("coll_") + to_string(roomId) + "_" + to_string(collId));
				if (collIdx < 0) break;
				auto& coll = room.colliders.emplace_back();
				coll.bounds = NodeToBounds(model, collIdx);
				int type = 0;
				int parameter = 0;
				readPairExtras(model.nodes[collIdx].extras, type, parameter);
				coll.type = type;
				coll.parameter = parameter;
				collId++;
			}

			collId = 0;
			while (true) {
				int zoneIdx = findNodeIndex(model, string("zone_") + to_string(roomId) + "_" + to_string(collId));
				if (zoneIdx < 0) break;
				auto& zone = room.zones.emplace_back();
				zone.bounds = NodeToBounds(model, zoneIdx);
				int type = 0;
				int parameter = 0;
				readPairExtras(model.nodes[zoneIdx].extras, type, parameter);
				zone.type = (RoomZoneType)type;
				zone.parameter = parameter;
				collId++;
			}

			//room.cameraIds = stageJson["rooms"][roomId]["cameras"].get<vector<int>>();

			roomId++;
		}
	}

	void Stage::loadCameras(tinygltf::Model& model) {
		int cameraId = 0;
		while (true) {
			tinygltf::Node* cameraN = findNode(model, string("camera_") + to_string(cameraId));
			if (!cameraN) break;
			auto& cam = cameras.emplace_back();
			vector<int> roomIds;
			{
				const tinygltf::Value& roomsArr = cameraN->extras.Get("rooms");
				for (size_t i = 0; i < roomsArr.ArrayLen(); i++) {
					roomIds.push_back(roomsArr.Get(i).GetNumberAsInt());
				}
			}
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
						int ovlZIdx = findNodeIndex(model,
							string("overlay_zone_") + to_string(cameraId) + "_" + to_string(camRoom.roomId) + "_" +
							to_string(overlayId) + "_" + to_string(overlayZoneId)
						);
						if (ovlZIdx < 0) break;
						auto b = NodeToBounds(model, ovlZIdx);
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