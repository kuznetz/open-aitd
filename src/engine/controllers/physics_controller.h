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
			auto& p = gobj.getPosition();
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

		void processStaticColliders(GameObject& gobj, Room& room) {
			Bounds& objB = gobj.getBounds();
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
						}
					}
					else if (room.colliders[i].type == 3) {
						gobj.physics.staticColl = 255;
					}
				}
			}
			if (gobj.physics.collidable) {
				gobj.physics.moveVec = v;
			}
		}

		void processDynamicColliders(GameObject& gobj, Room& room, int ignoredId = -1) {
			Bounds& objB = gobj.getBounds();
			Vector3 v = gobj.physics.moveVec;
			bool collided = false;

			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj2 = world->gobjects[i];
				if (&gobj == &gobj2) continue;
				if (gobj2.id == ignoredId) continue;
				if (!gobj2.physics.collidable) continue;
				if (gobj2.modelId == -1) continue;
				if (gobj2.getStageId() != gobj.getStageId()) continue;

				Bounds objB2 = gobj2.getBounds();
				if (gobj2.getRoomId() != gobj.getRoomId()) {
					if (resources->isRoomsConnected(*world->curStage, gobj.getRoomId(), gobj2.getRoomId())) {
						objB2 = world->curStage->BoundsChangeRoom(objB2, gobj2.getRoomId(), gobj.getRoomId());
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
							auto v2 = v; //v2 for not modify v
							c2 = objB.CollToBoxV_XZ(v2, objB2S);
							if (c2) {
								v = pushObject(gobj2, room, v, gobj.id);
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
					if (gobj.physics.hitObjectDamage) {
						hitObjDamage(gobj2, gobj);
					}
				}
			}
			if (gobj.physics.collidable) {
				gobj.physics.moveVec = v;
			}			
		}

		void processSCollidersNoColl(GameObject& gobj, Room& room) {
			Bounds& objB = gobj.getBounds();
			gobj.physics.staticColl = -1;
			for (int i = 0; i < room.colliders.size(); i++) {
				Bounds& colB = room.colliders[i].bounds;
				if (objB.CollToBox(colB)) {
  				gobj.physics.staticColl = 1;
					break;
				}
			}
		}

		void processDCollidersNoColl(GameObject& gobj, Room& room) {
			Bounds& objB = gobj.getBounds();
			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj2 = world->gobjects[i];
				if (&gobj == &gobj2) continue;
				if (!gobj2.physics.collidable) continue;
				if (gobj2.modelId == -1) continue;
				if (gobj2.getStageId() != gobj.getStageId()) continue;

				Bounds objB2 = gobj2.getBounds();
				if (gobj2.getRoomId() != gobj.getRoomId()) {
					if (resources->isRoomsConnected(*world->curStage, gobj.getRoomId(), gobj2.getRoomId())) {
						objB2 = world->curStage->BoundsChangeRoom(objB2, gobj2.getRoomId(), gobj.getRoomId());
					}
					else {
						continue;
					}
				}

				if (objB.CollToBox(objB2)) {
					gobj.physics.objectColl = gobj2.id;
					if (gobj2.physics.collidedBy == -1) {
						gobj2.physics.collidedBy = gobj.id;
					}
					if (gobj.physics.hitObjectDamage) {
						hitObjDamage(gobj2, gobj);
					}
				}
			}	
		}

		Vector3 pushObject(GameObject& gobj, Room& room, Vector3 v, int ignoredId = -1) {
			gobj.physics.moveVec = v;
			gobj.physics.moving = true;
			processStaticColliders(gobj, room);
			processDynamicColliders(gobj, room, ignoredId);
			Vector3 result = gobj.physics.moveVec;
			gobj.setPosition(Vector3Add(gobj.getPosition(), result));
			gobj.physics.moving = false;
			return result;
		}

		void processGravity(GameObject& gobj, Room& room, float timeDelta) {
			if (!gobj.bitField.fallable) return;
			if (gobj.track.id != -1 ) return;
			gobj.physics.falling = true;

			auto& objB = gobj.getBounds();
			float moveY = (-2 * timeDelta);
			auto objBM = objB;
			objBM.min.y += moveY;
			objBM.max.y += moveY;

			if (objBM.min.y <= 0.001) {
				Vector3 pos = gobj.getPosition();
				pos.y = 0;
				gobj.setPosition(pos);
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
				if (gobj2.getStageId() != gobj.getStageId()) continue;
				if (gobj2.getRoomId() != gobj.getRoomId()) {
					continue;
				}				
				Bounds objB2 = gobj2.getBounds();
				Bounds objB2S = objB2.getExpanded(-0.002f);
				if (!objBM.CollToBox(objB2S)) continue;
				gobj.physics.falling = false;
				moveY = (objB2S.max.y - objB.min.y) + 0.002f;
			}

			if (moveY < 0.0001f) {
				Vector3 pos = gobj.getPosition();
				pos.y += moveY;
				gobj.setPosition(pos);
			}
		}

		// Helper: finds the highest surface (max Y) of any static collider that
		// horizontally overlaps the object in XZ plane.
		// Returns -1.0f if no supporting collider is found.
		static float findGroundHeight(const GameObject& gobj, const World* world) {
				if (gobj.getStageId() != world->curStageId) {
						throw std::runtime_error("Object is not in the current stage");
				}
				auto& room = world->curStage->rooms[gobj.getRoomId()];
				const auto& objPos = gobj.getPosition();

				float groundY = 0.0f;
				bool hasColliderUnder = false;

				for (const auto& collider : room.colliders) {
						const Bounds& colB = collider.bounds;
						bool overlapX = (objPos.x < colB.max.x && objPos.x > colB.min.x);
						bool overlapZ = (objPos.z < colB.max.z && objPos.z > colB.min.z);
						if (overlapX && overlapZ) {
								if (!hasColliderUnder || objPos.y + 0.001f > groundY) {
										groundY = colB.max.y;
										hasColliderUnder = true;
								}
						}
				}

				return hasColliderUnder ? groundY : -1.0f;   // -1 means no support
		}

    // Lift a stuck object.
		void raiseStuckObject(GameObject& gobj) {
				float groundY = findGroundHeight(gobj, world);
				if (groundY < 0.0f) {
						// No surface below – do nothing (or leave as is)
						return;
				}
				Vector3 pos = gobj.getPosition();
				if (pos.y < groundY) {
						pos.y = groundY + 0.001f;
						gobj.setPosition(pos);
				}
		}

		// If the object is above the found surface, drop it down onto that surface.
		// If no surface exists, reset Y to 0
		void placeOnSurface(GameObject& gobj) {
				float groundY = findGroundHeight(gobj, world);
				Vector3 pos = gobj.getPosition();
				if (groundY < 0.0f) {
						pos.y = 0.0f;
				} else if (pos.y > groundY) {
						pos.y = groundY + 0.001f;
				}
				gobj.setPosition(pos);
		}

		void processZones(GameObject& gobj, Room* curRoom) {
			//Check Zones
			gobj.physics.zoneTriggered = -1;
			if (!gobj.bitField.trigger) return;

			for (int i = 0; i < curRoom->zones.size(); i++) {
				auto& curZone = curRoom->zones[i];
				if (!objectInZone(gobj, &curZone)) continue;
				if (curZone.type == RoomZoneType::ChangeRoom) {
					//printf("Change room obj %d: %d -> %d\n", gobj.id, gobj.getRoomId(), curZone.parameter);
					gobj.changeRoom(curZone.parameter);
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

		void process(float timeDelta) {
			auto& curStage = resources->stages[world->curStageId];

			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj = world->gobjects[i];
				if (gobj.getStageId() != world->curStageId) continue;
				gobj.physics.collidedBy = -1;
				gobj.physics.staticColl = -1;
				gobj.physics.objectColl = -1;
			}

			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj = world->gobjects[i];
				if (gobj.getStageId() != world->curStageId) continue;
				if (gobj.modelId == -1) continue;
				if (!world->isObjectActive(gobj)) continue;

				if (gobj.changingStage) {
					if (gobj.bitField.fallable) {
						raiseStuckObject(gobj);
						placeOnSurface(gobj);
					}
					gobj.changingStage = false;
				}

				auto* curRoom = &curStage.rooms[gobj.getRoomId()];
  			auto& moveVec = gobj.physics.moveVec;
				moveVec = { 0,0,0 };
				Vector3 moveVec0 = { 0,0,0 };
				if (gobj.bitField.animated) {
					moveVec0 = Vector3Add(
						moveVec0,
						Vector3Subtract(gobj.animation.moveRoot, gobj.animation.prevMoveRoot)
					);
				}
				gobj.physics.moving = ( fabs(moveVec0.x) > 0.0001 || fabs(moveVec0.z) > 0.0001 );
				if (gobj.physics.moving) {
        	moveVec = Vector3Transform(moveVec0, gobj.getRotMatrix());
					if (gobj.physics.collidable) {
						processStaticColliders(gobj, *curRoom);
						processDynamicColliders(gobj, *curRoom);
					} else {
						processSCollidersNoColl(gobj, *curRoom);
						processDCollidersNoColl(gobj, *curRoom);
					}
					gobj.setPosition(Vector3Add(gobj.getPosition(), moveVec));
				}

				processGravity(gobj, *curRoom, timeDelta);
				processZones(gobj, curRoom);
			}
		}
	};

}
