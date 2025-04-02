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
				move = -move * timeDelta;
				auto& m = QuaternionToMatrix(player->location.rotation);
				auto& p = player->location.position;
				Vector3 v = { m.m8, m.m9, m.m10 };
				player->location.position = Vector3Add(p, Vector3Scale(v, move));
			}
			if (move == 0 && rotate == 0) {
				player->model.animId = anims.idle;
			}

			if (world->curStageId == player->location.stageId) {
				auto& curStage = resources->stages[world->curStageId];
				auto camId = curStage.closestCamera(player->location.position);
				if (camId != -1) {
					world->curCameraId = camId;

					//TODO: Replace camera change to trigger					
					Vector3 oldRoomPos = curStage.rooms[player->location.roomId].position;
					player->location.roomId = curStage.cameras[camId].rooms[0].roomId;
					Vector3 newRoomPos = curStage.rooms[player->location.roomId].position;
					player->location.position = Vector3Subtract(Vector3Add(player->location.position, oldRoomPos), newRoomPos);
				}
			}

			if (IsKeyDown(KEY_SPACE)) {
				player->location.stageId = world->curStageId;
				auto newRoom = resources->stages[world->curStageId].cameras[world->curCameraId].rooms[0].roomId;
				player->location.roomId = newRoom;
				player->location.position = { 0,0,0 };//resources->stages[world->curStageId].rooms[newRoom].position;
			}

		}
	};

}