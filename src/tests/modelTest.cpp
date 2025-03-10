#include <raylib.h>
#include <rlgl.h>
#include <string.h>
#include <string>
#include <iostream>
#include "extractor/background_extractor.h"
#include "extractor/floor_extractor.h"
#include <renderer.h>

namespace ModelTest {

    int screenW = 1280;
    int screenH = 960;

    int curFloorId = 0;
    int curRoomId = 0;
    int curCameraId = 0;
    floorStruct* curFloor = 0;
    cameraStruct* curCamera = 0;
    Texture2D backgroundTex;
    Texture2D maskTex;

    Vector3 testPos = { 0, 0, 0 };

    bool renderLayers[10] = { true, true, false, true, true, true, true, true, true, true };

    const Camera3D testCamera = {
        { 0.0f, 3.0f, 5.0f }, // mainCamera position
        { 0.0f, 0.0f, 0.0f },   // mainCamera looking at point
        { 0.0f, 1.0f, 0.0f },   // mainCamera up vector (rotation towards target)
        60.0f,
        CAMERA_PERSPECTIVE
    };

    void drawDebugText() {
        char text[100] = "";
        //sprintf((char*)text, "FLOOR:%d CAMERA:%d", curFloorId, curCameraId);
        //const Vector2 text_size = MeasureTextEx(GetFontDefault(), (char*)text, 20, 1);
        //DrawText((char*)text, screenW / 2. - text_size.x / 2, screenH - (text_size.y + 10), 20, BLACK);
        DrawFPS(5, 5);
    }

    void runTest()
    {
        InitWindow(screenW, screenH, "oAITD Model Test");
        SetTargetFPS(60);

        while (!WindowShouldClose())
        {
            float frameTime = GetFrameTime();

            if (IsKeyDown(KEY_RIGHT)) {
                testPos.x += frameTime * 2;
            }
            if (IsKeyDown(KEY_LEFT)) {
                testPos.x -= frameTime * 2;
            }
            if (IsKeyDown(KEY_UP)) {
                testPos.z += frameTime * 2;
            }
            if (IsKeyDown(KEY_DOWN)) {
                testPos.z -= frameTime * 2;
            }
            for (int i = 0; i < 10; i++) {
                if (IsKeyPressed(KEY_ZERO + i)) {
                    renderLayers[i] = !renderLayers[i];
                }
            }

            BeginDrawing();
            ClearBackground(DARKBLUE);

            BeginMode3D(testCamera);
            //DrawCube(testPos, 2, 0.5f, 0.5f, BLUE);
            DrawPlane(testPos, { 10, 10 }, DARKGRAY);
            EndMode3D();

            if (renderLayers[0]) {
                drawDebugText();
            }

            
            EndDrawing();
        }

        CloseWindow();
    }

}