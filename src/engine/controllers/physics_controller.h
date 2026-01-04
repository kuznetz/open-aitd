#pragma once
#include "../resources/resources.h"
#include "../world/world.h"
#include "../screens/found_screen.h"
#include "./throw_controller.h"

using namespace std;
namespace openAITD {

	class PhysicsController {
	public:
		World* world;
		Resources* resources;
		FoundScreen* foundScreen;
		ThrowController* throwContr;

		PhysicsController(Resources* res, World* world, FoundScreen* found, ThrowController* throwContr) {
			this->resources = res;
			this->world = world;
			this->throwContr = throwContr;
			foundScreen = found;
		}

		bool objectInZone(GameObject& gobj, RoomZone* zone) {
			auto& p = gobj.location.position;
			auto& b = zone->bounds;
			return (
				(p.x >= b.min.x) && (p.x <= b.max.x) &&
				((p.y+0.001) >= b.min.y) && (p.y <= b.max.y) &&
				(p.z >= b.min.z) && (p.z <= b.max.z)
			);
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

		void processStaticColliders(GameObject& gobj, Room& room) {
			Bounds& objB = world->getObjectBounds(gobj);
			Vector3 v = gobj.physics.moveVec;
			bool collided = false;
			gobj.physics.staticColl = -1;
			for (int i = 0; i < room.colliders.size(); i++) {
				Bounds& colB = room.colliders[i].bounds;
				if (objB.CollToBox(colB)) {
					collided = true;
					Bounds colBS = colB.getExpanded(-0.001f);
					if (gobj.physics.moving && objB.CollToBox(colBS)) {
						objB.CollToBoxV_XZ(v, colBS);
					}
					if (room.colliders[i].type == 9) {
						gobj.physics.staticColl = room.colliders[i].parameter;
						int gobjStatId = room.colliders[i].linkedObjectId;
						if (gobjStatId >= 0 ) {
							GameObject& gobjStat = world->gobjects[gobjStatId];
							if (gobj.throwing.active) {
								this->throwDamage(gobjStat, gobj);
								gobjStat.physics.collidedBy = gobj.id;
							}
						}
					}
					else if (room.colliders[i].type == 3) {
						gobj.physics.staticColl = 255;
					}
				}
			}
			if (collided) {
				if (gobj.throwing.active) {
					throwContr->throwStop(gobj);
				}
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
				if (!gobj2.physics.collidable) continue;
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

				bool c2 = false;
				if (objB.CollToBox(objB2)) {
					collided = true;
  				//Shrink to constant collision check
					Bounds objB2S = objB2.getExpanded(-0.001f);
					if (gobj.physics.moving && !gobj2.bitField.foundable) {
						if (gobj2.bitField.movable) {
							auto v2 = v;
							c2 = objB.CollToBoxV_XZ(v2, objB2S);
							if (c2) {
								pushObject(gobj2, room, v);
							}
						}
						else {
							c2 = objB.CollToBoxV_XZ(v, objB2S);
						}
					}
					gobj.physics.objectColl = gobj2.id;
					if (gobj2.physics.collidedBy == -1) {
						gobj2.physics.collidedBy = gobj.id;
					}
					//takable
					if (
						 gobj.track.mode == GOTrackMode::manual &&
						 gobj2.bitField.foundable && 
						 gobj2.invItem.foundTimeout < this->world->chrono
					) {
						foundScreen->main(gobj2.id);
					}
				}
				if (c2) {
					if (gobj.throwing.active) {
						throwContr->throwStop(gobj);
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

		void processGravity(GameObject& gobj, Room& room, float timeDelta) {
			if (!gobj.bitField.fallable) return;
			if (gobj.track.id != -1 ) return;
			gobj.physics.falling = true;

			auto& objB = world->getObjectBounds(gobj);
			float moveY = (-2 * timeDelta);
			auto objBM = objB;
			objBM.min.y += moveY;
			objBM.max.y += moveY;

			if (objBM.min.y <= 0.001) {
				gobj.location.position.y = 0;
				gobj.physics.falling = false;
				return;
			}

			for (int i = 0; i < room.colliders.size(); i++) {
				Bounds& colB = room.colliders[i].bounds;
				Bounds colBS = colB.getExpanded(-0.002f);
				if (!objBM.CollToBox(colBS)) continue;
				gobj.physics.falling = false;
				moveY = (colBS.max.y - objB.min.y) + 0.002f;
			}

			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj2 = world->gobjects[i];
				if (&gobj == &gobj2) continue;
				if (!gobj2.physics.collidable) continue;
				if (gobj2.modelId == -1) continue;
				if (gobj2.location.stageId != gobj.location.stageId) continue;
				if (gobj.throwing.active && gobj.throwing.throwedBy == &gobj2) continue;
				if (gobj2.location.roomId != gobj.location.roomId) {
					continue;
				}				
				Bounds objB2 = world->getObjectBounds(gobj2);
				Bounds objB2S = objB2.getExpanded(-0.002f);
				if (!objBM.CollToBox(objB2S)) continue;
				gobj.physics.falling = false;
				moveY = (objB2S.max.y - objB.min.y) + 0.002f;
			}

			if (moveY < 0.0001f) {
				gobj.physics.boundsCached = false;
				gobj.location.position.y += moveY;
			}
		}

		void processZones(GameObject& gobj, Room* curRoom) {
			//Check Zones
			gobj.physics.zoneTriggered = -1;
			if (!gobj.bitField.trigger) return;

			for (int i = 0; i < curRoom->zones.size(); i++) {
				auto& curZone = curRoom->zones[i];
				if (!objectInZone(gobj, &curZone)) continue;
				if (curZone.type == RoomZoneType::ChangeRoom) {
					//printf("Change room obj %d: %d -> %d\n", gobj.id, gobj.location.roomId, curZone.parameter);
					gobj.location.position = world->VectorChangeRoom(gobj.location.position, gobj.location.roomId, curZone.parameter);
					gobj.location.roomId = curZone.parameter;
					gobj.physics.boundsCached = false;
					break;
				} else if (curZone.type == RoomZoneType::Trigger) {
					//printf("Triggered obj %d zone %d\n", gobj.id, curZone.parameter);
					gobj.physics.zoneTriggered = curZone.parameter;
					// AITD1 stops at the first zone
					break;
				} else if (curZone.type == RoomZoneType::ChangeStage) {
					if (gobj.stageLifeId != -1) {
						gobj.lifeId = gobj.stageLifeId;
						gobj.physics.zoneTriggered = curZone.parameter;
						//flagFloorChange = true;
					}
					break;
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

				auto* curRoom = &curStage.rooms[gobj.location.roomId];
  			auto& moveVec = gobj.physics.moveVec;
				moveVec = { 0,0,0 };
				Vector3 moveVec0 = { 0,0,0 };
				if (gobj.throwing.active) {
					moveVec0.z += -3 * timeDelta;
				}
				if (gobj.bitField.animated) {
					moveVec0 = Vector3Add(
						moveVec0,
						Vector3Subtract(gobj.animation.moveRoot, gobj.animation.prevMoveRoot)
					);
				}
				gobj.physics.moving = ( fabs(moveVec0.x) > 0.0001 || fabs(moveVec0.z) > 0.0001 );
				if (gobj.physics.moving) {
        	moveVec = Vector3RotateByQuaternion(moveVec0, gobj.location.rotation);
					if (gobj.physics.collidable) {
						processStaticColliders(gobj, *curRoom);
						processDynamicColliders(gobj, *curRoom);
					}
					gobj.physics.boundsCached = false;
					gobj.location.position = Vector3Add(gobj.location.position, moveVec);
				}

				processGravity(gobj, *curRoom, timeDelta);
				processZones(gobj, curRoom);
			}

			followCameraProcess();
		}
	};

}
