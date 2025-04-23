#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;
namespace openAITD {

	class PhysicsController {
	public:
		World* world;
		Resources* resources;

		PhysicsController(Resources* res, World* world) {
			this->resources = res;
			this->world = world;
		}

		bool objectInZone(GameObject& gobj, RoomZone* zone) {
			auto& p = gobj.location.position;
			auto& b = zone->bounds;
			return ((p.x > b.min.x) && (p.x < b.max.x) && (p.z > b.min.z) && (p.z < b.max.z));
		}

		bool CollPointToBox(const Vector3& p, Vector3& v, BoundingBox& b) {
			Vector3& p2 = Vector3Add(p, v);
			auto inB = (p2.x > b.min.x) && (p2.x < b.max.x) && (p2.z > b.min.z) && (p2.z < b.max.z);
			if (!inB) return false;

			float halfX = (b.min.x + b.max.x) / 2;
			float halfZ = (b.min.z + b.max.z) / 2;

			float revX = (p2.x < halfX) ? (b.min.x - p2.x) : (b.max.x - p2.x);
			float revZ = (p2.z < halfZ) ? (b.min.z - p2.z) : (b.max.z - p2.z);

			if (abs(revX) < abs(revZ)) {
				v.x += revX;
			}
			else {
				v.z += revZ;
			}
			return true;
		}

		bool CollBoxToBox(BoundingBox& b1_0, Vector3& v, BoundingBox& b2) {
			BoundingBox b1 = { Vector3Add(b1_0.min, v), Vector3Add(b1_0.max, v) };
			if (
				(b1.min.x > b2.max.x || b1.max.x < b2.min.x) &&
				(b2.min.x > b1.max.x || b2.max.x < b1.min.x)
			)  return false;
			if (
				(b1.min.y > b2.max.y || b1.max.y < b2.min.y) &&
				(b2.min.y > b1.max.y || b2.max.y < b1.min.y)
			)  return false;
			if (
				(b1.min.z > b2.max.z || b1.max.z < b2.min.z) &&
				(b2.min.z > b1.max.z || b2.max.z < b1.min.z)
			)  return false;

			float revX = (v.x < 0) ? (b2.max.x - b1.min.x) : (b2.min.x - b1.max.x);
			float revZ = (v.z < 0) ? (b2.max.z - b1.min.z) : (b2.min.z - b1.max.z);
			if (abs(revX) < abs(revZ)) {
				v.x += revX;
			}
			else {
				v.z += revZ;
			}

			//bool c = false;
			//c = PointToBox({ b1_0.min.x, 0, b1_0.min.z }, v, b2) || c;
			//c = PointToBox({ b1_0.max.x, 0, b1_0.min.z }, v, b2) || c;
			//c = PointToBox({ b1_0.min.x, 0, b1_0.max.z }, v, b2) || c;
			//c = PointToBox({ b1_0.max.x, 0, b1_0.max.z }, v, b2) || c;

			//auto v2 = Vector3Negate(v);
			//bool c2 = false;
			//c2 = PointToBox({ b2.min.x, 0, b2.min.z }, v2, b1_0) || c2;
			//c2 = PointToBox({ b2.max.x, 0, b2.min.z }, v2, b1_0) || c2;
			//c2 = PointToBox({ b2.min.x, 0, b2.max.z }, v2, b1_0) || c2;
			//c2 = PointToBox({ b2.max.x, 0, b2.max.z }, v2, b1_0) || c2;
			/*v = Vector3Negate(v2);*/

			//if (c) printf("c");
			//if (c2) printf("c2");
			return true;
		}

		BoundingBox correctBounds(const BoundingBox& b) {
			BoundingBox r = b;
			if (b.min.x > b.max.x) {
				r.min.x = b.max.x;
				r.max.x = b.min.x;
			}
			if (b.min.y > b.max.y) {
				r.min.y = b.max.y;
				r.max.y = b.min.y;
			}
			if (b.min.z > b.max.z) {
				r.min.z = b.max.z;
				r.max.z = b.min.z;
			}
			return  r;
		}

		inline BoundingBox getCubeBounds(BoundingBox& b)
		{
			BoundingBox r = b;
			r.max.z = r.max.x = (b.max.x + b.max.z) / 2;
			r.min.z = r.min.x = -r.max.z;
			return r;
		}

