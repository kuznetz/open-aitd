﻿#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;
namespace openAITD {

	const float frameT = 1. / 60;

	class AnimationController {
	public:
		World* world;
		Resources* resources;
		
		AnimationController(Resources* res, World* world) {
			this->resources = res;
			this->world = world;
		}

		void process(float timeDelta) {
			for (int i = 0; i < this->world->gobjects.size(); i++) {
				auto& gobj = this->world->gobjects[i];
				if (gobj.location.stageId != this->world->curStageId) continue;
				if (gobj.model.id == -1) continue;
				if (gobj.model.animId == -1) continue;

				auto mdl = resources->models.getModel(gobj.model.id);
				auto anim = mdl->animations[gobj.model.animId];
				if (anim == 0) {
					printf("Miss animation %d in model %d", gobj.model.animId, gobj.model.id);
					continue;
				}
				gobj.model.animTime += timeDelta;
				int curFrame = (gobj.model.animTime / frameT);
				while (curFrame >= anim->frameCount) {
					gobj.model.animTime -= anim->frameCount * frameT;
					curFrame -= anim->frameCount;
				}
				UpdateModelAnimation(mdl->model, *anim, curFrame);
			}
		}
	};

}