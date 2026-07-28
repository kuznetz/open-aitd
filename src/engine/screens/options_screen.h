#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include "../../raylib-cpp/raylib-cpp.h"
#include "../world/world.h"
#include "../resources/resources.h"
#include "./widgets/vertical_menu.hpp"

using namespace std;
using namespace raylib;

namespace openAITD {

struct ScreenRes {
    int w, h;
};
vector<ScreenRes> resolutions = { {320,240}, {640,480}, {960,720}, {1280,960}, {1600,1200} };

class OptionsScreen {
public:
    World& world;
    Resources& resources;
    Config& config;
    Config newConfig;

    VerticalMenuWidget verticalMenu;

    int resolutionIdx = -1;
    bool firstFrame = true;
    bool exit = false;

    OptionsScreen(World& world) :
        world(world),
        resources(*world.resources),
        config(resources.config),
        verticalMenu(resources.screen.mainFont, {0, 0, (float)config.screenW, (float)config.screenH}, 10)
    {
    }

    ~OptionsScreen() {}

    void reload() {
        newConfig = config;
        firstFrame = true;
        resolutionIdx = -1;
        for (int i = 0; i < resolutions.size(); i++) {
            if (config.screenW == resolutions[i].w && config.screenH == resolutions[i].h) {
                resolutionIdx = i;
                break;
            }
        }

        verticalMenu.bounds = { 0, 0, (float)config.screenW, (float)config.screenH };
        reloadMenu();
    }

    void reloadMenu() {
        vector<pair<int, string>> menuItems;
        std::ostringstream sstream;

        string fullscreen = string("Fullscreen: ") + (newConfig.fulllscreen ? "yes" : "no");
        menuItems.push_back({1, fullscreen});

        if (!newConfig.fulllscreen) {
            string resol = string("Resolution: ") + to_string(newConfig.screenW) + "x" + to_string(newConfig.screenH);
            menuItems.push_back({2, resol});
        }

        sstream.str("");
        sstream << std::fixed << std::setprecision(2) << newConfig.antialiasing;
        string antialiasing = string("Antialiasing: ") + sstream.str();
        menuItems.push_back({4, antialiasing});

        string showFps = string("Show fps: ") + (newConfig.showFps ? "yes" : "no");
        menuItems.push_back({3, showFps});

        this->menuItems = menuItems;

        vector<string> labels;
        for (const auto& p : menuItems) {
            labels.push_back(p.second);
        }
        verticalMenu.setItems(labels);
        verticalMenu.active = true;
    }

    void changeConfig() {
        saveConfig(newConfig);
        reloadMenu();
    }

    void submit() {
        if (menuItems.empty()) return;
        int idx = verticalMenu.getSelectedIndex();
        if (idx < 0 || idx >= (int)menuItems.size()) return;

        int id = menuItems[idx].first;
        switch (id) {
            case 1: // Fullscreen
                newConfig.fulllscreen = !newConfig.fulllscreen;
                changeConfig();
                break;
            case 2: // Resolution
                resolutionIdx = (resolutionIdx + 1) % resolutions.size();
                newConfig.screenW = resolutions[resolutionIdx].w;
                newConfig.screenH = resolutions[resolutionIdx].h;
                changeConfig();
                break;
            case 3: // Show FPS
                newConfig.showFps = !newConfig.showFps;
                changeConfig();
                break;
            case 4: // Antialiasing
                newConfig.antialiasing += 0.25f;
                if (newConfig.antialiasing > 2.0f) newConfig.antialiasing = 1.0f;
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
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_KP_8)) {
            verticalMenu.moveUp();
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_KP_2)) {
            verticalMenu.moveDown();
        }
    }

		void process(float timeDelta) {
			verticalMenu.process(timeDelta);
		}

    void render() {
			resources.screen.drawCentered("Options", {
					0, config.screenH * 0.05f,
					(float)config.screenW, 0
			}, WHITE);
			resources.screen.drawCentered("Resolution applied after restart", {
					0, config.screenH * 0.90f,
					(float)config.screenW, 0
			}, WHITE);
			verticalMenu.draw();
    }

private:
    vector<pair<int, string>> menuItems;
};

} // namespace openAITD