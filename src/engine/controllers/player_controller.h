#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;
namespace openAITD {

	class PlayerController {
	public:
		World* world;
		Resources* resources;
		
		PlayerController(World* world) {
			this->world = world;
			this->resources = world->resources;
		}

		void processRotate(GameObject& gobj, float timeDelta) {
			float rotate = 0;
			if (IsKeyDown(KEY_LEFT)) {
				rotate = 1;
			}
			if (IsKeyDown(KEY_RIGHT)) {
				rotate = -1;
			}
			if (rotate != 0) {
				rotate = rotate * PI * timeDelta;
				auto q = QuaternionFromAxisAngle({ 0,1,0 }, rotate);
				auto r = gobj.location.rotation;
				gobj.location.rotation = QuaternionMultiply(r, q);
			}
		}

		void processObj(GameObject& gobj, float timeDelta) {
			bool isAction = false;
			int nextAnimation = gobj.animation.id;

			//if (gobj.track.mode == GOTrackMode::manual) {
			if (!gobj.animation.bitField.repeat) {
				isAction = true;
			}

			//Rotate Player
			if (!isAction) {
				float rotate = 0;
				if (IsKeyDown(KEY_LEFT)) {
					rotate = 1;					
					nextAnimation = world->player.animations.turnCCW;
				}
				if (IsKeyDown(KEY_RIGHT)) {
					rotate = -1;
					nextAnimation = world->player.animations.turnCW;
				}
				if (rotate != 0) {
					rotate = rotate * PI * timeDelta;
					auto q = QuaternionFromAxisAngle({ 0,1,0 }, rotate);
					auto r = gobj.location.rotation;
					gobj.location.rotation = QuaternionMultiply(r, q);
					isAction = true;
				}

				//Get Move Vec
				//gobj.physics.moveVec = { 0,0,0 };
				float move = 0;
				if (IsKeyDown(KEY_Z)) {
					move = 0.25;
					nextAnimation = 2;
					isAction = true;
				}
				else if (IsKeyDown(KEY_X)) {
					move = 0.25;
					nextAnimation = 5;
					isAction = true;
				}
				else if (IsKeyDown(KEY_C)) {
					move = 0.25;
					nextAnimation = 270;
					isAction = true;
				}
				else if (IsKeyDown(KEY_DOWN)) {
					move = -0.75;
					nextAnimation = world->player.animations.walkBackw;
				}
				else if (IsKeyDown(KEY_UP)) {
					if (IsKeyDown(KEY_LEFT_SHIFT)) {
						move = 3;
						nextAnimation = world->player.animations.runForw;
					}
					else {
						move = 0.75;
						nextAnimation = world->player.animations.walkForw;
					}
				}
				if (move != 0) {
					isAction = true;
				}
			}

			if (!isAction) {
				nextAnimation = world->player.animations.idle;
			}
			if (nextAnimation != gobj.animation.id) {
				world->setRepeatAnimation(gobj, nextAnimation);
			}
		}
	};

}