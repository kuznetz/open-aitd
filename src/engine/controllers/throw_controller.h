#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;
using namespace raylib;

namespace openAITD {

    struct ThrowAction {
        GameObject* gobj = 0;
        GameObject* throwedItem = 0;
        int animId;
        int keyFrameIdx;
        int activeBoneId;
        int hitDamage;
        bool throwRotated;
    };

    class ThrowController {
    public:
        World* world;
        Resources* resources;
        ThrowAction action;

        void throw_(GameObject* gobj, GameObject* throwedItem, int animId, int keyFrameIdx, int activeBone, bool throwRotated, int hitDamage) {
            action.gobj = gobj;
            action.throwedItem = throwedItem;
            action.animId = animId;
            action.keyFrameIdx = keyFrameIdx;
            action.activeBoneId = activeBone;
            action.throwRotated = throwRotated;
            action.hitDamage = hitDamage;
            world->delFromInventory(throwedItem->id);
        }

        ThrowController(World* world) {
            this->world = world;
            this->resources = world->resources;
        }

        void process(float timeDelta) {
            processThrowables(timeDelta);
            processAnim(timeDelta);
        }        

        Vector3 getBonePos() {
            auto& gobj = *action.gobj;
            auto mdl = resources->models.getModel(gobj.modelId, world->altModels);
            if (!mdl) throw new exception("Invalid model");
            auto& curAnim = mdl->model.animations[gobj.animation.animIdx];
            auto& curPose = curAnim.bakedPoses[gobj.animation.animFrame];

            mdl->model.ApplyPose(curPose.data());
            Vector3 v = mdl->model.curPose[action.activeBoneId].translation;
            v = Vector3Transform(v, MatrixRotateY(PI));

            Matrix rotMatrix = gobj.getRotMatrix();
            v = Vector3Transform(v, rotMatrix);
            v = Vector3Add(v, gobj.getPosition());
            return v;
        }

        void processAnim(float timeDelta) {
            if (action.gobj == 0) return;
            auto& gobj = *action.gobj;
            if (gobj.animation.id != action.animId) {
                action.gobj = 0;
                return;
            }
            if (gobj.animation.keyFrameIdx < action.keyFrameIdx) {
                return;
            }
            //printf("THROW\n");
            auto& item = *action.throwedItem;
            
            item.setStage(gobj.getStageId(),gobj.getRoomId(),getBonePos());
            item.setOrigRotation(gobj.getOrigRotation());
            item.boundsType = BoundsType::rotated;
            
            item.bitField.foundable = false;
            item.throwing.throwedBy = &gobj;
            item.throwing.hitDamage = action.hitDamage;
            item.throwing.active = true;
            action.gobj = 0;
        }

		void throwStop(GameObject& gobj) {
            action.throwedItem = 0;
			gobj.throwing.active = false;
			gobj.bitField.foundable = true;
            gobj.animation.prevMoveRoot = { 0,0,0 };
            gobj.animation.moveRoot = { 0,0,0 };
		}

        void processThrowables(float timeDelta) {
            for (auto& gobj : world->gobjects) {
                if (!gobj.throwing.active) continue;
                if (gobj.getStageId() != world->curStageId) continue;

                auto& room = world->curStage->rooms[gobj.getRoomId()];

                Matrix rotMatrix = gobj.getRotMatrix();
                Vector3 velocity = Vector3Transform({0,0,4.f}, rotMatrix);
                velocity = Vector3Scale(velocity, timeDelta);
                
                Bounds newBounds = gobj.getBounds();
                newBounds.min = Vector3Add(newBounds.min, velocity);
                newBounds.max = Vector3Add(newBounds.max, velocity);

                bool hitStatic = false;
                for (auto& collider : room.colliders) {
                    if (collider.bounds.CollToBox(newBounds)) {
                        hitStatic = true;
                        break;
                    }
                }

                bool hitDynamic = false;
                GameObject* hitObject = nullptr;
                for (auto& other : world->gobjects) {
                    if (&other == &gobj) continue;
                    if (other.getStageId() != gobj.getStageId()) continue;
                    if (other.getRoomId() != gobj.getRoomId()) continue;
                    if (&other == gobj.throwing.throwedBy) continue;
                    if (!other.physics.collidable) continue;
                    if (other.modelId == -1) continue;

                    Bounds otherBounds = other.getBounds();
                    if (newBounds.CollToBox(otherBounds)) {
                        hitDynamic = true;
                        hitObject = &other;
                        break;
                    }
                }

                if (hitStatic || hitDynamic) {
                    throwStop(gobj);
                    placeOnSurface(gobj);
                    if (hitDynamic && hitObject != nullptr) {
                        hitObject->damage.hitBy = &gobj;
                        hitObject->damage.damage = gobj.throwing.hitDamage;
                        gobj.hit.hitTo = hitObject;
                    }
                } else {
                    gobj.setPosition(Vector3Add(gobj.getPosition(), velocity));
                }
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

    };

}