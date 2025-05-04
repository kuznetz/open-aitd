#pragma once
#include <vector>
#include <string>
#include "../raylib.h"
#include "../world/world.h"
#include "../resources/resources.h"

using namespace std;
using namespace raylib;
namespace openAITD {

	class FoundScreen {
	public:
		World* world;
		Resources* resources;
		bool leave = false;

		FoundScreen(World* world) {
			this->world = world;
			this->resources = world->resources;
		}

		~FoundScreen() {
		}

		void submit() {
			auto& gobj = this->world->gobjects[world->foundItem];
			if (leave) {
				gobj.invItem.foundTimeout = world->chrono + 5;
				world->foundItem = -1;
				return;
			}
			world->take(world->foundItem);
			world->foundItem = -1;
		}

		void process(float timeDelta) {
			if (IsKeyPressed(KEY_LEFT)) {
				leave = true;
			}
			if (IsKeyPressed(KEY_RIGHT)) {
				leave = false;
			}
			if (IsKeyPressed(KEY_ESCAPE)) {
				leave = true;
				submit();
				return;
			}
			if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
				submit();
				return;
			}

			BeginDrawing();

			const auto& screenW = this->resources->config.screenW;
			const auto& screenH = this->resources->config.screenH;
			ClearBackground(BLACK);

			auto& gobj = this->world->gobjects[world->foundItem];
			auto& name = resources->texts[gobj.invItem.nameId];

			auto& f = resources->mainFont;
			const char* m = "New item:";
			raylib::Rectangle r = { 0, screenH * 0.05, screenW, 0 };
			resources->drawCentered("New item:", r, WHITE);
			r.y += f.baseSize;
			resources->drawCentered(name.c_str(), r, GOLD);

			r = { (screenW / 4.f), (screenH * 0.95f) - f.baseSize, (screenW / 4.f), 0 };
			resources->drawCentered("Leave", r, leave ? YELLOW : GRAY);
			r = { r.x + r.width, r.y, r.width, 0 };
			resources->drawCentered("Take", r, leave ? GRAY : YELLOW);

			EndDrawing();
		}

	};

}