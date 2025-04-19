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

		void drawItems() {
			auto& f = resources->mainFont;
			raylib::Rectangle r = { 0, resources->config.screenH * 0.05, resources->config.screenW, 0 };
			for (int i = 0; i < 5; i++) {
				if (i >= world->inventory.size()) break;
				auto& gobj = *world->inventory[i];
				auto& name = resources->texts[gobj.invItem.nameId];
				resources->drawCentered(name.c_str(), r, GOLD);
				r.y += f.baseSize;
			}
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
			drawItems();
			EndDrawing();
		}

	};

}