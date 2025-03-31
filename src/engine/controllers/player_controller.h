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
			if (!player) return;
			Vector3 move = { 0,0,0 };
            if (IsKeyDown(KEY_LEFT)) {
				move.x = -1;
            }
            if (IsKeyDown(KEY_RIGHT)) {
				move.x = 1;
            }
            if (IsKeyDown(KEY_DOWN)) {
				move.z = -1;
            }
			if (IsKeyDown(KEY_UP)) {
				move.z = 1;
			}
			player->location.position += Vector3Scale(move, 4 * timeDelta);

			if (IsKeyDown(KEY_SPACE)) {
				player->location.stageId = world->curStageId;
				auto newRoom = resources->stages[world->curStageId].cameras[world->curCameraId].rooms[0].roomId;
				player->location.roomId = newRoom;
				player->location.position = { 0,0,0 };//resources->stages[world->curStageId].rooms[newRoom].position;
			}

		}
	};

}