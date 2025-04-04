#pragma once
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

	inline BoundingBox getObjectBounds(GameObject& obj, int boundsType) {
		if (obj.model.id == -1) return defaultObjBounds;
		switch (boundsType)
		{
		case 1:
			//Simple Body
			break;
		case 3:
			//RotatedBody
			break;
		case 2:
			//CubeBody
			//getZvCube(bodyPtr, zvPtr);
			break;
			//getZvRot(bodyPtr, zvPtr, alpha, beta, gamma);
			break;
		default:
			throw new exception("Unsupported boundsType");
		}
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

		bool processColliders2(const Vector3& p, Vector3& v, BoundingBox& b) {
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

		void processColliders(GameObject& gobj, Room& room) {
			Vector3& p = gobj.location.position;
			auto& m = *resources->models.getModel(gobj.model.id);
			Vector3 v = gobj.location.moveVec;
			Vector3 v2;

			for (int i = 0; i < room.colliders.size(); i++) {
				if (!v.x && !v.y && !v.z) continue;

				//TODO: cache for not movable objs
				BoundingBox& objB = getCubeBounds(m.bounds);
				objB.min = Vector3Add(objB.min, p);
				objB.max = Vector3Add(objB.max, p);

				BoundingBox& colB = room.colliders[i].bounds;

				if (
					(objB.min.y > colB.max.y || objB.max.y < colB.min.y) &&
					(colB.min.y > objB.max.y || colB.max.y < objB.min.y)
					)  continue;

				bool c = false;
				c = processColliders2({ objB.min.x, 0, objB.min.z }, v, colB) || c;
				c = processColliders2({ objB.max.x, 0, objB.min.z }, v, colB) || c;
				c = processColliders2({ objB.min.x, 0, objB.max.z }, v, colB) || c;
				c = processColliders2({ objB.max.x, 0, objB.max.z }, v, colB) || c;

				v2 = Vector3Invert(v);

				bool c2 = false;
				c2 = processColliders2({ colB.min.x, 0, colB.min.z }, v2, objB) || c2;
				c2 = processColliders2({ colB.max.x, 0, colB.min.z }, v2, objB) || c2;
				c2 = processColliders2({ colB.min.x, 0, colB.max.z }, v2, objB) || c2;
				c2 = processColliders2({ colB.max.x, 0, colB.max.z }, v2, objB) || c2;

				v = Vector3Invert(v2);

				if (c) {
					printf("c");
				}
				if (c2) {
					printf("c2");
				}
			}

			gobj.location.moveVec = v;
		}

		void process(float timeDelta) {
			auto& curStage = resources->stages[world->curStageId];
			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj = world->gobjects[i];
				if (gobj.location.stageId != world->curStageId) continue;
				if (Vector3Equals(gobj.location.moveVec, {0,0,0})) continue;
				auto* curRoom = &curStage.rooms[world->curRoomId];

				processColliders(gobj, *curRoom);
				gobj.location.position = Vector3Add(gobj.location.position, gobj.location.moveVec);
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