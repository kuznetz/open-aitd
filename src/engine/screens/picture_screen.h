#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../raylib.h"

using namespace std;
using namespace raylib;
namespace openAITD {

	class PictureScreen {
	public:
		World* world;
		Resources* resources;
		
		PictureScreen(World* world) {
			this->world = world;
			this->resources = world->resources;
		}

		~PictureScreen() {
		}

		void process(float timeDelta) {
			float screenW = this->resources->config.screenW;
			float screenH = this->resources->config.screenH;

			auto& pic = this->world->picture;
			auto& texture = this->resources->backgrounds.loadPicture(pic.id);
			BeginDrawing();
			DrawTexturePro(
				texture,
				{ 0, 0, screenW, screenH },
				{ 0, 0, screenW, screenH },
				{ 0, 0 }, 0, WHITE
			);
			EndDrawing();

			pic.curTime += timeDelta;
			if (pic.curTime > pic.delay) {
				pic.id = -1;
			}
		}

	};

}