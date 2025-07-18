﻿#pragma once
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

		void process(float timeDelta) {
			for (int i = 0; i < this->world->gobjects.size(); i++) {
				auto& gobj = this->world->gobjects[i];
				if (gobj.location.stageId != this->world->curStageId) continue;

                //don't process without flag. Need for sitting enemies (lady, zombies)
				//TODO: if (gobj.modelId == -1 || !gobj.bitField.animated) continue; 
                if (gobj.modelId == -1) continue;

                if (!gobj.bitField.animated) {
                    gobj.animation.animChanged = false;
                    gobj.animation.animEnd = 0;
                    continue;
                }
                auto& objAni = gobj.animation;
                objAni.animChanged = false;
				if (objAni.id == -1) continue;
                auto mdl = resources->models.getModel(gobj.modelId);

                if (gobj.modelId != gobj.prevModelId) {
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

                objAni.animTime += timeDelta;

                if (objAni.id != objAni.prevId) {
                    objAni.animChanged = true;
                    objAni.animEnd = 0;
                    objAni.animTime = 0;
                }

                if (objAni.animEnd) {
                    objAni.animEnd = 0;
                    objAni.animChanged = true;
                    if (!objAni.bitField.repeat) {
                        objAni.id = objAni.nextId;
                        objAni.animTime = 0;
                        objAni.flags = 0;
                        objAni.bitField.repeat = 1;
                    }
                    else if (mdlAnim.duration > 0) {
                        while (objAni.animTime >= mdlAnim.duration) {
                            objAni.animTime -= mdlAnim.duration;
                        }
                    }
                }

				objAni.animFrame = (objAni.animTime * resources->config.targetFps);
				auto& curFrame = objAni.animFrame;

                //For logic
                objAni.keyFrameIdx = mdl->model.getKeyFrame(mdlAnim, objAni.animTime);

                auto lastFrame = mdlAnim.bakedPoses.size() - 1;
                if (curFrame >= lastFrame) {
                    curFrame = lastFrame;
                    objAni.animEnd = 1;
                }

                if (objAni.animChanged) {
                    objAni.prevMoveRoot = { 0,0,0 };
                }
                else {
                    objAni.prevMoveRoot = objAni.moveRoot;
                }
                if (objAni.id != -1 && mdlAnim.duration > 0) {
                    objAni.moveRoot = mdlAnim.rootMotion[curFrame].translation;
                }
                else {
                    objAni.moveRoot = { 0,0,0 };
                }

                gobj.prevModelId = gobj.modelId;
                objAni.prevId = objAni.id;
			}

        }
	};

}