﻿#pragma once
#include <vector>
#include <string>
#include "./resources/resources.h"
#include "./world/world.h"
#include "./controllers/camera_renderer.h"
#include "./controllers/freelook_renderer.h"
#include "./controllers/player_controller.h"
#include "./controllers/physics_controller.h"
#include "./controllers/animation_controller.h"
#include "./controllers/life_controller.h"

using namespace std;
namespace openAITD {

    enum AppState {
        AS_Loading,
        AS_InWorld,
        AS_MainMenu,
        AS_Inventory,
        AS_Book,
        AS_SelectGame
    };
    AppState state;

    int screenW = 1280;
    int screenH = 960;
    
    Resources resources;
    World world;
    CameraRenderer renderer(&resources, &world);
    FreelookRenderer flRenderer(&resources, &world);
    PlayerController playerContr(&resources, &world);
    PhysicsController physContr(&resources, &world);
    AnimationController animContr(&resources, &world);
    LifeController lifeContr(&resources, &world);
    bool freeLook = false;

    int main(void)
    {
        InitWindow(screenW, screenH, "AITD Mask Test");

        resources.stages.resize(8);
        for (int i = 0; i < 8; i++) {
            resources.stages[i].load(string("data/stages/")+to_string(i));
        }

        world.curStageId = 0;
        world.curCameraId = 0;
        world.loadGObjects("data/objects.json");
        world.renderTarget = &world.gobjects[1];
        playerContr.player = &world.gobjects[1];
        //world.loadVars("data/vars.json");

        renderer.screenW = screenW;
        renderer.screenH = screenH;
        renderer.loadCamera(world.curStageId, world.curCameraId);
        DisableCursor();

        float timeDelta = 0;        
        while (!WindowShouldClose()) {
            timeDelta = GetFrameTime();

            if (IsKeyPressed(KEY_O)) {
                freeLook = !freeLook;
            }

            if (!freeLook) {
                playerContr.process(timeDelta);
                physContr.process(timeDelta);
                animContr.process(timeDelta);
                lifeContr.process();
            }

            BeginDrawing();
            if (freeLook) {
                flRenderer.process();
            }
            else {
                renderer.process();
            }
            EndDrawing();
        }

        return 0;
    }
}

int main(void)
{
    return openAITD::main();
}