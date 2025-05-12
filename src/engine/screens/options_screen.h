#pragma once
#include <vector>
#include <string>
#include "../raylib.h"
#include "../world/world.h"
#include "../resources/resources.h"
#include "./base_menu.h"

using namespace std;
using namespace raylib;
namespace openAITD {

	struct ScreenRes {
		int w, h;
	};
	vector<ScreenRes> resolutions = { {320,240}, {640,480}, {960,720}, {1280,960}, {1600,1200} };

	class OptionsScreen {
	public:
		World* world;
		Resources* resources;
		Config* config;
		Config newConfig;
		BaseMenu menu;
		int resolutionIdx = -1;
		bool firstFrame = true;
		bool exit = false;

		OptionsScreen(World* world) {
			this->world = world;
			this->resources = world->resources;
			this->config = &resources->config;
			menu.resources = this->resources;
			menu.maxRenderItems = 8;
		}

		~OptionsScreen() {
		}

		void reload() {
			newConfig = *config;
			firstFrame = true;
			resolutionIdx = -1;
			for (int i = 0; i < resolutions.size(); i++) {
				if (config->screenW == resolutions[i].w && config->screenW == resolutions[i].h) {
					resolutionIdx = i;
					break;
				}
			}

			menu.rect = { 0, 0, (float)config->screenW, (float)config->screenH };
			menu.middle = 1;
			reloadMenu();
			menu.reset();
		}

		void reloadMenu() {
			string fullscreen = string("Fullscreen: ") + (newConfig.fulllscreen ? "yes" : "no");
			string resol = string("Resolution: ") + to_string(newConfig.screenW) + "x" + to_string(newConfig.screenH);
			string showFps = string("Show fps: ") + (newConfig.showFps ? "yes" : "no");
			menu.items.clear();
			menu.items.push_back({ 1, fullscreen });
			if (!newConfig.fulllscreen) {
				menu.items.push_back({ 2, resol });
			}
			menu.items.push_back({ 3, showFps });
		}

		void changeConfig() {
			saveConfig(newConfig);
			reloadMenu();
		}

		void submit() {
 			switch (menu.curItemId)
			{
			case 1:
				newConfig.fulllscreen = !newConfig.fulllscreen;
				changeConfig();
				break;
			case 2:
				resolutionIdx = (resolutionIdx+1) % resolutions.size();
				newConfig.screenW = resolutions[resolutionIdx].w;
				newConfig.screenH = resolutions[resolutionIdx].h;
				changeConfig();
				break;
			case 3:
				newConfig.showFps = !newConfig.showFps;
				changeConfig();
				break;
		    }
		}

		void processKeys() {
			if (firstFrame) {
				firstFrame = false;
				return;
			}
			if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
				submit();				
			}
			menu.processKeys();
		}

		void render() {
			resources->drawCentered("Options", {
				0, config->screenH * 0.05f,
				(float)config->screenW, 0
				}, WHITE);
			resources->drawCentered("Resolution applied after restart", {
				0, config->screenH * 0.90f,
				(float)config->screenW, 0
				}, WHITE);
			menu.render();
		}

	};

}