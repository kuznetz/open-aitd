#pragma once
#include <vector>
#include <string>
#include "./resources/resources.h"
#include "./world/world.h"
#include "./controllers/camera_renderer.h"
#include "./controllers/player_controller.h"

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
    PlayerController playerContr(&resources, &world);

    int main(void)
    {
        InitWindow(screenW, screenH, "AITD Mask Test");

        resources.stages.resize(8);
        for (int i = 0; i < 8; i++) {
            resources.stages[i].load(string("data/stages/")+to_string(i));
        }

        world.curStageId = 1;
        world.curCameraId = 3;
        world.loadGObjects("data/objects.json");
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
                renderer.flyMode = !renderer.flyMode;
            }
            if (!renderer.flyMode) {
                playerContr.process(timeDelta);

                if (IsKeyPressed(KEY_D) && (world.curCameraId < renderer.curStage->cameras.size() - 1)) {
                    if (world.curCameraId++);
                    renderer.loadCamera(world.curStageId, world.curCameraId);
                }
                if (IsKeyPressed(KEY_A) && (world.curCameraId > 0)) {
                    if (world.curCameraId--);
                    renderer.loadCamera(world.curStageId, world.curCameraId);
                }
                if (IsKeyPressed(KEY_W)) {
                    world.curStageId++;
                    world.curCameraId = 0;
                    renderer.loadCamera(world.curStageId, world.curCameraId);
                }
                if (IsKeyPressed(KEY_S)) {
                    world.curStageId--;
                    world.curCameraId = 0;
                    renderer.loadCamera(world.curStageId, world.curCameraId);
                }
            }
            else
            {
                if (IsKeyPressed(KEY_LEFT_BRACKET)) {
                    renderer.invX = !renderer.invX;
                }
                if (IsKeyPressed(KEY_RIGHT_BRACKET)) {
                    renderer.invZ = !renderer.invZ;
                }
            }

            BeginDrawing();
            renderer.process();
            EndDrawing();
        }

        return 0;
    }
}

int main(void)
{
    return openAITD::main();
}