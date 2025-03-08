#include <raylib.h>
#include <string.h>
#include <string>
#include <iostream>
#include "extractor/background_extractor.h"
#include "extractor/floor_extractor.h"
#include <renderer.h>

int screenW = 1280;
int screenH = 960;

int curFloorId = 0;
int curRoomId = 0;
int curCameraId = 0;
floorStruct* curFloor = 0;
cameraStruct* curCamera = 0;
Texture2D backgroundTex;

void changeCamera(int floor, int camera) {
    if (!curFloor || curFloorId != floor) {
        if (curFloor) {
            delete curFloor;
        }
        char fname[50];
        sprintf(fname, "original/ETAGE0%d", floor);
        curFloor = loadFloorPak(fname);
        //saveFloorTxt(fname, fs);
    }
    if (!backgroundTex.id || curFloorId != floor || curCameraId != camera) {
        curCamera = &curFloor->cameras[camera];
        char str[100];
        sprintf(str, "backgrounds/%d_%d.png", floor, camera);
        Image image = LoadImage(str);
        backgroundTex = LoadTextureFromImage(image);
    }

    //Image backgroundImage = GenImageChecked(screenW, screenH, 40, 40, ORANGE, YELLOW);
    //Texture2D backgroundTexture = LoadTextureFromImage(backgroundImage);

    curFloorId = floor;
    curRoomId = 0;
    curCameraId = camera;
}

void drawDebugText() {
    char text[100] = "";
    sprintf((char*)text, "FLOOR:%d CAMERA:%d", curFloorId, curCameraId);
    const Vector2 text_size = MeasureTextEx(GetFontDefault(), (char*)text, 20, 1);
    DrawText((char*)text, screenW / 2. - text_size.x / 2, screenH - (text_size.y + 10), 20, BLACK);
}

void runCameraTester()
{
    backgroundTex.id = 0;
    InitWindow(screenW, screenH, "Open AITD");
    SetTargetFPS(60);
    changeCamera(0, 0);

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_RIGHT)) {
            if (curCameraId < curFloor->cameras.size() - 1) changeCamera(curFloorId, curCameraId + 1);
        }
        if (IsKeyPressed(KEY_LEFT)) {
            if (curCameraId > 0) changeCamera(curFloorId, curCameraId - 1);
        }
        if (IsKeyPressed(KEY_UP)) {
            if (curFloorId < 7) changeCamera(curFloorId + 1, 0);
        }
        if (IsKeyPressed(KEY_DOWN)) {
            if (curFloorId > 0) changeCamera(curFloorId - 1, 0);
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);
        //Original game rendered on 320x200
        //But displayed on 4/3 monitor, with non squared pixels
        DrawTexturePro(
            backgroundTex,
            { 0, 0, (float)backgroundTex.width, (float)backgroundTex.height },
            { 0, 0, (float)screenW, (float)screenH},
            { 0, 0 }, 0, WHITE
        );

        for (int i = 0; i < curCamera->viewedRoomTable.size(); i++) {
            auto vw = &curCamera->viewedRoomTable[i];
            for (int i2 = 0; i2 < vw->overlays_V1.size(); i2++) {
                auto mask = &vw->overlays_V1[i2];
                for (int i3 = 0; i3 < mask->polys.size(); i3++) {
                    auto psize = mask->polys[i3].points.size() / 2;
                    for (int i4 = 0; i4 < psize; i4++) {
                        auto x1 = mask->polys[i3].points[i4 * 2 + 0];
                        auto y1 = mask->polys[i3].points[i4 * 2 + 1];
                        auto i5 = (i4 + 1) % psize;
                        auto x2 = mask->polys[i3].points[i5 * 2 + 0];
                        auto y2 = mask->polys[i3].points[i5 * 2 + 1];
                        DrawLine(
                            (float)x1 * screenW / 320,
                            (float)y1 * screenH / 200,
                            (float)x2 * screenW / 320,
                            (float)y2 * screenH / 200,
                            RED);
                    }
                }
            }            
        }

        //setCamera(fs->cameras[0]);
        //BeginMode3D(mainCamera);
        //renderDebug();
        ////DrawCube( {0, 0, 0 }, 2.0f, 2.0f, 2.0f, RED);
        ////DrawGrid(10, 1.0f);
        //EndMode3D();

        drawDebugText();
        EndDrawing();
    }

    CloseWindow();
}
