#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../raylib.h"

using namespace std;
namespace openAITD {

	class InventoryScreen {
	public:
		World* world;
		Resources* resources;
		int screenW = 0;
		int screenH = 0;

		InventoryScreen(World* world) {
			this->world = world;
			this->resources = world->resources;
		}

		~InventoryScreen() {
		}

		void process() {
			ClearBackground(BLACK);

			auto& f = resources->mainFont;
			const char* m = "Inventory";
			auto mt = MeasureTextEx(f, m, f.baseSize, 0);
			Vector2 v = { (int)(screenW - mt.x) / 2, screenH - (f.baseSize * 2) };
			DrawTextEx(f, m, v, f.baseSize, 0, WHITE);
		}

	};

}