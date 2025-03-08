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

bool renderLayers[10] = { true, true, true, true, true, true, true, true, true, true };

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
        sprintf(str, "data/floor_%02d/camera_%02d/background.png", floor, camera);
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

void getRoomsIn() {

}

void drawOverlayLines() {
    for (int i = 0; i < curCamera->viewedRoomTable.size(); i++) {
        auto vw = &curCamera->viewedRoomTable[i];
        for (int i2 = 0; i2 < vw->overlays_V1.size(); i2++) {
            auto mask = &vw->overlays_V1[i2];
            for (int i3 = 0; i3 < mask->polygons.size(); i3++) {
                auto psize = mask->polygons[i3].size();
                for (int i4 = 0; i4 < psize; i4++) {
                    auto p1 = mask->polygons[i3][i4];
                    auto p2 = mask->polygons[i3][(i4 + 1) % psize];
                    DrawLineEx(
                        { (float)p1.x * screenW / 320, (float)p1.y * screenH / 200 },
                        { (float)p2.x * screenW / 320, (float)p2.y * screenH / 200 },
                        2, RED);
                }
            }
        }
    }
}

void drawColliders() {

}

void set3DCamera() {

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
                backgroundTex,
                { 0, 0, (float)backgroundTex.width, (float)backgroundTex.height },
                { 0, 0, (float)screenW, (float)screenH },
                { 0, 0 }, 0, WHITE
            );
        }

        if (renderLayers[2]) {
            drawOverlayLines();
        }



        //setCamera(fs->cameras[0]);
        //BeginMode3D(mainCamera);
        //renderDebug();
        ////DrawCube( {0, 0, 0 }, 2.0f, 2.0f, 2.0f, RED);
        ////DrawGrid(10, 1.0f);
        //EndMode3D();

        if (renderLayers[0]) {
            drawDebugText();
        }

        EndDrawing();
    }

    CloseWindow();
}
