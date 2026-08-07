#pragma once
#include "../resources/missed_anims.h"
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;
using namespace raylib;

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
                if (gobj.modelId == -1) continue; 
				if (gobj.getStageId() != this->world->curStageId) continue;
                if (!world->isObjectActive(gobj)) continue;

                auto& objAni = gobj.animation;
                if (!gobj.bitField.animated) {
                    //objAni.animChanged = false;
                    //objAni.animEnd = 0;
                    //objAni.keyFrameIdx = 0;
                    //objAni.animFrame = 0;
                    continue;
                }

                objAni.animChanged = false;
				if (objAni.id == -1) continue;
                auto mdl = resources->models.getModel(gobj.modelId, world->altModels);

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
                        if (objAni.nextId == -1) {
                            gobj.bitField.animated = 0;
                        } else {
                            objAni.id = objAni.nextId;
                            objAni.animTime = 0;
                            objAni.flags = 0;
                            objAni.bitField.repeat = 1;
                        }
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
                auto newFrameIdx = mdl->model.getKeyFrame(mdlAnim, objAni.animTime);
                if (newFrameIdx != objAni.keyFrameIdx) {
                    objAni.keyFrameSoundIdx = -1;
                    objAni.keyFrameIdx = mdl->model.getKeyFrame(mdlAnim, objAni.animTime);
                }

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

                objAni.moveRoot = Vector3Transform(objAni.moveRoot, MatrixRotateY(PI));
                objAni.moveRoot.x = -objAni.moveRoot.x;

                gobj.prevModelId = gobj.modelId;
                objAni.prevId = objAni.id;

                if (objAni.id != -1) {
                    processPose(gobj);
                }
			}

        }

        void processPose(GameObject& gobj) {
			if (gobj.animation.animIdx < 0) return;

            auto rmodel = resources->models.getModel(gobj.modelId, world->altModels);
            auto& model = rmodel->model;

			int bonesSize = model.skin->joints_count;
			if (bonesSize != gobj.animation.transitionPose.size()) {
                gobj.animation.fromPose.resize(bonesSize);
				gobj.animation.transitionPose.resize(bonesSize);
				gobj.animation.hasPose = false;
			}

			auto& curPose = gobj.animation.curPose;
			auto& curAnim = model.animations[gobj.animation.animIdx];
			auto& newPose = curAnim.bakedPoses[gobj.animation.animFrame];
			bool isTransition = (curAnim.duration > 0) && (gobj.animation.animTime <= curAnim.transition);

            if (gobj.animation.animChanged) {
                if (gobj.animation.hasPose) {
                    //Set current pose to transition start
                    memcpy_s(
                        gobj.animation.fromPose.data(), bonesSize * sizeof(Transform),
                        curPose, bonesSize * sizeof(Transform)
                    );
                } else {
                    //Init fromPose
                    memcpy_s(
                        gobj.animation.fromPose.data(), bonesSize * sizeof(Transform),
                        newPose.data(), bonesSize * sizeof(Transform)
                    );
                    gobj.animation.hasPose = true;
                }
            }

            if (curAnim.duration == 0) {
                curPose = newPose.data();
			}
            else if (isTransition && gobj.animation.hasPose) {
				model.PoseLerp(gobj.animation.transitionPose.data(), gobj.animation.fromPose.data(), newPose.data(), gobj.animation.animTime / curAnim.transition);
				curPose = gobj.animation.transitionPose.data();
			}
			else {
				curPose = newPose.data();
			}
        	
        }

    private:
	};

}