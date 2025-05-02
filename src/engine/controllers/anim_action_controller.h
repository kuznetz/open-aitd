#pragma once
#include "../resources/missed_anims.h"
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;

namespace openAITD {

    enum class AnimActionType {
        hit = 1,
        fire = 2,
        throw_ = 3,
        sound = 4,
    };

    struct AnimAction {
        GameObject* gobj = 0;
        AnimActionType type;
        int animId;
        int keyFrameIdx;
        bool triggered;
    };

    class AnimActionController {
    public:
        static const int AnimActionsCount = 50;
        MissedAnims missedAnims;
        World* world;
        Resources* resources;
        AnimAction actions[AnimActionsCount];

        AnimAction* getAction(GameObject* gobj, AnimActionType type, int animId, int keyFrameIdx) {
            for (int i = 0; i < AnimActionsCount; i++) {
                if (actions[i].gobj != gobj) continue;
                if (actions[i].type != type) continue;
                if (actions[i].animId != animId) continue;
                if (actions[i].keyFrameIdx != keyFrameIdx) continue;
                return &actions[i];
            }
            return 0;
        }

        AnimAction* addAction(GameObject* gobj, AnimActionType type, int animId, int keyFrameIdx) {
            AnimAction* act = getAction(gobj, type, animId, keyFrameIdx);
            if (!act) {
                for (int i = 0; i < AnimActionsCount; i++) {
                    if (actions[i].gobj != 0) continue;
                    act = &actions[i];
                    act->gobj = gobj;
                    act->type = type;
                    act->animId = animId;
                    act->keyFrameIdx = keyFrameIdx;
                    act->triggered = false;
                    break;
                }
                if (!act) throw new exception("AnimAction full");
            }
            return act;
        }

        AnimActionController(World* world) {
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

        void process(float timeDelta) {
            for (int i = 0; i < AnimActionsCount; i++) {
                AnimAction* act = &actions[i];
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
                if (act->gobj->hit.active) {
                    calcHitBounds(*act->gobj);
                }
            }
        }
    };

}