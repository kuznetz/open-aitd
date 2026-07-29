#pragma once

#include <vector>
#include <string>
#include <memory>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../../raylib-cpp/raylib-cpp.h"
#include "./widgets/vertical_menu.hpp"

using namespace std;
using namespace raylib;

namespace openAITD {

class InventoryScreen {
public:
    World* world;
    Resources* resources;    
    bool exitting = false;
    bool exit = false;

    std::unique_ptr<VerticalMenuWidget> itemsMenu;
    std::unique_ptr<VerticalMenuWidget> actionsMenu;

    bool selAction = false;
    vector<int> curActions;

    Camera3D modelCamera = {
        { 0, 2.5f, -5.f },
        { 0, 0, 0 },
        { 0, 1, 0 },
        50.0f,
        CAMERA_PERSPECTIVE
    };
    float modelRotate = 0;
    bool firstFrame = true;

    // ---------- animation ----------
    static constexpr float animSpeed = 4.0f;   // 0.25 sec to fully appear
    bool starting = false;
    float animProgress = 0.0f;
    float targetItemsY = 0.0f;
    float targetActionsY = 0.0f;
    float targetModelY = 0.0f;
    // -------------------------------

    InventoryScreen(World* world)
        : world(world)
        , resources(world->resources)
    {
        float W = static_cast<float>(resources->config.screenW);
        float H = static_cast<float>(resources->config.screenH);

        itemsMenu = std::make_unique<VerticalMenuWidget>(
            resources->screen.mainFont,
            raylib::Rectangle{ 0, 0.05f * H, W, 0.45f * H },
            5
        );

        actionsMenu = std::make_unique<VerticalMenuWidget>(
            resources->screen.mainFont,
            raylib::Rectangle{ W / 2, H / 2, W / 2, H / 2 },
            5
        );

        reload();   // will also start the appear animation
    }

    ~InventoryScreen() = default;

    void reload() {
        vector<string> itemNames;
        itemNames.reserve(world->inventory.size());
        for (auto& gobjPtr : world->inventory) {
            if (gobjPtr) {
                int nameId = gobjPtr->invItem.nameId;
                itemNames.push_back(resources->texts[nameId]);
            }
        }
        itemsMenu->setItems(itemNames);
        selAction = false;
        itemsMenu->active = true;
        actionsMenu->active = false;

        exitting = false;
        exit = false;
        firstFrame = true;

        float W = static_cast<float>(resources->config.screenW);
        float H = static_cast<float>(resources->config.screenH);

        // remember final positions
        targetItemsY   = 0.05f * H;
        targetActionsY = H / 2.0f;
        targetModelY   = 0;

        // set initial animated positions
        itemsMenu->bounds   = raylib::Rectangle{ 0, -0.45f * H, W, 0.45f * H };   // completely above screen
        actionsMenu->bounds = raylib::Rectangle{ W / 2, H, W / 2, H / 2 };        // completely below

        // start the appear animation
        animProgress = 0.0f;
        starting = true;

        reloadActions();
    }

    void reloadActions() {
        curActions.clear();
        int idx = itemsMenu->getSelectedIndex();
        if (idx >= 0 && idx < static_cast<int>(world->inventory.size())) {
            auto& gobj = *world->inventory[idx];
            for (int i = 0; i < 11; ++i) {
                if (gobj.invItem.flags & (1 << i)) {
                    curActions.push_back(i + 23);
                }
            }
        }

        vector<string> actionNames;
        actionNames.reserve(curActions.size());
        for (int actionId : curActions) {
            actionNames.push_back(resources->texts[actionId]);
        }
        actionsMenu->setItems(actionNames);
    }

    void processKeys() {
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (selAction) {
                selAction = false;
                itemsMenu->active = true;
                actionsMenu->active = false;
            } else {
                exitting = true;
            }
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            if (!selAction) {
                selAction = true;
                itemsMenu->active = false;
                actionsMenu->active = true;
                reloadActions();
            } else {
                submit();
                exitting = true;
            }
        }

        if (IsKeyPressed(KEY_UP)) {
            if (!selAction) {
                itemsMenu->moveUp();
                reloadActions();
            } else {
                actionsMenu->moveUp();
            }
        }

        if (IsKeyPressed(KEY_DOWN)) {
            if (!selAction) {
                itemsMenu->moveDown();
                reloadActions();
            } else {
                actionsMenu->moveDown();
            }
        }
    }

