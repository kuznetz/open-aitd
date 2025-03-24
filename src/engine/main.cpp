#pragma once
#include <vector>
#include <string>
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
    int screenH = 800;
    World world;
    CameraRenderer renderer;

    int main(void)
    {
        world.loadGObjects("data/objects.json");
        world.loadVars("data/vars.json");
        return 0;
    }

}