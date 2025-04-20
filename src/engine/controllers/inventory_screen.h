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
		
		int curItemIdx = 0;
		int itemsOffs = 0;
		bool firstFrame = true;
		bool selAction = false;
		bool exit;

		InventoryScreen(World* world) {
			this->world = world;
			this->resources = world->resources;
			reload();
		}

		~InventoryScreen() {
		}

		void reload() {
			curItemIdx = 0;
			itemsOffs = 0;
			exit = false;
			selAction = false;
			firstFrame = true;
		}

		void drawLines() {
			auto& w = resources->config.screenW;
			auto& h = resources->config.screenH;
			DrawLineEx({ 0 ,(float)(h / 2) }, { (float)w ,(float)(h / 2) }, 2, GRAY);
			DrawLineEx({ (float)(w / 2) ,(float)(h / 2) }, { (float)(w / 2) ,(float)h }, 2, GRAY);
		}

		void drawItems() {
			auto& f = resources->mainFont;
			raylib::Rectangle r = { 0, resources->config.screenH * 0.05, resources->config.screenW, 0 };
			for (int i = 0; i < 5; i++) {
				int id = i+itemsOffs;
				if (id >= world->inventory.size()) break;
				auto& gobj = *world->inventory[id];
				auto& name = resources->texts[gobj.invItem.nameId];
				Color c = (id == curItemIdx) ? GOLD : WHITE;
				resources->drawCentered(name.c_str(), r, c);
				r.y += f.baseSize;
			}
		}

		void nextItem() {
			if (curItemIdx >= world->inventory.size() - 1) return;
			curItemIdx++;
			if (curItemIdx >= (itemsOffs + 4)) itemsOffs++;
		}

		void prevItem() {
			if (curItemIdx <= 0) return;
			curItemIdx--;
			if (curItemIdx-1 < itemsOffs && itemsOffs > 0) itemsOffs--;
		}

		void drawActions() {
			auto& f = resources->mainFont;
			raylib::Rectangle r = { 0, resources->config.screenH * 0.05, resources->config.screenW, 0 };
			for (int i = 0; i < 5; i++) {
				if (i >= world->inventory.size()) break;
				auto& gobj = *world->inventory[i];
				auto& name = resources->texts[gobj.invItem.nameId];
				resources->drawCentered(name.c_str(), r, WHITE);
				r.y += f.baseSize;
			}
		}

		void processKeys() {
			if (IsKeyPressed(KEY_ESCAPE)) {
				if (!selAction) {
					exit = true;
					return;
				}
				else {
					selAction = false;
				}
			}
			if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
				if (!selAction) {
					selAction = true;
				}
				else {

				}
			}
			if (IsKeyPressed(KEY_UP)) {
				if (!selAction) {
					prevItem();
				}
				else {

				}
			}
			if (IsKeyPressed(KEY_DOWN)) {
				if (!selAction) {
					nextItem();
				}
				else {

				}
			}
		}

		void process(float timeDelta) {
			const auto& screenW = this->resources->config.screenW;
			const auto& screenH = this->resources->config.screenH;
			
			if (!firstFrame) {
				processKeys();
			}

			BeginDrawing();
			ClearBackground(BLACK);
			drawLines();
			drawItems();
			EndDrawing();
			firstFrame = false;
		}

	};

}