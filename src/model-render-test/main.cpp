#pragma once
#include <vector>
#include <string>
#include "../raylib-cpp/raylib-cpp.h"

using namespace std;
using namespace raylib;
namespace openAITD {

    bool process(float timeDelta) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            return false;
        }        
        return true;
    }

    void render() {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawFPS(10, 10);
        EndDrawing();
    }

    int main(void)
    {
        //resources.config = loadConfig();
        InitWindow(1024, 768, "Model-Render");
        int m = GetCurrentMonitor();
        int targetFps = GetMonitorRefreshRate(m);
        //int targetFps = 60;

        SetWindowState(FLAG_VSYNC_HINT);
        SetTargetFPS(targetFps);
        SetExitKey(KEY_F10);

        //resources.screen.init();

        DisableCursor();

        float timeDelta = 0;        
        while (!WindowShouldClose()) {
            timeDelta = GetFrameTime();
            if (!process(timeDelta)) break;
            render();
        }

        return 0;
    }
}

int main(void)
{
    return openAITD::main();
}