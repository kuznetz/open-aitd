#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../raylib.h"

using namespace std;
using namespace raylib;
namespace openAITD {

	class InventoryScreen {
	public:
		World* world;
		Resources* resources;
		bool exit;

		InventoryScreen(World* world) {
			this->world = world;
			this->resources = world->resources;
		}

		~InventoryScreen() {
		}

		void reload() {
			exit = false;
		}

		void process(float timeDelta) {
			const auto& screenW = this->resources->config.screenW;
			const auto& screenH = this->resources->config.screenH;
			
			if (IsKeyPressed(KEY_SPACE)) {
				exit = true;
				return;
			}

			BeginDrawing();

			ClearBackground(BLACK);

			auto& f = resources->mainFont;
			const char* m = "Inventory";
			auto mt = MeasureTextEx(f, m, f.baseSize, 0);
			Vector2 v = { (int)(screenW - mt.x) / 2, screenH - (f.baseSize * 2) };
			//DrawTextEx(f, m, v, f.baseSize, 0, WHITE);

			EndDrawing();
		}

	};

}