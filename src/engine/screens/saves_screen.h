#pragma once
#include <vector>
#include <string>
#include <algorithm>   // для std::min
#include "../../common/raylib_cpp.hpp"
#include "../world/world.h"
#include "../resources/resources.h"
#include "./options_screen.h"
#include "./widgets/vertical_menu.hpp"
#include "./controllers/save_controller.h"

using namespace std;
using namespace raylib;

namespace openAITD {
class SavesScreen {
public:
    enum class Mode { Save, Load };

public:
    SavesScreen(Resources& resources, SaveController& saveContr)
        : resources(resources),
          saveContr(saveContr),
          menu(resources.screen.mainFont, raylib::Rectangle{0, 0, 1, 1}, 10)
    {}

    ~SavesScreen() {
        if (screenshotTexture.id != 0) {
            UnloadTexture(screenshotTexture);
            screenshotTexture = {0};
        }
    }

    void reload(Mode newMode) {
        mode = newMode;
        complete = false;
        selectedSlot = -1;
        curScreenshotIdx = -1;

        //Menu position
        menu.bounds = {
            0,
            resources.config.screenH * 0.1f,
            (float)resources.config.screenW / 2.f,
            (float)resources.config.screenH
        };

        // FillSlots
        saveSlots = saveContr.listSlots();
        vector<string> items;
        if (mode == Mode::Save) {
            items.push_back("<New save>");   // только для Save
        }
        for (const auto& slot : saveSlots) {
            items.push_back(slot.location);
        }
        menu.setItems(items);
        menu.setSelectedIndex(0);
        updateScreenshot(menu.getSelectedIndex());
    }

    void processKeys() {
        bool changed = false;
        if (IsKeyPressed(KEY_UP)) { menu.moveUp(); changed = true; }
        else if (IsKeyPressed(KEY_DOWN)) { menu.moveDown(); changed = true; }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            int menuIdx = menu.getSelectedIndex();
            int slotIdx = getSlotIdx(menuIdx);
            if (slotIdx < 0) {
                //New save
                int maxId = -1;
                for (const auto& slot : saveSlots) {
                    if (slot.id > maxId) maxId = slot.id;
                }
                selectedSlot = maxId + 1;
                complete = true;
            } else {
                selectedSlot = saveSlots[slotIdx].id;
                complete = true;
            }
        }
        if (changed) {
            updateScreenshot(menu.getSelectedIndex());
        }
    }

    void process(float timeDelta) {
        menu.process(timeDelta);
    }

    void render() {
        const char* title = (mode == Mode::Save) ? "Save Game" : "Load Game";
        resources.screen.drawCentered(title, {
            0, resources.config.screenH * 0.05f,
            (float)resources.config.screenW, 0
        }, WHITE);
        menu.draw();

        // Right half – screenshot preview
        float scrshotWidth = (float)resources.config.screenW * 0.48f;
        float scrshotHeight = scrshotWidth / 4. * 3.;
        raylib::Rectangle rightRect = {
            (float)resources.config.screenW * 0.51f,
            resources.config.screenH * 0.3f,
            scrshotWidth,
            scrshotHeight
        };
        if (hasScreenshot && screenshotTexture.id != 0) {
            float texW = (float)screenshotTexture.width;
            float texH = (float)screenshotTexture.height;
            float rectW = rightRect.width;
            float rectH = rightRect.height;
            float scale = std::min(rectW / texW, rectH / texH);
            float drawW = texW * scale;
            float drawH = texH * scale;
            float drawX = rightRect.x + (rectW - drawW) * 0.5f;
            float drawY = rightRect.y + (rectH - drawH) * 0.5f;
            DrawTexture(screenshotTexture, drawX, drawY, WHITE);
        } else {
            // Placeholder
            DrawRectangleRec(rightRect, DARKGRAY);
        }
    }

    bool isComplete() const { return complete; }
    int getSelectedSlot() const { return selectedSlot; }
    void resetComplete() { complete = false; }

private:
    void updateScreenshot(int menuIndex) {
        int slotIdx = getSlotIdx(menuIndex);
        if (slotIdx == curScreenshotIdx) return;

        // Unload Old
        if (screenshotTexture.id != 0) {
            UnloadTexture(screenshotTexture);
            screenshotTexture = {0};
            hasScreenshot = false;
        }

        if (slotIdx < 0) return;
        const auto& slot = saveSlots[slotIdx];
        
        std::string path = saveContr.saveDir + "/" + to_string(slot.id) + "/screen.png";
        if (FileExists(path.c_str())) {
            Image img = raylib::LoadImage(path.c_str());
            if (img.data != nullptr) {
                screenshotTexture = LoadTextureFromImage(img);
                hasScreenshot = true;
                UnloadImage(img);
            }
        }
    }

private:
    Resources& resources;
    SaveController& saveContr;
    std::vector<SaveSlot> saveSlots;
    VerticalMenuWidget menu;
    Mode mode = Mode::Save;
    bool complete = false;
    int selectedSlot = -1;

    // Screenshot preview
    ::Texture2D screenshotTexture = {0};
    bool hasScreenshot = false;
    int curScreenshotIdx = -1;

    int getSlotIdx(int menuIdx) {
        int slotIdx;
        if (mode == Mode::Save) {
            if (menuIdx == 0) return -1; // "<New save>"
            slotIdx = menuIdx - 1;
        } else { // Load
            slotIdx = menuIdx;
        }
       return slotIdx;
    }
};

} // namespace openAITD