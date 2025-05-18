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
		int foundItem = -1;

		FoundScreen(World* world) {
			this->world = world;
			this->resources = world->resources;
		}

		~FoundScreen() {
		}

		void submit() {
			auto& gobj = this->world->gobjects[foundItem];
			if (leave) {
				gobj.invItem.foundTimeout = world->chrono + 5;
				foundItem = -1;
				return;
			}
			world->take(foundItem);
			foundItem = -1;
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
		}

		void render() {
			const auto& screenW = this->resources->config.screenW;
			const auto& screenH = this->resources->config.screenH;

			auto& gobj = this->world->gobjects[foundItem];
			auto& name = resources->texts[gobj.invItem.nameId];

			auto& f = resources->screen.mainFont;
			const char* m = "New item:";
			raylib::Rectangle r = { 0, screenH * 0.05, screenW, 0 };
			resources->screen.drawCentered("New item:", r, WHITE);
			r.y += f.baseSize;
			resources->screen.drawCentered(name.c_str(), r, GOLD);

			r = { (screenW / 4.f), (screenH * 0.95f) - f.baseSize, (screenW / 4.f), 0 };
			resources->screen.drawCentered("Leave", r, leave ? YELLOW : GRAY);
			r = { r.x + r.width, r.y, r.width, 0 };
			resources->screen.drawCentered("Take", r, leave ? GRAY : YELLOW);
		}

		void main(int newFoundItem) {
			bool firstFrame = true;
			foundItem = newFoundItem;
			int timeDelta;
			while (foundItem != -1) {
				timeDelta = GetFrameTime();
				if (firstFrame) {
					firstFrame = false;
				}
				else {
					process(timeDelta);
				}
				if (foundItem == -1) break;
				resources->screen.begin();
				render();
				resources->screen.end();
			}
		}

	};

}