#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../../raylib-cpp/raylib-cpp.h"

using namespace std;
using namespace raylib;
namespace openAITD {

	class BookScreen {
	public:
		World* world;
		Resources* resources;
		
		BookScreen(World* world) {
			this->world = world;
			this->resources = world->resources;
		}

		~BookScreen() {
		}

		void process(float timeDelta) {
			auto& pic = this->world->picture;
			pic.curTime += timeDelta;
			if (pic.curTime > pic.delay) {
				pic.id = -1;
			}
		}

		void render() {
			float screenW = this->resources->config.screenW;
			float screenH = this->resources->config.screenH;
			auto& pic = this->world->picture;
			auto& texture = this->resources->backgrounds.loadPicture(pic.id);
			DrawTexturePro(
				texture,
				{ 0, 0, screenW, screenH },
				{ 0, 0, screenW, screenH },
				{ 0, 0 }, 0, WHITE
			);
		}

	};

}