		inline BoundingBox getRotatedBounds(const BoundingBox& b, const Quaternion& q)
		{
			Vector3 v[8];
			v[0] = { b.min.x, b.min.y, b.min.z };
			v[1] = { b.max.x, b.min.y, b.min.z };
			v[2] = { b.min.x, b.min.y, b.max.z };
			v[3] = { b.max.x, b.min.y, b.max.z };
			v[4] = { b.min.x, b.max.y, b.min.z };
			v[5] = { b.max.x, b.max.y, b.min.z };
			v[6] = { b.min.x, b.max.y, b.max.z };
			v[7] = { b.max.x, b.max.y, b.max.z };
			BoundingBox res;
			for (int i = 0; i < 8; i++) {
				v[i] = Vector3RotateByQuaternion(v[i], q);
				if (i == 0 || v[i].x < res.min.x) {
					res.min.x = v[i].x;
				}
				if (i == 0 || v[i].x > res.max.x) {
					res.max.x = v[i].x;
				}
				if (i == 0 || v[i].y < res.min.y) {
					res.min.y = v[i].y;
				}
				if (i == 0 || v[i].y > res.max.y) {
					res.max.y = v[i].y;
				}
				if (i == 0 || v[i].z < res.min.z) {
					res.min.z = v[i].z;
				}
				if (i == 0 || v[i].z > res.max.z) {
					res.max.z = v[i].z;
				}
			}
			return res;
		}

		BoundingBox getObjectBounds(GameObject& gobj) {
			if (gobj.physics.boundsCached) {
				return gobj.physics.bounds;
			}
			auto& m = *resources->models.getModel(gobj.modelId);
			BoundingBox& objB = gobj.physics.bounds;
			objB = correctBounds(m.bounds);
			if (gobj.boundsType == BoundsType::cube) {
				objB = getCubeBounds(objB);
			}
			if (gobj.boundsType == BoundsType::rotated) {
				objB = getRotatedBounds(objB, gobj.location.rotation);
			}
			Vector3& p = gobj.location.position;
			objB.min = Vector3Add(objB.min, p);
			objB.max = Vector3Add(objB.max, p);
			objB = correctBounds(objB);

			gobj.physics.boundsCached = true;
			return objB;
		}

		void processStaticColliders(GameObject& gobj, Room& room) {
			BoundingBox& objB = getObjectBounds(gobj);
			Vector3 v = gobj.physics.moveVec;
			for (int i = 0; i < room.colliders.size(); i++) {
				BoundingBox& colB = room.colliders[i].bounds;
				bool collided = CollBoxToBox(objB, v, colB);
				if (collided) {
					if (room.colliders[i].type == 9) {
						gobj.physics.staticColl = room.colliders[i].parameter;
					}
					else if (room.colliders[i].type == 3) {
						gobj.physics.staticColl = 255;
					}
				}
			}
			gobj.physics.moveVec = v;
		}

		void processDynamicColliders(GameObject& gobj, Room& room) {
			BoundingBox& objB = getObjectBounds(gobj);
			Vector3 v = gobj.physics.moveVec;

			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj2 = world->gobjects[i];
				if (&gobj == &gobj2) continue;
				if (gobj2.modelId == -1) continue;
				if (gobj2.location.stageId != gobj.location.stageId) continue;
				if (gobj2.location.roomId != gobj.location.roomId) {
					bool inConnRoom = false;
					for (int j = 0; j < room.zones.size(); j++) {
						if (room.zones[j].type != RoomZoneType::ChangeRoom) continue;
						if (gobj2.location.roomId == room.zones[j].parameter) {
							inConnRoom = true;
							break;
						}
					}
					if (!inConnRoom) continue;
				}

				BoundingBox& objB2 = world->BoundsChangeRoom(getObjectBounds(gobj2), gobj2.location.roomId, gobj.location.roomId);
				bool collided = CollBoxToBox(objB, v, objB2);
				if (collided) {
					gobj.physics.objectColl = gobj2.id;
					gobj2.physics.collidedBy = gobj.id;

					//takable
					if (gobj.track.mode == GOTrackMode::manual && gobj2.bitField.foundable && gobj2.invItem.foundTimeout < this->world->chrono) {
						world->foundItem = gobj2.id;
					}

					//TODO: pushable
				}
			}

			gobj.physics.moveVec = v;
		}

		void processZones(GameObject& gobj, Room* curRoom) {
			//Check Zones
			gobj.physics.zoneTriggered = -1;
			if (gobj.bitField.trigger) {
				for (int i = 0; i < curRoom->zones.size(); i++) {
					auto& curZone = curRoom->zones[i];
					if (!objectInZone(gobj, &curZone)) continue;
					if (curZone.type == RoomZoneType::ChangeRoom) {
						Vector3 oldRoomPos = curRoom->position;
						gobj.location.roomId = curZone.parameter;
						auto& newRoom = world->curStage->rooms[curZone.parameter];
						gobj.location.position = Vector3Subtract(Vector3Add(gobj.location.position, oldRoomPos), newRoom.position);
						gobj.physics.boundsCached = false;
						break;
					}
					if (curZone.type == RoomZoneType::Trigger) {
						gobj.physics.zoneTriggered = curZone.parameter;
					}
					if (curZone.type == RoomZoneType::ChangeStage) {
						if (gobj.stageLifeId != -1) {
							gobj.lifeId = gobj.stageLifeId;
							gobj.physics.zoneTriggered = curZone.parameter;
							//flagFloorChange = true;
						}
					}
				}
			}
		}

