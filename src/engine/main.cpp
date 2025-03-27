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

        resources.stages.resize(2);
        resources.stages[0].load("data/stages/0");
        resources.stages[1].load("data/stages/1");

        world.curStageId = 0;
        world.curCameraId = 0;
        world.loadGObjects("data/objects.json");
        //world.loadVars("data/vars.json");

        renderer.screenW = screenW;
        renderer.screenH = screenH;
        renderer.loadCamera(0, 0);

        while (!WindowShouldClose()) {
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