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

        Vector3 getBonePos() {
            auto& gobj = *action.gobj;
            auto mdl = resources->models.getModel(gobj.modelId);
            if (!mdl) throw new exception("Invalid model");
            auto& curAnim = mdl->model.animations[gobj.animation.animIdx];
            auto& curPose = curAnim.bakedPoses[gobj.animation.animFrame];

            mdl->model.ApplyPose(curPose.data());
            Vector3 v = mdl->model.curPose[action.activeBoneId].translation;

            Quaternion rotationQuat = QuaternionFromEuler(
                gobj.location.rotation2.x,
                gobj.location.rotation2.y,
                gobj.location.rotation2.z
            );
            v = Vector3RotateByQuaternion(v, rotationQuat);            
            v = Vector3Add(v, gobj.location.position);
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
            item.location.stageId = gobj.location.stageId;
            item.location.roomId = gobj.location.roomId;
            item.location.rotation2 = gobj.location.rotation2;
            item.location.position = getBonePos();

            item.physics.boundsCached = false;
            item.boundsType = BoundsType::rotated;
            
            item.bitField.foundable = false;
            item.throwing.throwedBy = &gobj;
            item.throwing.hitDamage = action.hitDamage;
            item.throwing.active = true;
            //item.throwing.direction = Vector3RotateByQuaternion({ -1, 0, 0}, item.location.rotation);
            action.gobj = 0;
        }

		void throwStop(GameObject& gobj) {
            action.throwedItem = 0;
			gobj.throwing.active = false;
			gobj.bitField.foundable = true;
            gobj.animation.prevMoveRoot = { 0,0,0 };
            gobj.animation.moveRoot = { 0,0,0 };
		}

        void process(float timeDelta) {
            processAnim(timeDelta);
        }
    };

}