		void followCameraProcess() {
			if (!world->followTarget) return;
			auto& loc = world->followTarget->location;
			if (loc.stageId == -1) return;
			world->setCurRoom(loc.stageId, loc.roomId);
			//Select Camera
			Vector3 pos = Vector3Add(loc.position, world->curRoom->position);
			auto camId = world->curStage->closestCamera(pos);
			if (camId != -1) {
				world->curCameraId = camId;
			}
		}

		void process(float timeDelta) {
			auto& curStage = resources->stages[world->curStageId];

			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj = world->gobjects[i];
				if (gobj.location.stageId != world->curStageId) continue;
				gobj.physics.collidedBy = -1;
				gobj.physics.staticColl = -1;
				gobj.physics.objectColl = -1;

				if (
					Vector3Equals(gobj.physics.moveVec, { 0,0,0 }) &&
					(gobj.rotateAnim.timeEnd <= 0)
				) continue;
				gobj.physics.boundsCached = false;
			}

			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj = world->gobjects[i];
				if (gobj.location.stageId != world->curStageId) continue;

				gobj.physics.moveVec = Vector3RotateByQuaternion(Vector3Subtract(gobj.animation.moveRoot, gobj.animation.prevMoveRoot), gobj.location.rotation);
				gobj.animation.prevMoveRoot = gobj.animation.moveRoot;

				if (Vector3Equals(gobj.physics.moveVec, {0,0,0})) continue;
				auto* curRoom = &curStage.rooms[world->curRoomId];

				if (gobj.physics.collidable) { //gobj.bitField.special???
					processStaticColliders(gobj, *curRoom);
					processDynamicColliders(gobj, *curRoom);
				}

				gobj.location.position = Vector3Add(gobj.location.position, gobj.physics.moveVec);

				processZones(gobj, curRoom);
			}

			followCameraProcess();
		}
	};

}

//BoundingBox handleCollision(BoundingBox& startZv, BoundingBox& nextZv, BoundingBox& collZv)
//{
//	int flag = 0;
//	int var_8;
//	float halfX;
//	float halfZ;
//	int var_A;
//	int var_6;

//	if (startZv.max.x > collZv.min.x)
//	{
//		if (collZv.max.x <= startZv.min.x)
//		{
//			flag = 8;
//		}
//	}
//	else
//	{
//		flag = 4;
//	}

//	if (startZv.max.z > collZv.min.z)
//	{
//		if (startZv.min.z >= collZv.max.z)
//		{
//			flag |= 2;
//		}
//	}
//	else
//	{
//		flag |= 1;
//	}

//	if (flag == 5 || flag == 9 || flag == 6 || flag == 10)
//	{
//		var_8 = 2;
//	}
//	else
//	{
//		if (!flag)
//		{
//			var_8 = 0;

//			globHardColStepZ = 0;
//			globHardColStepX = 0;

//			return;
//		}
//		else
//		{
//			var_8 = 1;
//		}
//	}

//	halfX = (nextZv.min.x + nextZv.max.x) / 2;
//	halfZ = (nextZv.min.z + nextZv.max.z) / 2;

//	if (collZv.min.x > halfX)
//	{
//		var_A = 4;
//	}
//	else
//	{
//		if (collZv.max.x < halfX)
//		{
//			var_A = 0;
//		}
//		else
//		{
//			var_A = 8;
//		}
//	}

//	if (collZv.min.z > halfZ)
//	{
//		var_A |= 1;
//	}
//	else
//	{
//		if (collZv.max.z < halfZ)
//		{
//			var_A |= 0; // once again, not that much usefull
//		}
//		else
//		{
//			var_A |= 2;
//		}
//	}

//	if (var_A == 5 || var_A == 9 || var_A == 6 || var_A == 10)
//	{
//		var_6 = 2;
//	}
//	else
//	{
//		if (!var_A)
//		{
//			var_6 = 0;
//		}
//		else
//		{
//			var_6 = 1;
//		}
//	}

//	if (var_8 == 1)
//	{
//		hardColSuB1Sub1(flag);
//		return;
//	}

//	if (var_6 == 1 && (var_A & flag))
//	{
//		hardColSuB1Sub1(var_A);
//		return;
//	}

//	if (var_A == flag || flag == 15)
//	{
//		int Xmod = abs(nextZv.min.x - startZv.min.x); // recheck
//		int Zmod = abs(nextZv.min.z - startZv.min.z);

//		if (Xmod > Zmod)
//		{
//			globHardColStepZ = 0;
//		}
//		else
//		{
//			globHardColStepX = 0;
//		}
//	}
//	else
//	{
//		if (!var_6 || (var_6 == 1 && !(var_A & flag)))
//		{
//			globHardColStepZ = 0;
//			globHardColStepX = 0;
//		}
//		else
//		{
//			hardColSuB1Sub1(flag & var_A);
//		}
//	}
//}