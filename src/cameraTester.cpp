#include <raylib.h>
#include <string.h>
#include <string>
#include <iostream>
#include "extractor/background_extractor.h"
#include "extractor/floor_extractor.h"
#include <renderer.h>

int screenW = 1280;
int screenH = 800;

int curFloorId = 0;
int curRoomId = 0;
int curCameraId = 0;
floorStruct* curFloor = 0;

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
