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
			//Rotate Player
			float rotate = 0;
            if (IsKeyDown(KEY_LEFT)) {
				rotate = 1;
				player->model.animId = anims.turnCCW;
            }
            if (IsKeyDown(KEY_RIGHT)) {
				rotate = -1;
				player->model.animId = anims.turnCW;
			}
			if (rotate != 0) {
				rotate = rotate * PI * timeDelta;
				auto q = QuaternionFromAxisAngle({ 0,1,0 }, rotate);
				auto r = player->location.rotation;
				player->location.rotation = QuaternionMultiply(r, q);
			}

			//Get Move Vec
			player->location.moveVec = { 0,0,0 };
			float move = 0;
			if (IsKeyDown(KEY_DOWN)) {
				move = -1;
				player->model.animId = anims.walkBackw;
			}
			if (IsKeyDown(KEY_UP)) {
				if (IsKeyDown(KEY_LEFT_SHIFT)) {
					move = 3;
					player->model.animId = anims.runForw;
				} else {
					move = 1;
					player->model.animId = anims.walkForw;
				}
			}
			if (move != 0) {
				auto& p = player->location.position;
				Vector3 v = { 0, 0, -move * timeDelta };
				player->location.moveVec = Vector3RotateByQuaternion(v, player->location.rotation);
			}
			if (move == 0 && rotate == 0) {
				player->model.animId = anims.idle;
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
			if (IsKeyPressed(KEY_D) && (world->curCameraId < curStage.cameras.size() - 1)) {
				if (world->curCameraId++);
				teleportPlayer = true;
			}
			if (IsKeyPressed(KEY_A) && (world->curCameraId > 0)) {
				if (world->curCameraId--);
				teleportPlayer = true;
			}
			if (IsKeyPressed(KEY_W)) {
				world->curStageId++;
				world->curCameraId = 0;
				teleportPlayer = true;
			}
			if (IsKeyPressed(KEY_S)) {
				teleportPlayer = true;
				world->curStageId--;
				world->curCameraId = 0;
			}
			if (IsKeyDown(KEY_SPACE)) {
				teleportPlayer = true;
			}
			if (teleportPlayer) {
				player->location.stageId = world->curStageId;
				auto newRoom = resources->stages[world->curStageId].cameras[world->curCameraId].rooms[0].roomId;
				player->location.roomId = newRoom;
				player->location.position = { 0,0,0 };//resources->stages[world->curStageId].rooms[newRoom].position;
			}

			world->curRoomId = player->location.roomId;
		}
	};

}