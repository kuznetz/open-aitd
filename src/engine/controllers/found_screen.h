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

		void drawCentered(const char* text, raylib::Rectangle r, Color color) {
			auto& f = resources->mainFont;
			auto mt = MeasureTextEx(f, text, f.baseSize, 0);
			int x = (int)(r.x + ((r.width - mt.x) / 2));
			Vector2 v = { x, (int)r.y };						
			raylib::DrawTextEx(f, text, v, f.baseSize, 0, color);
		}

		void submit() {
			if (leave) {
				world->foundItem = -1;
				return;
			}

			world->foundItem = -1;
		}

		void process(float timeDelta) {
			if (IsKeyPressed(KEY_LEFT)) {
				leave = true;
			}
			if (IsKeyPressed(KEY_RIGHT)) {
				leave = false;
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
			raylib::Rectangle r = { 0, screenH * 0.10, screenW, 0 };
			drawCentered("New item:", r, WHITE);
			r.y += f.baseSize;
			drawCentered(name.c_str(), r, GOLD);

			r = { 0, screenH * 0.8f, (screenW/2.f), 0 };
			drawCentered("Leave", r, leave ? YELLOW : GRAY);
			r = { r.width, r.y, r.width, 0 };
			drawCentered("Take", r, leave ? GRAY : YELLOW);

			EndDrawing();
		}

	};

}