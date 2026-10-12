#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include "../../common/raylib_cpp.hpp"
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
    vector<string> languages;
    int languageIdx = -1;
    bool firstFrame = true;
    bool exit = false;

    OptionsScreen(World& world) :
        world(world),
        resources(*world.resources),
        config(resources.config),
        verticalMenu(resources.texts.mainFont, {0, 0, (float)config.screenW, (float)config.screenH}, 10)
    {
    }

    ~OptionsScreen() {}

    void reload() {
        newConfig = config;
        firstFrame = true;
        resolutionIdx = -1;

        if (newConfig.language.empty()) {
            newConfig.language = Texts::defaultLanguage;
        }
        languages = Texts::getLanguageList();
        languageIdx = -1;
        for (int i = 0; i < (int)languages.size(); i++) {
            if (languages[i] == newConfig.language) {
                languageIdx = i;
                break;
            }
        }
        if (languageIdx == -1) {
            languages.push_back(newConfig.language);
            languageIdx = (int)languages.size() - 1;
        }

        for (int i = 0; i < resolutions.size(); i++) {
            if (config.screenW == resolutions[i].w && config.screenH == resolutions[i].h) {
                resolutionIdx = i;
                break;
            }
        }

        verticalMenu.bounds = { 0, 0, (float)config.screenW, (float)config.screenH };
        reloadMenu();
    }

    // Пересобирает список пунктов меню. Порядок добавления задаёт порядок
    // отображения; id пункта хранит сам виджет, поэтому submit не зависит
    // от позиции элемента.
    void reloadMenu() {
        vector<VerticalMenuWidget::Item> items;
        items.push_back({ OptLanguage,   _T("options.language") + " " + newConfig.language });
        items.push_back({ OptFullscreen, _T("options.fullscreen") + " " + yesNo(newConfig.fulllscreen) });

        if (!newConfig.fulllscreen) {
            items.push_back({ OptResolution,
                _T("options.resolution") + " " + to_string(newConfig.screenW) + "x" + to_string(newConfig.screenH) });
        }

        items.push_back({ OptScale,   _T("options.scale") + " " + scaleLabel() });
        items.push_back({ OptShowFps, _T("options.showFps") + " " + yesNo(newConfig.showFps) });

        verticalMenu.setItemsExt(items);
        verticalMenu.active = true;
    }

    void changeConfig() {
        saveConfig(newConfig);
        reloadMenu();
    }

    void submit() {
        switch (static_cast<OptionId>(verticalMenu.getSelectedId())) {
            case OptFullscreen:
                newConfig.fulllscreen = !newConfig.fulllscreen;
                changeConfig();
                break;
            case OptResolution:
                resolutionIdx = (resolutionIdx + 1) % resolutions.size();
                newConfig.screenW = resolutions[resolutionIdx].w;
                newConfig.screenH = resolutions[resolutionIdx].h;
                changeConfig();
                break;
            case OptShowFps:
                newConfig.showFps = !newConfig.showFps;
                changeConfig();
                break;
            case OptScale:
                newConfig.antialiasing += 0.25f;
                if (newConfig.antialiasing > 2.0f) newConfig.antialiasing = 1.0f;
                changeConfig();
                break;
            case OptLanguage:
                if (!languages.empty()) {
                    languageIdx = (languageIdx + 1) % (int)languages.size();
                    newConfig.language = languages[languageIdx];
                    applyLanguage();
                    changeConfig();
                }
                break;
            default:
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
   resources.texts.drawCentered(_T("options.title").c_str(), {
    	0, config.screenH * 0.05f,
    	(float)config.screenW, 0
   }, WHITE);
   resources.texts.drawCentered(_T("options.restartHint").c_str(), {
    	0, config.screenH * 0.90f,
    	(float)config.screenW, 0
   }, WHITE);
   verticalMenu.draw();
    }

private:
    // Идентификаторы пунктов меню. Значения не обязаны совпадать с порядком
    // вывода: порядок задаётся в reloadMenu.
    enum OptionId {
        OptFullscreen = 1,
        OptResolution,
        OptShowFps,
        OptScale,
        OptLanguage,
    };

    string _T(const string& key) {
        return resources.texts.getEngineText(key);
    }

    string yesNo(bool value) {
        return value ? _T("options.yes") : _T("options.no");
    }

    // Значение масштаба с двумя знаками после точки без ostringstream.
    string scaleLabel() const {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.2f", newConfig.antialiasing);
        return buffer;
    }

    // Применяет выбранный язык в рантайме: обновляет config.language и
    // перезагружает строки/шрифт (Texts::reload()).
    void applyLanguage() {
        config.language = newConfig.language;
        resources.texts.load();
    }
};

} // namespace openAITD