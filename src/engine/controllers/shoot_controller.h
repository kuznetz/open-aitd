#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;

namespace openAITD {

    // Structure describing a single shoot action triggered by an animation keyframe
    struct ShootAction {
        GameObject* gobj = nullptr;   // shooter object
        int animId;                   // animation ID that triggers the shot
        int keyFrameIdx;              // keyframe index at which the shot occurs
        int boneIdx;                  // armature bone index from which the shot occurs
        bool triggered;               // ensures single execution per keyframe
        int damage;                   // damage dealt on hit
        float range;                  // maximum ray distance
    };

    class ShootController {
    public:
        static const int ShootActionsCount = 10;
        World* world;
        Resources* resources;
        ShootAction actions[ShootActionsCount];

        ShootController(World* world)
            : world(world)
            , resources(world->resources) {
        }

        // Find an existing action by object, animation and keyframe
        ShootAction* getAction(GameObject* gobj, int animId, int keyFrameIdx) {
            for (int i = 0; i < ShootActionsCount; i++) {
                if (actions[i].gobj != gobj) continue;
                if (actions[i].animId != animId) continue;
                if (actions[i].keyFrameIdx != keyFrameIdx) continue;
                return &actions[i];
            }
            return nullptr;
        }

        // Add a new shoot action or return an existing one
        ShootAction* addAction(GameObject* gobj, int animId, int keyFrameIdx, int boneIdx, int damage, float range = 10000.) {
            ShootAction* act = getAction(gobj, animId, keyFrameIdx);
            if (!act) {
                for (int i = 0; i < ShootActionsCount; i++) {
                    if (actions[i].gobj != nullptr) continue;
                    act = &actions[i];
                    act->gobj = gobj;
                    act->animId = animId;
                    act->keyFrameIdx = keyFrameIdx;
                    act->boneIdx = boneIdx;
                    act->damage = damage;
                    act->range = range;
                    act->triggered = false;
                    break;
                }
                if (!act) throw new exception("ShootAction full");
            }
            return act;
        }

        // Main update loop: checks animations and fires shots when keyframe is reached
        void process() {
            for (int i = 0; i < ShootActionsCount; i++) {
                ShootAction* act = &actions[i];
                if (act->gobj == nullptr) continue;
                // Remove action if object is on a different stage
                if (act->gobj->location.stageId != world->curStageId) {
                    act->gobj = nullptr;
                    continue;
                }
                // Remove action if the animation changed
                if (act->gobj->animation.id != act->animId) {
                    act->gobj = nullptr;
                    continue;
                }

                // One‑shot trigger logic
                if (!act->triggered) {
                    if (act->gobj->animation.keyFrameIdx >= act->keyFrameIdx) {
                        // Execute the shot
                        shootInternal(act);
                        act->triggered = true;
                    }
                } else {
                    // Reset trigger when keyframe goes backwards (allows re‑fire)
                    if (act->gobj->animation.keyFrameIdx < act->keyFrameIdx) {
                        act->triggered = false;
                    }
                }
            }
        }

    private:

        Vector3 getBonePosition(GameObject* gobj, int boneIdx) {
            auto mdl = resources->models.getModel(gobj->modelId);
            if (!mdl) throw std::runtime_error("Invalid model");
            auto& curAnim = mdl->model.animations[gobj->animation.animIdx];
            auto& curPose = curAnim.bakedPoses[gobj->animation.animFrame];
            mdl->model.ApplyPose(curPose.data());
            Vector3 bonePos = mdl->model.curPose[boneIdx].translation;
            Matrix rotM = MatrixRotateZYX(gobj->location.rotation2);
            bonePos = Vector3Transform(bonePos, rotM);
            return Vector3Add(bonePos, gobj->location.position);
        }

        // Internal raycast shot implementation
        void shootInternal(const ShootAction* act) {
            GameObject* shooter = act->gobj;
            Room room = resources->stages[world->curStageId].rooms[shooter->location.roomId];
            Vector3 origin = getBonePosition(shooter, act->boneIdx);

            Matrix rotMatrix = MatrixRotateZYX(shooter->location.rotation2);
            Vector3 dir = Vector3Transform({ 0, 0, -1 }, rotMatrix);
            float range = act->range;
            int damage = act->damage;

            float minDist = range;
            GameObject* hitTarget = nullptr;
            bool hitStatic = false;

            // Check dynamic objects
            for (auto& gobj : world->gobjects) {
                if (gobj.id == shooter->id) continue;
                if (gobj.location.stageId != world->curStageId) continue;
                if (gobj.modelId == -1 || !gobj.physics.collidable) continue;

                Bounds bounds = world->getObjectBounds(gobj);
                bounds = world->BoundsChangeRoom(bounds, gobj.location.roomId, shooter->location.roomId);

                float t;
                if (bounds.RayIntersect(origin, dir, t)) {
                    if (t > 0.0f && t < minDist) {
                        minDist = t;
                        hitTarget = &gobj;
                        hitStatic = false;
                    }
                }
            }

            // Check static colliders in the shooter's current room
            for (const auto& collider : room.colliders) {
                const Bounds& colB = collider.bounds;
                float t;
                if (colB.RayIntersect(origin, dir, t)) {
                    if (t > 0.0f && t < minDist) {
                        minDist = t;
                        hitTarget = nullptr;
                        hitStatic = true;
                    }
                }
            }

            world->debugShootFrom = origin;
            Vector3 hitPoint;
            if (hitTarget || hitStatic) {
                hitPoint = Vector3Add(origin, Vector3Scale(dir, minDist));
                world->debugShootTo = hitPoint;
                if (hitTarget) {
                    hitTarget->damage.hitBy = shooter;
                    hitTarget->damage.damage = damage;
                    shooter->hit.hitTo = hitTarget;
                }

            } else {
                hitPoint = Vector3Add(origin, Vector3Scale(dir, range));
                world->debugShootTo = hitPoint;
            }
        }
    };

}