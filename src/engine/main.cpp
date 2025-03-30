#pragma once
#include <vector>
#include <string>
#include "./resources/resources.h"
#include "./world/world.h"
#include "./controllers/camera_renderer.h"

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
        //world.loadVars("data/vars.json");

        renderer.screenW = screenW;
        renderer.screenH = screenH;
        renderer.loadCamera(world.curStageId, world.curCameraId);
        DisableCursor();

        while (!WindowShouldClose()) {

            if (IsKeyPressed(KEY_RIGHT)) {
                if (world.curCameraId++);
                renderer.loadCamera(world.curStageId, world.curCameraId);
            }
            if (IsKeyPressed(KEY_LEFT)) {
                if (world.curCameraId--);
                renderer.loadCamera(world.curStageId, world.curCameraId);
            }
            if (IsKeyPressed(KEY_UP)) {
                world.curStageId++;
                world.curCameraId = 0;
                renderer.loadCamera(world.curStageId, world.curCameraId);
            }
            if (IsKeyPressed(KEY_DOWN)) {
                world.curStageId--;
                world.curCameraId = 0;
                renderer.loadCamera(world.curStageId, world.curCameraId);
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