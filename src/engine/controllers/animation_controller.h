#pragma once
#include "../resources/missed_anims.h"
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;

namespace openAITD {

	class AnimationController {
	public:
        MissedAnims missedAnims;
		World* world;
		Resources* resources;

		AnimationController(Resources* res, World* world) {
			this->resources = res;
			this->world = world;
		}

        float QuaterionDotProduct(Quaternion q1, Quaternion q2) {
            return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
        }

        void processRotateAnim(float timeDelta) {
            for (int i = 0; i < world->gobjects.size(); i++) {
                auto& gobj = world->gobjects[i];
                if (gobj.location.stageId != world->curStageId) continue;
                if (gobj.rotateAnim.timeEnd == 0) continue;

                auto& rot = gobj.rotateAnim;
                rot.curTime += timeDelta;
                if (rot.curTime == 0) {
                    gobj.location.rotation = rot.from;
                    /*auto dot_product = QuaterionDotProduct(rot.from, rot.to);
                    if (dot_product < 0) {
                        rot.to = raylib::QuaternionInvert(rot.to);
                    }*/

                } else if (rot.curTime >= rot.timeEnd) {
                    gobj.location.rotation = rot.to;
                    gobj.rotateAnim.lifeAngles[0] = gobj.rotateAnim.toLifeAngles[0];
                    gobj.rotateAnim.lifeAngles[1] = gobj.rotateAnim.toLifeAngles[1];
                    gobj.rotateAnim.lifeAngles[2] = gobj.rotateAnim.toLifeAngles[2];
                    rot.timeEnd = 0;
                }
                else {
                    gobj.location.rotation = QuaternionSlerp(rot.from, rot.to, rot.curTime / rot.timeEnd);
                }
            }
        }

		void process(float timeDelta) {
            processRotateAnim(timeDelta);

			for (int i = 0; i < this->world->gobjects.size(); i++) {
				auto& gobj = this->world->gobjects[i];
				if (gobj.location.stageId != this->world->curStageId) continue;
				if (gobj.modelId == -1) continue;
                auto& objAni = gobj.animation;
				if (objAni.id == -1) continue;
                auto mdl = resources->models.getModel(gobj.modelId);

                if (gobj.modelId != gobj.prevModelId) {
                    gobj.animation.oldPose.resize(0);
                    objAni.animTime = 0;
                    objAni.animEnd = 0;
                }

                auto p = mdl->animsIds.find(objAni.id);
                if (p == mdl->animsIds.end())
                {
                    missedAnims.addMissed(gobj.modelId, objAni.id);
                    objAni.id = -1;
                    continue;
                }
                objAni.animIdx = p->second;
                auto& mdlAnim = mdl->model.animations[objAni.animIdx];

                if (objAni.animEnd) {
                    if (gobj.id == 1) {
                        printf("!");
                    }
                    objAni.animEnd = 0;
                    objAni.prevMoveRoot = { 0,0,0 };
                    if (objAni.prevId != objAni.id) {
                    } 
                    else if (!objAni.bitField.repeat) {
                        objAni.id = objAni.nextId;
                        objAni.flags = 1;
                    }
                    else {
                        while (objAni.animTime > mdlAnim.duration) {
                            objAni.animTime -= mdlAnim.duration;
                        }
                    }
                }

                if (objAni.prevId != objAni.id) {
                    objAni.prevMoveRoot = { 0,0,0 };
                    objAni.animTime = 0;
                }

				objAni.animTime += timeDelta;
                int oldFrame = objAni.animFrame;
				objAni.animFrame = (objAni.animTime * resources->config.fps);
                objAni.newFrame = objAni.animFrame != oldFrame;
				auto& curFrame = objAni.animFrame;

                auto frameCount = mdlAnim.bakedPoses.size();

                if (curFrame >= frameCount) {
                    curFrame = frameCount - 1;
                    objAni.animEnd = 1;
                }
                if (objAni.id != -1) {
                    objAni.moveRoot = mdlAnim.rootMotion[curFrame].translation;
                }

                gobj.prevModelId = gobj.modelId;
                objAni.prevId = objAni.id;
			}

        }
	};

}