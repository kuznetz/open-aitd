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
		
		vector<int> curActions;
		int curItemIdx = 0;
		int curActionIdx = 0;
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
			reloadActions();
		}

		void reloadActions() {
			curActionIdx = 0;
			curActions.clear();
			if (!world->inventory.size()) return;
			auto& gobj = *world->inventory[curItemIdx];
			for (int i = 0; i < 11; i++) {
				if (gobj.invItem.flags & (1 << i))
				{
					curActions.push_back(i + 23);
				}
			}
		}

		void drawLines() {
			auto& w = resources->config.screenW;
			auto& h = resources->config.screenH;
			DrawLineEx({ 0 ,(float)(h / 2) }, { (float)w ,(float)(h / 2) }, 2, GRAY);
			DrawLineEx({ (float)(w / 2) ,(float)(h / 2) }, { (float)(w / 2) ,(float)h }, 2, GRAY);
		}

		void drawActions() {
			auto& f = resources->screen.mainFont;
			float h2 = (resources->config.screenH / 2);
			float YOffs = (h2 - (f.baseSize * curActions.size())) / 2;
			raylib::Rectangle r = { resources->config.screenW / 2, h2 + YOffs, resources->config.screenW / 2, 0 };
			for (int i = 0; i < 5; i++) {
				if (i >= curActions.size()) break;
				auto& name = resources->texts[curActions[i]];
				Color c = GRAY;
				if (selAction) {
					c = (i == curActionIdx) ? GOLD : WHITE;
				}
				resources->screen.drawCentered(name.c_str(), r, c);
				r.y += f.baseSize;
			}
		}

		void nextAction() {
			if (curActionIdx >= curActions.size() - 1) return;
			curActionIdx++;
		}

		void prevAction() {
			if (curActionIdx <= 0) return;
			curActionIdx--;
		}

		void drawItems() {
			auto& f = resources->screen.mainFont;
			raylib::Rectangle r = { 0, resources->config.screenH * 0.05, resources->config.screenW, 0 };
			for (int i = 0; i < 5; i++) {
				int id = i+itemsOffs;
				if (id >= world->inventory.size()) break;
				auto& gobj = *world->inventory[id];
				auto& name = resources->texts[gobj.invItem.nameId];
				Color c = GRAY;
				if (!selAction) {
					c = (id == curItemIdx) ? GOLD : WHITE;
				}
				resources->screen.drawCentered(name.c_str(), r, c);
				r.y += f.baseSize;
			}
		}

		void nextItem() {
			if (curItemIdx >= world->inventory.size() - 1) return;
			curItemIdx++;
			if (curItemIdx >= (itemsOffs + 4)) itemsOffs++;
			reloadActions();
		}

		void prevItem() {
			if (curItemIdx <= 0) return;
			curItemIdx--;
			if (curItemIdx-1 < itemsOffs && itemsOffs > 0) itemsOffs--;
			reloadActions();
		}

		void submit() {
			world->curInvGObject = world->inventory[curItemIdx];
			world->curInvAction = 1 << (curActions[curActionIdx] - 23);
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
					submit();
					exit = true;
					return;
				}
			}
			if (IsKeyPressed(KEY_UP)) {
				if (!selAction) {
					prevItem();
				}
				else {
					prevAction();
				}
			}
			if (IsKeyPressed(KEY_DOWN)) {
				if (!selAction) {
					nextItem();
				}
				else {
					nextAction();
				}
			}
		}

		void process(float timeDelta) {
			if (!firstFrame) {
				processKeys();
			}
			firstFrame = false;
		}

		void render() {
			drawLines();
			drawItems();
			drawActions();
		}

	};

}