#pragma once
#include <vector>
#include <string>
#include "../raylib-cpp.h"
#include "../resources/resources.h"

using namespace std;
using namespace raylib;
namespace openAITD {

	struct BaseMenuItem {
		int id;
		string name;
	};

	class BaseMenu {
	public:
		Resources* resources = 0;
		vector<BaseMenuItem> items;
		raylib::Rectangle rect = { 0,0,0,0 };
		bool middle = true;
		int maxRenderItems = 5;

		bool active = true;
		int curItemIdx = 0;
		int curItemId = -1;
		int itemsOffs = 0;

		BaseMenu() {
			//rect = { 0, resources->config.screenH * 0.05f, (float)resources->config.screenW, 0.f };
			//this->resources = world->resources;
			//reload();
		}

		~BaseMenu() {
		}

		void reset() {
			curItemIdx = 0;
			curItemId = items[curItemIdx].id;
			itemsOffs = 0;
		}

		void nextItem() {
			if (curItemIdx >= items.size() - 1) return;
			curItemIdx++;
			if (curItemIdx >= (itemsOffs + maxRenderItems - 1)) itemsOffs++;
			curItemId = items[curItemIdx].id;
		}

		void prevItem() {
			if (curItemIdx <= 0) return;
			curItemIdx--;
			if (curItemIdx - 1 < itemsOffs && itemsOffs > 0) itemsOffs--;
			curItemId = items[curItemIdx].id;
		}

		void processKeys() {
			if (!active) return;
			if (IsKeyPressed(KEY_UP)) {
				prevItem();
			}
			if (IsKeyPressed(KEY_DOWN)) {
				nextItem();
			}
		}

		void render() {
			auto& f = resources->screen.mainFont;
			raylib::Rectangle r = rect;

			if (middle) {
				auto& f = resources->screen.mainFont;
				auto mt = MeasureTextEx(f, "Q", f.baseSize, 0);
				int height = items.size();
				if (height > maxRenderItems) {
					height = maxRenderItems;
				}
				height = mt.y * height;
				r.y = (int)(r.height - height) / 2;
			}

			for (int i = 0; i < maxRenderItems; i++) {
				int id = i + itemsOffs;
				if (id >= items.size()) break;
				auto& name = items[id].name;
				Color c = GRAY;
				if (active) {
					c = (id == curItemIdx) ? GOLD : WHITE;
				}
				resources->screen.drawCentered(name.c_str(), r, c);
				r.y += f.baseSize;
			}
		}

	};

}