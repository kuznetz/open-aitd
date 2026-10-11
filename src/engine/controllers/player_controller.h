#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;
namespace openAITD {

	class PlayerController {
	public:
		World* world;
		Resources* resources;
		const int quickSearchAnimId = 2;
		
		PlayerController(World* world) {
			this->world = world;
			this->resources = world->resources;
		}

		void processAim(GameObject& gobj, const float timeDelta) {
			float rotate = 0;
			if (IsKeyDown(KEY_LEFT)) {
				rotate = 1;
			}
			if (IsKeyDown(KEY_RIGHT)) {
				rotate = -1;
			}
			if (rotate != 0) {
				auto newRot = gobj.getOrigRotation();
				newRot.y += rotate * PI * timeDelta;
				gobj.setOrigRotation(newRot);
			}
		}

		void processMove(GameObject& gobj, const float timeDelta) {
			bool isAction = false;
			auto& playerAnims = world->player.animations;
			int nextAnimation = gobj.animation.id;

			if (gobj.animation.bitField.uninterruptable) {
				isAction = true;
			}

			// A scripted one-shot animation must play to its end
			//  and must not be overwritten by idle/input
			if (
				!gobj.animation.bitField.repeat &&
				gobj.bitField.animated
			) {
				isAction = true;
			}

			//Fix for some scripts
			if (
				playerAnims.walkForw == nextAnimation ||
				playerAnims.idle == nextAnimation
			) {
				isAction = false;
			}

			// quickSearch
			if (world->player.quickSearch) {
				gobj.setOnceAnimation(quickSearchAnimId, playerAnims.idle);
				return;
			}

			//Rotate Player
			if (!isAction) {
				float rotate = 0;
				if (IsKeyDown(KEY_LEFT)) {
					rotate = 1;					
					nextAnimation = playerAnims.turnCCW;
				}
				if (IsKeyDown(KEY_RIGHT)) {
					rotate = -1;
					nextAnimation = playerAnims.turnCW;
				}
				if (rotate != 0) {
					auto newRot = gobj.getOrigRotation();
					newRot.y += rotate * PI * timeDelta;
					gobj.setOrigRotation(newRot);
					isAction = true;
				}

				//Get Move Vec
				//gobj.physics.moveVec = { 0,0,0 };
				float move = 0;
				if (IsKeyDown(KEY_DOWN)) {
					move = -0.75;
					nextAnimation = playerAnims.walkBackw;
				}
				else if (IsKeyDown(KEY_UP)) {
					if (IsKeyDown(KEY_LEFT_SHIFT)) {
						move = 3;
						nextAnimation = playerAnims.runForw;
					}
					else {
						move = 0.75;
						nextAnimation = playerAnims.walkForw;
					}
				}
				if (move != 0) {
					isAction = true;
				}
			}

			if (!isAction) {
				nextAnimation = playerAnims.idle;
			}
			if (nextAnimation != gobj.animation.id) {
				gobj.setRepeatAnimation(nextAnimation);
			}
		}

		void process(const float timeDelta) {
			if (world->player.objectId == -1) return;
			auto& gobj = world->gobjects[world->player.objectId];
			if (world->player.aiming) {
				processAim(gobj, timeDelta);
				world->player.aiming = false;
			} else if (world->player.movable) {
				processMove(gobj, timeDelta);
				world->player.movable = false;
			}
		}

	};

}