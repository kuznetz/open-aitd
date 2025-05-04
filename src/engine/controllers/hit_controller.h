#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;

namespace openAITD {

    struct HitAction {
        GameObject* gobj = 0;
        int animId;
        int keyFrameIdx;
        bool triggered;
    };

    class HitController {
    public:
        static const int AnimActionsCount = 50;
        World* world;
        Resources* resources;
        HitAction actions[AnimActionsCount];

        HitAction* getAction(GameObject* gobj, int animId, int keyFrameIdx) {
            for (int i = 0; i < AnimActionsCount; i++) {
                if (actions[i].gobj != gobj) continue;
                if (actions[i].animId != animId) continue;
                if (actions[i].keyFrameIdx != keyFrameIdx) continue;
                return &actions[i];
            }
            return 0;
        }

        HitAction* addAction(GameObject* gobj, int animId, int keyFrameIdx) {
            HitAction* act = getAction(gobj, animId, keyFrameIdx);
            if (!act) {
                for (int i = 0; i < AnimActionsCount; i++) {
                    if (actions[i].gobj != 0) continue;
                    act = &actions[i];
                    act->gobj = gobj;
                    act->animId = animId;
                    act->keyFrameIdx = keyFrameIdx;
                    act->triggered = false;
                    break;
                }
                if (!act) throw new exception("AnimAction full");
            }
            return act;
        }

        HitController(World* world) {
            this->world = world;
            this->resources = world->resources;
        }

        void calcHitBounds(GameObject& gobj) {
            auto mdl = resources->models.getModel(gobj.modelId);
            if (!mdl) throw new exception("Invalid model");
            auto& curAnim = mdl->model.animations[gobj.animation.animIdx];
            auto& curPose = curAnim.bakedPoses[gobj.animation.animFrame];
            
            mdl->model.ApplyPose(curPose.data());
            //Vector3 v = Vector3Negate(mdl->model.curPose[gobj.hit.boneIdx].translation);
            Vector3 v = mdl->model.curPose[gobj.hit.boneIdx].translation;
            v = Vector3RotateByQuaternion(v, gobj.location.rotation);
            v = Vector3Add(v, gobj.location.position);
            //v.y = -v.y;
            //v = Vector3Add(v, roomPos);
            auto& r = gobj.hit.range;

            gobj.hit.bounds = { { v.x - r, v.y - r, v.z - r }, { v.x + r, v.y + r, v.z + r } };
        }

        bool CollBoxToBox(BoundingBox& b1, BoundingBox& b2) {
            if (b1.max.x < b2.min.x || b1.min.x > b2.max.x)  return false;
            if (b1.max.y < b2.min.y || b1.min.y > b2.max.y)  return false;
            if (b1.max.z < b2.min.z || b1.min.z > b2.max.z)  return false;
            return true;
        }

        void process(float timeDelta) {
            for (int j = 0; j < world->gobjects.size(); j++) {
                auto& gobj = world->gobjects[j];
                gobj.hit.hitTo = 0;
                gobj.hit.hitBy = 0;
                gobj.hit.damage = 0;
            }

            for (int i = 0; i < AnimActionsCount; i++) {
                HitAction* act = &actions[i];
                if (act->gobj == 0) continue;

                if (act->gobj->animation.id != act->animId) {
                    act->gobj->hit.active = false;
                    act->gobj = 0;
                    continue;
                }
                if (!act->triggered) {
                    if (act->gobj->animation.keyFrameIdx >= act->keyFrameIdx) {
                        act->gobj->hit.active = true;
                        act->triggered = true;
                    }
                } else {
                    if (act->gobj->animation.keyFrameIdx < act->keyFrameIdx) {
                        act->gobj->hit.active = false;
                        act->triggered = false;
                    }
                }
                if (!act->gobj->hit.active) continue;
                calcHitBounds(*act->gobj);

                for (int j = 0; j < world->gobjects.size(); j++) {
                    //if (j == 6) printf("6");
                    auto& gobj = world->gobjects[j];
                    if (gobj.location.stageId != world->curStageId) continue;
                    if (gobj.modelId == -1) continue;
                    if (gobj.id == act->gobj->id) continue;

                    BoundingBox& objB = world->BoundsChangeRoom(world->getObjectBounds(gobj), gobj.location.roomId, act->gobj->location.roomId);
                    if (!CollBoxToBox(act->gobj->hit.bounds, objB)) continue;
                    printf("HIT %d->%d\n", act->gobj->id, gobj.id);
                    gobj.hit.hitBy = act->gobj;
                    gobj.hit.damage = act->gobj->hit.hitDamage;
                    act->gobj->hit.hitTo = &gobj;
                    act->gobj->hit.active = false;
                }
                
            }
        }
    };

}