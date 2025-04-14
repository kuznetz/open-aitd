#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;
namespace openAITD {

	class PlayerController {
	public:
		World* world;
		Resources* resources;
		GameObject* player = 0;
		
		PlayerController(Resources* res, World* world) {
			this->resources = res;
			this->world = world;
		}

		void process(float timeDelta) {
			player = 0;
			for (int i = 0; i < world->gobjects.size(); i++) {
				auto& gobj = world->gobjects[i];
				if (gobj.track.mode == GOTrackMode::manual) {
					player = &gobj;
					break;
				}
			}
			if (!player || !player->moveFlag) return;

			bool isAction = false;
			int nextAnimation = player->animation.id;

			//if (player->track.mode == GOTrackMode::manual) {
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
					auto r = player->location.rotation;
					player->location.rotation = QuaternionMultiply(r, q);
					isAction = true;
				}

				//Get Move Vec
				//player->physics.moveVec = { 0,0,0 };
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
			if (nextAnimation != player->animation.id) {
				world->setRepeatAnimation(*player, nextAnimation);
			}

			world->player.space = IsKeyDown(KEY_SPACE);

			bool teleportPlayer = false;
			int newStageId = world->curStageId;
			int newRoomId = world->curRoomId;
			if (IsKeyPressed(KEY_D) && (newRoomId < world->curStage->rooms.size() - 1)) {
				newRoomId++;
				teleportPlayer = true;
			}
			if (IsKeyPressed(KEY_A) && (world->curRoomId > 0)) {
				newRoomId--;
				teleportPlayer = true;
			}
			if (IsKeyPressed(KEY_W)) {
				newStageId++;
				newRoomId = 0;
				teleportPlayer = true;
			}
			if (IsKeyPressed(KEY_S) && (newStageId > 0)) {
				newStageId--;
				newRoomId = 0;
				teleportPlayer = true;
			}
			if (IsKeyDown(KEY_T)) {
				teleportPlayer = true;
			}
			if (teleportPlayer) {
				//world->setCurRoom(newStageId, newRoomId);
				player->location.stageId = newStageId;
				player->location.roomId = newRoomId;
				player->location.position = { 0,0,0 };//resources->stages[world->curStageId].rooms[newRoom].position;
			}

			world->setCurRoom(player->location.stageId, player->location.roomId);
			player->moveFlag;
		}
	};

}