#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <cmath>
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
    // Indexes of the selected volume level (see volumeOptions()).
    int soundVolumeIdx = 2; // normal
    int musicVolumeIdx = 2;
    int voiceVolumeIdx = 2;
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

        soundVolumeIdx = volumeIndexFromValue(newConfig.soundVolume);
        musicVolumeIdx = volumeIndexFromValue(newConfig.musicVolume);
        voiceVolumeIdx = volumeIndexFromValue(newConfig.voiceVolume);

        verticalMenu.bounds = { 0, 0, (float)config.screenW, (float)config.screenH };
        applyVolumes();
        reloadMenu();
    }

    // Rebuilds the menu item list. The order of insertion determines the display
    // order; the item id is stored by the widget itself, so submit does not depend
    // on the item's position.
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
        items.push_back({ OptSoundVolume, _T("options.soundVolume") + " " + volumeLabel(soundVolumeIdx) });
        items.push_back({ OptMusicVolume, _T("options.musicVolume") + " " + volumeLabel(musicVolumeIdx) });
        items.push_back({ OptVoiceVolume, _T("options.voiceVolume") + " " + volumeLabel(voiceVolumeIdx) });

        verticalMenu.setItemsExt(items);
        verticalMenu.active = true;
    }

    void changeConfig() {
        config.soundVolume = newConfig.soundVolume;
        config.musicVolume = newConfig.musicVolume;
        config.voiceVolume = newConfig.voiceVolume;
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
            case OptSoundVolume:
                soundVolumeIdx = nextVolumeIndex(soundVolumeIdx);
                newConfig.soundVolume = volumeOptions()[soundVolumeIdx].value;
                applyVolumes();
                changeConfig();
                break;
            case OptMusicVolume:
                musicVolumeIdx = nextVolumeIndex(musicVolumeIdx);
                newConfig.musicVolume = volumeOptions()[musicVolumeIdx].value;
                applyVolumes();
                changeConfig();
                break;
            case OptVoiceVolume:
                voiceVolumeIdx = nextVolumeIndex(voiceVolumeIdx);
                newConfig.voiceVolume = volumeOptions()[voiceVolumeIdx].value;
                applyVolumes();
                changeConfig();
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
    // Menu item identifiers. Values do not have to match display order:
    // the order is set in reloadMenu.
    enum OptionId {
        OptFullscreen = 1,
        OptResolution,
        OptShowFps,
        OptScale,
        OptLanguage,
        OptSoundVolume,
        OptMusicVolume,
        OptVoiceVolume,
    };

    string _T(const string& key) {
        return resources.texts.getEngineText(key);
    }

    string yesNo(bool value) {
        return value ? _T("options.yes") : _T("options.no");
    }

    // Scale value with two decimal places, without ostringstream.
    string scaleLabel() const {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.2f", newConfig.antialiasing);
        return buffer;
    }

    // Volume level available in settings: label + value.
    struct VolumeLevel {
        float value;
        const char* textKey;
    };

    // Discrete volume levels: off(0.0), quiet(0.3), normal(0.6), loud(1.0).
    // This is an entity of the options screen only; the config stores only the float value.
    static const vector<VolumeLevel>& volumeOptions() {
        static const vector<VolumeLevel> options = {
            { 0.0f, "options.volumeOff" },
            { 0.3f, "options.volumeQuiet" },
            { 0.6f, "options.volumeNormal" },
            { 1.0f, "options.volumeLoud" },
        };
        return options;
    }

    // Volume level label (off/quiet/normal/loud).
    string volumeLabel(int idx) {
        const auto& options = volumeOptions();
        if (idx < 0 || idx >= (int)options.size()) idx = 0;
        return _T(options[idx].textKey);
    }

    // Index of the closest level for a saved volume value.
    static int volumeIndexFromValue(float value) {
        const auto& options = volumeOptions();
        int best = 0;
        float bestDiff = fabsf(options[0].value - value);
        for (int i = 1; i < (int)options.size(); i++) {
            float diff = fabsf(options[i].value - value);
            if (diff < bestDiff) {
                bestDiff = diff;
                best = i;
            }
        }
        return best;
    }

    // Next volume level, wrapping around.
    static int nextVolumeIndex(int idx) {
        const int count = (int)volumeOptions().size();
        if (idx < 0 || idx >= count) idx = 0;
        return (idx + 1) % count;
    }

    // Applies volume settings from config to the audio subsystem.
    void applyVolumes() {
        resources.audio.SetSoundVolume(newConfig.soundVolume);
        resources.audio.SetMusicVolume(newConfig.musicVolume);
        resources.audio.SetVoiceVolume(newConfig.voiceVolume);
    }

    // Applies the selected language at runtime: updates config.language and
    // reloads strings/font (Texts::reload()).
    void applyLanguage() {
        config.language = newConfig.language;
        resources.texts.load();
    }
};

} // namespace openAITD