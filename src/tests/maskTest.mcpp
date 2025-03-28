#include <raylib.h>
#include <rlgl.h>
#include <string.h>
#include <string>
#include <iostream>
#include "extractor/extractor.h"
#include <renderer.h>
#include "../mask_render.h"

namespace MaskTest {

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
    }

    void drawColliders() {

    }

    void set3DCamera() {

    }

    void runTest()
    {


        InitWindow(screenW, screenH, "AITD Mask Test");
        SetTargetFPS(60);

        char str[100];
        sprintf(str, "data/floor_%02d/camera_%02d/background.png", 0, 0);
        Image image = LoadImage(str);
        backgroundTex = LoadTextureFromImage(image);

        Image image2 = LoadImage("data/floor_00/camera_00/mask_00_00.png");
        //Image image2 = GenImageChecked(100, 100, 10, 10, DARKGREEN, RAYWHITE);
        maskTex = LoadTextureFromImage(image2);

        RenderTexture2D screenTex = LoadRenderTexture(screenW, screenH);
        
        BeginTextureMode(screenTex);
        SetTextureFilter(backgroundTex, TEXTURE_FILTER_BILINEAR);
        DrawTexturePro(
            backgroundTex,
            { 0, 0, (float)backgroundTex.width, (float)backgroundTex.height },
            { 0, 0, (float)screenW, (float)screenH },
            { 0, 0 }, 0, WHITE
        );
        EndTextureMode();
        Image bgImageScaled = LoadImageFromTexture(screenTex.texture);
        ImageFlipVertical(&bgImageScaled);
        Texture2D backgroundTex2 = LoadTextureFromImage(bgImageScaled);

        BeginTextureMode(screenTex);
        ClearBackground(BLACK);
        SetTextureFilter(maskTex, TEXTURE_FILTER_BILINEAR);
        DrawTexturePro(
            maskTex,
            { 0, 0, (float)maskTex.width, (float)maskTex.height },
            { 0, 0, (float)screenW, (float)screenH },
            { 0, 0 }, 0, WHITE
        );
        EndTextureMode();
        Image maskImageScaled = LoadImageFromTexture(screenTex.texture);
        ImageFlipVertical(&maskImageScaled);
        Texture2D maskTex2 = LoadTextureFromImage(maskImageScaled);

        for (int i = 0; i < (screenW * screenH); i++) {
            ((u8*)bgImageScaled.data)[i * 4 + 3] = ((u8*)maskImageScaled.data)[i * 4 + 0];
        }
        Texture2D maskTex3 = LoadTextureFromImage(bgImageScaled);

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
            ClearBackground(DARKGRAY);

            if (renderLayers[1]) {
                //Original game rendered on 320x200
                //But displayed on 4/3 monitor, with non squared pixels
                DrawTexturePro(
                    backgroundTex2,
                    { 0, 0, (float)screenW, (float)screenH },
                    { 0, 0, (float)screenW, (float)screenH },
                    { 0, 0 }, 0, WHITE
                );
            }

            BeginMode3D(testCamera);
            DrawCube({ -1.06667542, 0.00000000, -4.83492994 }, 2, 0.5f, 0.5f, RED);
            EndMode3D();

            if (renderLayers[2]) {
                DrawTexturePro(
                    maskTex2,
                    { 0, 0, (float)screenW, (float)screenH },
                    { 0, 0, (float)screenW, (float)screenH },
                    { 0, 0 }, 0, WHITE
                );
            }

            if (renderLayers[3]) {
                DrawTexturePro(
                    maskTex3,
                    { 0, 0, (float)screenW, (float)screenH },
                    { 0, 0, (float)screenW, (float)screenH },
                    { 0, 0 }, 0, WHITE
                );
            }
            
            BeginMode3D(testCamera);
            DrawCube(testPos, 2, 0.5f, 0.5f, BLUE);
            EndMode3D();

            if (renderLayers[0]) {
                drawDebugText();
            }

            DrawFPS(5, 5);
            EndDrawing();
        }

        CloseWindow();
    }

}