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

        void process(float timeDelta) {
            for (int i = 0; i < AnimActionsCount; i++) {
                AnimAction* act = &actions[i];
                if (act->gobj == 0) continue;
                if (act->gobj->animation.id != act->animId) {
                    act->gobj = 0;
                    continue;
                }
                if (!act->triggered) {
                    if (act->gobj->animation.keyFrameIdx >= act->keyFrameIdx) {
                        printf("TRIGGER");
                        act->triggered = true;
                    }
                } else {
                    if (act->gobj->animation.keyFrameIdx < act->keyFrameIdx) {
                        act->triggered = false;
                    }
                }
            }
        }
    };

}