#pragma once
#include "../resources/resources.h"
#include "../world/world.h"
#include "../screens/found_screen.h"

using namespace std;
namespace openAITD {

	class PhysicsController {
	public:
		World* world;
		Resources* resources;
		FoundScreen* foundScreen;

		PhysicsController(Resources* res, World* world, FoundScreen* found) {
			this->resources = res;
			this->world = world;
			foundScreen = found;
		}

		bool objectInZone(GameObject& gobj, RoomZone* zone) {
			auto& p = gobj.location.position;
			auto& b = zone->bounds;
			return ((p.x > b.min.x) && (p.x < b.max.x) && (p.z > b.min.z) && (p.z < b.max.z));
		}

		bool CollPointToBox(const Vector3& p, Vector3& v, Bounds& b) {
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

		void hitObjDamage(GameObject& gobj, GameObject& damager) {
			damager.hit.hitTo = &gobj;
			gobj.damage.hitBy = &damager;
			gobj.damage.damage = damager.physics.hitObjectDamage;
		}

		void throwDamage(GameObject& gobj, GameObject& throwed) {
			gobj.damage.hitBy = &throwed;
			gobj.damage.damage = throwed.throwing.hitDamage;
		}

		void throwStop(GameObject& gobj) {
			gobj.throwing.active = false;
			gobj.bitField.foundable = true;
			gobj.location.position.y = 0;
		}

		void processStaticColliders(GameObject& gobj, Room& room) {
			Bounds& objB = world->getObjectBounds(gobj);
			Vector3 v = gobj.physics.moveVec;
			bool collided = false;
			for (int i = 0; i < room.colliders.size(); i++) {
				Bounds& colB = room.colliders[i].bounds;

				bool c = objB.CollToBox(colB);
				collided = collided || c;

				if (c) {
					Bounds colBS = colB.getExpanded(-0.01f);
					if (gobj.physics.moving) {
						objB.CollToBoxV(v, colBS);
					}
					if (room.colliders[i].type == 9) {
						gobj.physics.staticColl = room.colliders[i].parameter;
					}
					else if (room.colliders[i].type == 3) {
						gobj.physics.staticColl = 255;
					}
				}
			}
			if (collided) {
				if (gobj.throwing.active) throwStop(gobj);
			}
			if (gobj.physics.collidable) {
				gobj.physics.moveVec = v;
			}
		}

		void processDynamicColliders(GameObject& gobj, Room& room) {
			Bounds& objB = world->getObjectBounds(gobj);
			Vector3 v = gobj.physics.moveVec;
			bool collided = false;

			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj2 = world->gobjects[i];
				if (&gobj == &gobj2) continue;
				if (gobj2.modelId == -1) continue;
				if (gobj2.location.stageId != gobj.location.stageId) continue;
				if (gobj.throwing.active && gobj.throwing.throwedBy == &gobj2) continue;

				Bounds objB2 = world->getObjectBounds(gobj2);
				if (gobj2.location.roomId != gobj.location.roomId) {
					if (resources->isRoomsConnected(*world->curStage, gobj.location.roomId, gobj2.location.roomId)) {
						objB2 = world->BoundsChangeRoom(objB2, gobj2.location.roomId, gobj.location.roomId);
					}
					else {
						continue;
					}
				}

				//Expand to constant collision check
				bool c = objB.CollToBox(objB2);
				bool c2 = false;
				collided = collided || c;
				//if (gobj2.id == 0) printf("chest coll = %d\n", c);
				if (c) {
					Bounds objB2S = objB2.getExpanded(-0.01f);
					if (gobj.physics.moving && !gobj2.bitField.foundable) {
						if (gobj2.bitField.movable) {
							auto v2 = v;
							c2 = objB.CollToBoxV(v2, objB2S);
							if (c2) {
								pushObject(gobj2, room, v);
							}
						}
						else {
							c2 = objB.CollToBoxV(v, objB2S);
						}
					}
					gobj.physics.objectColl = gobj2.id;
					if (gobj2.physics.collidedBy == -1) {
						gobj2.physics.collidedBy = gobj.id;
					}
					//takable
					if (gobj.track.mode == GOTrackMode::manual && gobj2.bitField.foundable && gobj2.invItem.foundTimeout < this->world->chrono) {
						foundScreen->main(gobj2.id);
					}
				}
				if (c2) {
					if (gobj.throwing.active) {
						throwStop(gobj);
						throwDamage(gobj2, gobj);
					}
					if (gobj.physics.hitObjectDamage) {
						hitObjDamage(gobj2, gobj);
					}
				}
			}
			if (gobj.physics.collidable) {
				gobj.physics.moveVec = v;
			}			
		}

		void pushObject(GameObject& gobj, Room& room, Vector3& v) {
			gobj.physics.moveVec = v;
			gobj.physics.moving = true;
			gobj.physics.boundsCached = false;
			processStaticColliders(gobj, room);
			processDynamicColliders(gobj, room);
			v = gobj.physics.moveVec;
			gobj.location.position = Vector3Add(gobj.location.position, v);
			gobj.physics.moving = false;
		}

		void processZones(GameObject& gobj, Room* curRoom) {
			//Check Zones
			gobj.physics.zoneTriggered = -1;
			if (gobj.bitField.trigger) {
				for (int i = 0; i < curRoom->zones.size(); i++) {
					auto& curZone = curRoom->zones[i];
					if (!objectInZone(gobj, &curZone)) continue;
					if (curZone.type == RoomZoneType::ChangeRoom) {
						//printf("Change room obj %d: %d -> %d\n", gobj.id, gobj.location.roomId, curZone.parameter);
						gobj.location.position = world->VectorChangeRoom(gobj.location.position, gobj.location.roomId, curZone.parameter);
						gobj.location.roomId = curZone.parameter;
						gobj.physics.boundsCached = false;
						break;
					}
					if (curZone.type == RoomZoneType::Trigger) {
						//printf("Triggered obj %d zone %d\n", gobj.id, curZone.parameter);
						gobj.physics.zoneTriggered = curZone.parameter;
						// AITD1 stops at the first zone
						break;
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
			world->setCurStage(loc.stageId, loc.roomId);
			//Select Camera
			if (world->curStageId != loc.stageId) return;
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
			}

			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj = world->gobjects[i];
				if (gobj.location.stageId != world->curStageId) continue;
				if (gobj.modelId == -1) continue;

				gobj.physics.moving = false;
				auto& v = gobj.physics.moveVec;
				v = Vector3Subtract(gobj.animation.moveRoot, gobj.animation.prevMoveRoot);
				gobj.physics.moving = (v.x != 0 || v.y != 0 || v.z != 0);

				if (gobj.physics.moving) {
					v = Vector3RotateByQuaternion(v, gobj.location.rotation);
					gobj.animation.prevMoveRoot = gobj.animation.moveRoot;
					gobj.physics.boundsCached = false;
				}

				auto* curRoom = &curStage.rooms[gobj.location.roomId];
				processStaticColliders(gobj, *curRoom);
				processDynamicColliders(gobj, *curRoom);

				if (gobj.physics.moving) {
					gobj.location.position = Vector3Add(gobj.location.position, gobj.physics.moveVec);
				}

				processZones(gobj, curRoom);
			}

			followCameraProcess();
		}
	};

}