    void submit() {
        int itemIdx = itemsMenu->getSelectedIndex();
        int actionIdx = actionsMenu->getSelectedIndex();
        if (itemIdx < 0 || itemIdx >= static_cast<int>(world->inventory.size())) return;
        if (actionIdx < 0 || actionIdx >= static_cast<int>(curActions.size())) return;

        world->curInvGObject = world->inventory[itemIdx];
        world->curInvAction = 1 << (curActions[actionIdx] - 23);
    }

    void render() {
        drawLines();
        itemsMenu->draw();
        actionsMenu->draw();
        drawModel();
        resources->screen.resetViewport();
        drawVariable();
    }

    void process(float timeDelta) {
        // animation update (runs even on the very first frame)
        if (starting) {
            animProgress += timeDelta * animSpeed;
            if (animProgress > 1.0f) {
                animProgress = 1.0f;
                starting = false;
            }
        }

        if (exitting) {
            animProgress -= timeDelta * animSpeed * 2;
            if (animProgress < 0.0f) {
                animProgress = 0.0f;
                exit = true;
            }
        }

        if (starting || exitting) {
            float H = static_cast<float>(resources->config.screenH);
            // lerp helper
            auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };
            itemsMenu->bounds.y   = lerp(-0.45f * H, targetItemsY, animProgress);
            actionsMenu->bounds.y = lerp(H, targetActionsY, animProgress);
        }

        // skip rotation on the very first frame (keeps original behaviour)
        if (!firstFrame) {
            modelRotate += timeDelta * 180.f;
        }
        firstFrame = false;

        processKeys();
        itemsMenu->process(timeDelta);
        actionsMenu->process(timeDelta);
    }

    void drawLines() {
        auto& w = resources->config.screenW;
        auto& h = resources->config.screenH;

        const raylib::Color clr = GRAY;

        raylib::Vector2 L1From { 0, (float)(h / 2) };
        raylib::Vector2 L2From { w, (float)(h / 2) };
        raylib::Vector2 L3From { (float)(w / 2), (float)h };
        
        raylib::Vector2 L1To { (float)(w / 2), (float)(h / 2) };
        raylib::Vector2 L2To { (float)(w / 2), (float)(h / 2) };
        raylib::Vector2 L3To { (float)(w / 2), (float)(h / 2) };

        L1To = Vector2Lerp(L1From, L1To, animProgress);
        L2To = Vector2Lerp(L2From, L2To, animProgress);
        L3To = Vector2Lerp(L3From, L3To, animProgress);

        DrawLineEx(L1From, L1To, 2, clr);
        DrawLineEx(L2From, L2To, 2, clr);
        DrawLineEx(L3From, L3To, 2, clr);
    }

    void drawModel() {
        if (world->inventory.empty()) return;
        int idx = itemsMenu->getSelectedIndex();
        if (idx < 0 || idx >= static_cast<int>(world->inventory.size())) return;

        GameObject& gobj = *world->inventory[idx];
        RModel* rmodel = resources->models.getModel(gobj.invItem.modelId);
        if (!rmodel) return;

        auto& c = resources->config;
        // calculate animated Y for the model viewport (slides up from bottom)
        auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };
        float startY = static_cast<float>(-c.screenH / 2.f);   // below screen
        float currentModelY = lerp(startY, targetModelY, animProgress);

        BeginMode3D(modelCamera);
        rlViewport(c.screenX, static_cast<int>(currentModelY), c.screenW / 2, c.screenH / 2);
        float aspect = static_cast<float>(c.screenW) / c.screenH;

        Matrix proj = MatrixPerspective(modelCamera.fovy * DEG2RAD, aspect, 0.01f, 100.0);
        rlMatrixMode(RL_PROJECTION);
        rlSetMatrixProjection(proj);

        rlMatrixMode(RL_MODELVIEW);
        Matrix view = MatrixLookAt(modelCamera.position, modelCamera.target, modelCamera.up);
        rlSetMatrixModelview(view);
        rlRotatef(modelRotate, 0, 1, 0);

        rmodel->model.Render();
        EndMode3D();
    }

    void drawVariable() {
        if (animProgress < 1.0f) return;
        int idx = itemsMenu->getSelectedIndex();
        if (idx < 0 || idx >= static_cast<int>(world->inventory.size())) return;
        auto& gobj = *world->inventory[idx];
        if (gobj.stageLifeId == -1) return;

        string valStr = std::to_string(world->vars[gobj.stageLifeId]);
        raylib::Rectangle r = {
            resources->config.screenW * 0.01f,
            (float)(resources->config.screenH / 2),
            (float)(resources->config.screenW / 2),
            (float)resources->config.screenH
        };
        resources->screen.drawLeft(valStr.c_str(), r, raylib::WHITE);
    }
};

} // namespace openAITD