#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;
namespace openAITD {

	struct PlayerAnimations {
		int idle = 4;
		int walkForw = 254;
		int runForw = 255;
		int idle2 = 4;
		int walkBackw = 256;
		int turnCW = 257;
		int turnCCW = 258;
	};

	class PlayerController {
	public:
		World* world;
		Resources* resources;
		GameObject* player = 0;
		PlayerAnimations anims;
		
		PlayerController(Resources* res, World* world) {
			this->resources = res;
			this->world = world;
		}

		void process(float timeDelta) {
			bool isAction = false;
			int nextAnimation = player->model.animId;

			if (player->trackMode == 0) {
				isAction = true;
			}

			//Rotate Player
			if (!isAction) {
				float rotate = 0;
				if (IsKeyDown(KEY_LEFT)) {
					rotate = 1;
					nextAnimation = anims.turnCCW;
				}
				if (IsKeyDown(KEY_RIGHT)) {
					rotate = -1;
					nextAnimation = anims.turnCW;
				}
				if (rotate != 0) {
					rotate = rotate * PI * timeDelta;
					auto q = QuaternionFromAxisAngle({ 0,1,0 }, rotate);
					auto r = player->location.rotation;
					player->location.rotation = QuaternionMultiply(r, q);
					isAction = true;
				}

				//Get Move Vec
				player->physics.moveVec = { 0,0,0 };
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
				else if (IsKeyDown(KEY_DOWN)) {
					move = -0.75;
					nextAnimation = anims.walkBackw;
				}
				else if (IsKeyDown(KEY_UP)) {
					if (IsKeyDown(KEY_LEFT_SHIFT)) {
						move = 3;
						nextAnimation = anims.runForw;
					}
					else {
						move = 0.75;
						nextAnimation = anims.walkForw;
					}
				}
				if (move != 0) {
					auto& p = player->location.position;
					Vector3 v = { 0, 0, -move * timeDelta };
					player->physics.moveVec = Vector3RotateByQuaternion(v, player->location.rotation);
					isAction = true;
				}
			}

			if (!isAction) {
				nextAnimation = anims.idle;
			}
			if (nextAnimation != player->model.animId) {
				world->setRepeatAnimation(*player, nextAnimation);
			}

			auto& curStage = resources->stages[world->curStageId];
			auto* curRoom = &curStage.rooms[player->location.roomId];
			if (world->curStageId == player->location.stageId) {
				//Select Camera
				Vector3 pos = Vector3Add(player->location.position, curRoom->position);
				auto camId = curStage.closestCamera(pos);
				if (camId != -1) {
					world->curCameraId = camId;
				}
			}

			bool teleportPlayer = false;
			if (IsKeyPressed(KEY_D) && (world->curRoomId < curStage.rooms.size() - 1)) {
				if (world->curRoomId++);
				teleportPlayer = true;
			}
			if (IsKeyPressed(KEY_A) && (world->curRoomId > 0)) {
				if (world->curRoomId--);
				teleportPlayer = true;
			}
			if (IsKeyPressed(KEY_W)) {
				world->curStageId++;
				world->curRoomId = 0;
				teleportPlayer = true;
			}
			if (IsKeyPressed(KEY_S)) {
				teleportPlayer = true;
				world->curStageId--;
				world->curRoomId = 0;
			}
			if (IsKeyDown(KEY_SPACE)) {
				teleportPlayer = true;
			}
			if (teleportPlayer) {
				world->curCameraId = 0;
				player->location.stageId = world->curStageId;
				player->location.roomId = world->curRoomId;
				player->location.position = { 0,0,0 };//resources->stages[world->curStageId].rooms[newRoom].position;
			}

			world->curRoomId = player->location.roomId;
		}
	};

}