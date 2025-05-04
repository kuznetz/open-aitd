#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;

namespace openAITD {

    struct ThrowAction {
        GameObject* gobj = 0;
        GameObject* throwedItem = 0;
        int animId;
        int keyFrameIdx;
        int activeBoneId;
        int hitDamage;
        bool started;
        bool throwing;
        bool throwRotated;
    };

    class ThrowController {
    public:
        World* world;
        Resources* resources;
        ThrowAction action;

        void throw_(GameObject* gobj, GameObject* throwedItem, int animId, int keyFrameIdx, int activeBone, bool throwRotated, int hitDamage) {
            printf("THROW\n");
            action.gobj = gobj;
            action.throwedItem = throwedItem;
            action.animId = animId;
            action.keyFrameIdx = keyFrameIdx;
            action.activeBoneId = activeBone;
            action.throwRotated = throwRotated;
            action.hitDamage = hitDamage;
            action.started = false;
        }

        ThrowController(World* world) {
            this->world = world;
            this->resources = world->resources;
        }

        void process(float timeDelta) {

        }
    };

}