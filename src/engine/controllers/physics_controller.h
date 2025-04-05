﻿#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;
namespace openAITD {

	inline BoundingBox getCubeBounds(BoundingBox& b)
	{
		BoundingBox r = b;
		r.max.z = r.max.x = (b.max.x + b.max.z) / 2;
		r.min.z = r.min.x = -r.max.z;
		return r;
	}

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

		bool PointToBox(const Vector3& p, Vector3& v, BoundingBox& b) {
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

		bool BoxToBox(BoundingBox& b1, Vector3& v, BoundingBox& b2) {
			if (
				(b1.min.y > b2.max.y || b1.max.y < b2.min.y) &&
				(b2.min.y > b1.max.y || b2.max.y < b1.min.y)
			)  return false;

			bool c = false;
			c = PointToBox({ b1.min.x, 0, b1.min.z }, v, b2) || c;
			c = PointToBox({ b1.max.x, 0, b1.min.z }, v, b2) || c;
			c = PointToBox({ b1.min.x, 0, b1.max.z }, v, b2) || c;
			c = PointToBox({ b1.max.x, 0, b1.max.z }, v, b2) || c;

			auto v2 = Vector3Negate(v);
			bool c2 = false;
			c2 = PointToBox({ b2.min.x, 0, b2.min.z }, v2, b1) || c2;
			c2 = PointToBox({ b2.max.x, 0, b2.min.z }, v2, b1) || c2;
			c2 = PointToBox({ b2.min.x, 0, b2.max.z }, v2, b1) || c2;
			c2 = PointToBox({ b2.max.x, 0, b2.max.z }, v2, b1) || c2;

			v = Vector3Negate(v2);

			//if (c) printf("c");
			//if (c2) printf("c2");
			return c || c2;
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

		BoundingBox getObjectBounds(GameObject& gobj) {
			if (gobj.physics.boundsCached) {
				return gobj.physics.bounds;
			}
			Vector3& p = gobj.location.position;
			auto& m = *resources->models.getModel(gobj.model.id);
			BoundingBox objB = correctBounds(m.bounds);
			if (gobj.model.boundsType == GOModel::BoundsType::cube) {
				objB = getCubeBounds(objB);
			}
			if (gobj.model.boundsType == GOModel::BoundsType::rotated) {
				objB.min = Vector3RotateByQuaternion(objB.min, gobj.location.rotation);
				objB.max = Vector3RotateByQuaternion(objB.max, gobj.location.rotation);
				objB = correctBounds(objB);
			}
			objB.min = Vector3Add(objB.min, p);
			objB.max = Vector3Add(objB.max, p);
			if (!gobj.physics.boundsCached) {
				gobj.physics.bounds = objB;
				gobj.physics.boundsCached = true;
			}
			return objB;
		}

		void processColliders(GameObject& gobj, Room& room) {
			BoundingBox& objB = getObjectBounds(gobj);
			Vector3 v = gobj.physics.moveVec;
			for (int i = 0; i < room.colliders.size(); i++) {
				if (!v.x && !v.y && !v.z) continue;
				BoundingBox& colB = room.colliders[i].bounds;
				BoxToBox(objB, v, colB);
			}

			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj2 = world->gobjects[i];
				if (&gobj == &gobj2) continue;
				if (gobj2.model.id == -1) continue;
				if (gobj2.location.stageId != gobj.location.stageId) continue;
				if (gobj2.location.roomId != gobj.location.roomId) continue;
				BoundingBox& objB2 = getObjectBounds(gobj2);
				bool collided = BoxToBox(objB, v, objB2);
				if (collided) {
					gobj.physics.collidedBy = gobj2.id;
					gobj2.physics.collidedBy = gobj.id;
				}
			}

			gobj.physics.moveVec = v;
		}

		void process(float timeDelta) {
			auto& curStage = resources->stages[world->curStageId];

			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj = world->gobjects[i];
				if (gobj.location.stageId != world->curStageId) continue;
				gobj.physics.collidedBy = -1;
				if (Vector3Equals(gobj.physics.moveVec, { 0,0,0 })) continue;
				gobj.physics.boundsCached = false;
			}

			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj = world->gobjects[i];
				if (gobj.location.stageId != world->curStageId) continue;
				if (Vector3Equals(gobj.physics.moveVec, {0,0,0})) continue;
				auto* curRoom = &curStage.rooms[world->curRoomId];

				processColliders(gobj, *curRoom);
				gobj.location.position = Vector3Add(gobj.location.position, gobj.physics.moveVec);

				//Check Zones
				for (int i = 0; i < curRoom->zones.size(); i++) {
					if (!objectInZone(gobj, &curRoom->zones[i])) continue;
					if (curRoom->zones[i].type == RoomZone::RoomZoneType::ChangeRoom) {
						Vector3 oldRoomPos = curRoom->position;
						gobj.location.roomId = curRoom->zones[i].parameter;
						curRoom = &curStage.rooms[gobj.location.roomId];
						gobj.location.position = Vector3Subtract(Vector3Add(gobj.location.position, oldRoomPos), curRoom->position);
					}
				}			
			}
		}
	};

}