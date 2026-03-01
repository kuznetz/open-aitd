#pragma once
#include <vector>
#include <string>
#include <sstream>
#include "../raylib-cpp/raylib-cpp.h"
#include "../model-render/model_instance.hpp"

namespace openAITD {
    using namespace ::std;
    using namespace ::raylib;

    int modelCount = 100;
    int modelIdx = 11;
    int animIdx = 0;
    bool paused = false;
    bool showGrid = true;
    bool showText = true;

    Model* model;
    ModelInstance* modelInst;
    Camera3D camera;

    void loadModelByIndex(int idx) {
        if (modelInst) {
            delete modelInst;
            delete model;
        }
        
        model = new Model();
        modelInst = new ModelInstance();
        std::ostringstream oss;
        oss << "./data/models/" << idx << "/model.gltf";
        model->Load(oss.str().c_str());
        modelInst->Init(*model);
        animIdx = 0;
    }

    void init() {
        camera.position = Vector3{ 0.0f, 2.0f, 6.0f };
        camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
        camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
        camera.fovy = 45.0f;
        camera.projection = CAMERA_PERSPECTIVE;

        //resources.config = loadConfig();
        InitWindow(1024, 768, "Model-Render");
        int m = GetCurrentMonitor();
        int targetFps = GetMonitorRefreshRate(m);
        //int targetFps = 60;

        SetWindowState(FLAG_VSYNC_HINT);
        SetTargetFPS(targetFps);
        SetExitKey(KEY_F10);
        DisableCursor();

        loadModelByIndex(modelIdx);
    }

    bool process(float timeDelta) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            return false;
        }
        if (IsKeyPressed(KEY_LEFT)) {
            modelIdx = (modelIdx - 1 + modelCount) % modelCount;
            loadModelByIndex(modelIdx);
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            modelIdx = (modelIdx + 1) % modelCount;
            loadModelByIndex(modelIdx);
        }
        if (IsKeyPressed(KEY_UP)) {
            animIdx = (animIdx + 1) % model->animations.size();
            modelInst->SetAnimation(animIdx);
        }
        if (IsKeyPressed(KEY_DOWN)) {
            animIdx = (animIdx - 1 + model->animations.size()) % model->animations.size();
            modelInst->SetAnimation(animIdx);
        }
        if (IsKeyPressed(KEY_SPACE)) {
            paused = !paused;
        }
        if (IsKeyPressed(KEY_G)) {
            showGrid = !showGrid;
        }
       if (IsKeyPressed(KEY_H)) {
            showText = !showText;
        }
        
        if (!paused) {
            modelInst->Process(timeDelta);
        }
        UpdateCamera(&camera, CAMERA_FIRST_PERSON);
        return true;
    }

    void render() {
        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(camera);
            if (showGrid) {
                DrawGrid(40, 0.2f); 
            }
            modelInst->Render();
        EndMode3D();

        if (showText) {
            DrawFPS(10, 10);
            std::ostringstream oss;
            auto animCount = model->animations.size()-1;
            oss << "Mdl " << modelIdx << " Anim " << animIdx << "/" << animCount;
            DrawText(oss.str().c_str(), 10, 30, 20, WHITE);
        }

        EndDrawing();
    }

    int main(void)
    {
        init();

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