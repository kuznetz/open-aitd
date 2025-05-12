#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;
namespace openAITD {

	class ObjRotateController {
	public:
		World* world;
		Resources* resources;

		ObjRotateController(World* world) {
			this->world = world;
			this->resources = world->resources;
		}

		void process(float timeDelta) {
			auto& curStage = resources->stages[world->curStageId];

			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj = world->gobjects[i];
				if (gobj.location.stageId != world->curStageId) continue;
				if (gobj.rotateAnim.timeEnd <= 0) continue;

				auto oldRotate = gobj.location.rotation;
				auto oldTime = gobj.rotateAnim.curTime;

				//Calc new rotate
				auto& rot = gobj.rotateAnim;
				rot.curTime += timeDelta;
				if (rot.curTime == 0) {
					gobj.location.rotation = rot.from;
				}
				else if (rot.curTime >= rot.timeEnd) {
					//Animation end
					gobj.location.rotation = rot.to;
					gobj.location.rotOrig = rot.toOrig;
					rot.timeEnd = 0;
					continue;
				}
				else {
					gobj.location.rotation = QuaternionSlerp(rot.from, rot.to, rot.curTime / rot.timeEnd);
				}

				if (gobj.boundsType == BoundsType::rotated) {
					gobj.physics.boundsCached = false;
				}

				// && gobj.bitField.animated
				if (gobj.boundsType == BoundsType::rotated) {
					auto* curRoom = &curStage.rooms[gobj.location.roomId];

					Bounds& objB = world->getObjectBounds(gobj);
					bool collided = false;
					for (int i = 0; i < world->gobjects.size(); i++) {
						auto& gobj2 = world->gobjects[i];
						if (&gobj == &gobj2) continue;
						if (gobj2.modelId == -1) continue;
						if (gobj2.location.stageId != gobj.location.stageId) continue;

						Bounds objB2 = world->getObjectBounds(gobj2);
						if (gobj2.location.roomId != gobj.location.roomId) {
							if (resources->isRoomsConnected(*world->curStage, gobj.location.roomId, gobj2.location.roomId)) {
								objB2 = world->BoundsChangeRoom(objB2, gobj2.location.roomId, gobj.location.roomId);
							}
							else {
								continue;
							}
						}
						bool c = objB.CollToBox(objB2);
						collided = collided || c;
						if (c) {
							gobj.physics.objectColl = gobj2.id;
							gobj2.physics.collidedBy = gobj.id;
						}
					}

					//Prevent rotation (not need)
					//if (collided) {
					//	//gobj.rotateAnim.curTime = oldTime;
					//	gobj.rotateAnim.timeEnd = 0;
					//	gobj.location.rotation = oldRotate;
					//}

				}
			}

		}
	};

}

//Bounds handleCollision(Bounds& startZv, Bounds& nextZv, Bounds& collZv)
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