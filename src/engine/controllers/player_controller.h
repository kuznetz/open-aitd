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

		void processObj(GameObject& gobj, float timeDelta) {
			bool isAction = false;
			int nextAnimation = gobj.animation.id;

			//if (gobj.track.mode == GOTrackMode::manual) {
			//	isAction = true;
			//}

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

			bool teleportPlayer = false;
			int newStageId = world->curStageId;
			int newRoomId = world->curRoomId;
			if (IsKeyPressed(KEY_KP_MULTIPLY) && (newRoomId < world->curStage->rooms.size() - 1)) {
				newRoomId++;
				teleportPlayer = true;
			}
			if (IsKeyPressed(KEY_KP_DIVIDE) && (world->curRoomId > 0)) {
				newRoomId--;
				teleportPlayer = true;
			}
			if (IsKeyPressed(KEY_KP_ADD)) {
				newStageId++;
				newRoomId = 0;
				teleportPlayer = true;
			}
			if (IsKeyPressed(KEY_KP_SUBTRACT) && (newStageId > 0)) {
				newStageId--;
				newRoomId = 0;
				teleportPlayer = true;
			}
			if (IsKeyDown(KEY_KP_ENTER)) {
				teleportPlayer = true;
			}
			if (teleportPlayer) {
				//world->setCurRoom(newStageId, newRoomId);
				gobj.location.stageId = newStageId;
				gobj.location.roomId = newRoomId;
				gobj.location.position = { 0,0,0 };//resources->stages[world->curStageId].rooms[newRoom].position;
			}

			world->setCurRoom(gobj.location.stageId, gobj.location.roomId);
		}
	};

}