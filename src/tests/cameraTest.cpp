#include <raylib.h>
#include <rlgl.h>
#include <string.h>
#include <string>
#include <iostream>
#include "../extractor/floor_extractor.h"
#include <renderer.h>
#include <rcamera.h>
#include <raymath.h>
#include <vector>

namespace CameraTest {

    int screenW = 1280;
    int screenH = 960;

    int curFloorId = 0;
    int curRoomId = 0;
    int curCameraId = 0;
    floorStruct* curFloor = 0;
    cameraStruct* curCamera = 0;
    Texture2D backgroundTex;
    Texture2D maskTex;

    bool useFilter = false;
    bool renderLayers[10] = { true, true, false, true, true, true, true, true, true, true };

    Camera3D testCamera = {
        { 0.0f, 5.0f, -10.0f }, // mainCamera position
        { 0.0f, 0.0f, 0.0f },   // mainCamera looking at point
        { 0.0f, 1.0f, 0.0f },   // mainCamera up vector (rotation towards target)
        45.0f,
        CAMERA_PERSPECTIVE
    };

    void DrawZVWires(Vector3 *v1, Vector3* v2, Color color)
    {
        rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);        
        // Front face
        //------------------------------------------------------------------
        // Bottom line
        rlVertex3f(v1->x, v1->y, v2->z);  // Bottom left
        rlVertex3f(v2->x, v1->y, v2->z);  // Bottom right
        // Left line
        rlVertex3f(v2->x, v1->y, v2->z);  // Bottom right
        rlVertex3f(v2->x, v2->y, v2->z);  // Top right
        // Top line
        rlVertex3f(v2->x, v2->y, v2->z);  // Top right
        rlVertex3f(v1->x, v2->y, v2->z);  // Top left
        // Right line
        rlVertex3f(v1->x, v2->y, v2->z);  // Top left
        rlVertex3f(v1->x, v1->y, v2->z);  // Bottom left
        // Back face
        //------------------------------------------------------------------
        // Bottom line
        rlVertex3f(v1->x, v1->y, v1->z);  // Bottom left
        rlVertex3f(v2->x, v1->y, v1->z);  // Bottom right
        // Left line
        rlVertex3f(v2->x, v1->y, v1->z);  // Bottom right
        rlVertex3f(v2->x, v2->y, v1->z);  // Top right
        // Top line
        rlVertex3f(v2->x, v2->y, v1->z);  // Top right
        rlVertex3f(v1->x, v2->y, v1->z);  // Top left
        // Right line
        rlVertex3f(v1->x, v2->y, v1->z);  // Top left
        rlVertex3f(v1->x, v1->y, v1->z);  // Bottom left
        // Top face
        //------------------------------------------------------------------
        // Left line
        rlVertex3f(v1->x, v2->y, v2->z);  // Top left front
        rlVertex3f(v1->x, v2->y, v1->z);  // Top left back
        // Right line
        rlVertex3f(v2->x, v2->y, v2->z);  // Top right front
        rlVertex3f(v2->x, v2->y, v1->z);  // Top right back
        // Bottom face
        //------------------------------------------------------------------
        // Left line
        rlVertex3f(v1->x, v1->y, v2->z);  // Top left front
        rlVertex3f(v1->x, v1->y, v1->z);  // Top left back
        // Right line
        rlVertex3f(v2->x, v1->y, v2->z);  // Top right front
        rlVertex3f(v2->x, v1->y, v1->z);  // Top right back
        rlEnd();
    }

    void setCamera(cameraStruct* camera) {
        //
    }

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
            //SetTextureFilter(backgroundTex, TEXTURE_FILTER_BILINEAR);
            testCamera.position = { 
                -(float)curCamera->x / 100,
                (float)curCamera->y / 100,
                -(float)curCamera->z / 100,
            };
            testCamera.target = {
                -(float)curCamera->x / 100,
                (float)curCamera->y / 100,
                -(float)curCamera->z / 100 + 1,
            };
        }

        //Image backgroundImage = GenImageChecked(screenW, screenH, 40, 40, ORANGE, YELLOW);
        //Texture2D backgroundTexture = LoadTextureFromImage(backgroundImage);

        curFloorId = floor;
        curRoomId = 0;
        curCameraId = camera;
    }

    void drawDebugText() {
        char text[100] = "";
        sprintf((char*)text, "FLOOR:%d CAMERA:%d %f %f %f", curFloorId, curCameraId, testCamera.position.x, testCamera.position.y, testCamera.position.z);
        const Vector2 text_size = MeasureTextEx(GetFontDefault(), (char*)text, 30, 1);
        DrawText((char*)text, screenW / 2. - text_size.x / 2, screenH - (text_size.y + 10), 30, WHITE);
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
        for (int i = 0; i < curCamera->viewedRoomTable.size(); i++) {
            auto vw = &curCamera->viewedRoomTable[i];
            auto curRoom = &curFloor->rooms[vw->viewedRoomIdx];
            Vector3 RoomV = {
                (float)curRoom->worldX / 100,
                -(float)curRoom->worldY / 100,
                -(float)curRoom->worldZ / 100
            };
            DrawSphere(RoomV, 0.2f, GREEN);
            for (int colIdx = 0; colIdx < curRoom->hardColTable.size(); colIdx++) {
                auto col = &curRoom->hardColTable[colIdx];
                //rlPushMatrix();
                //rlLoadIdentity();
                //rlTranslatef(curRoom->worldX, curRoom->worldX, curRoom->worldZ);
                //rlPopMatrix();
                Vector3 V1 = {
                    (float)col->zv.ZVX1 / 1000,
                    -(float)col->zv.ZVY1 / 1000,
                    (float)col->zv.ZVZ1 / 1000
                };
                Vector3 V2 = {
                    (float)col->zv.ZVX2 / 1000,
                    -(float)col->zv.ZVY2 / 1000,
                    (float)col->zv.ZVZ2 / 1000
                };
                V1 = Vector3Add(V1, RoomV);
                V2 = Vector3Add(V2, RoomV);
                V1.x = -V1.x;
                V2.x = -V2.x;
                DrawZVWires(&V1, &V2, RED);
            }
        }
    }

    void drawOverlayZones() {
        for (int i = 0; i < curCamera->viewedRoomTable.size(); i++) {
            auto vw = &curCamera->viewedRoomTable[i];
            for (int i2 = 0; i2 < vw->overlays_V1.size(); i2++) {
                auto mask = &vw->overlays_V1[i2];
                auto curRoom = &curFloor->rooms[vw->viewedRoomIdx];
                Vector3 RoomV = {
                    (float)curRoom->worldX / 100,
                    -(float)curRoom->worldY / 100,
                    -(float)curRoom->worldZ / 100
                };
                for (int i3 = 0; i3 < mask->zones.size(); i3++) {
                    auto zone = &mask->zones[i3];
                    Vector3 V1 = {
                        (float)zone->zoneX1 / 100,
                        0,
                        (float)zone->zoneZ1 / 100
                    };
                    Vector3 V2 = {
                        (float)zone->zoneX2 / 100,
                        0,
                        (float)zone->zoneZ2 / 100
                    };
                    V1 = Vector3Add(V1, RoomV);
                    V2 = Vector3Add(V2, RoomV);
                    V1.x = -V1.x;
                    V2.x = -V2.x;
                    DrawZVWires(&V1, &V2, BLUE);
                }
            }
        }
    }

    void runTest()
    {
        backgroundTex.id = 0;
        InitWindow(screenW, screenH, "Open AITD");
        SetTargetFPS(60);
        changeCamera(0, 0);

        Image image = GenImageChecked(screenW, screenH, 10, 10, BLACK, WHITE);
        //Image image = LoadImage("data/mask_test.png");
        maskTex = LoadTextureFromImage(image);
        DisableCursor();

        while (!WindowShouldClose())
        {
            if (IsKeyPressed(KEY_ESCAPE)) {
                CloseWindow();
            }
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
            if (IsKeyPressed(KEY_F)) {
                useFilter = !useFilter;
                SetTextureFilter(backgroundTex, useFilter? TEXTURE_FILTER_BILINEAR: TEXTURE_FILTER_POINT );
            }            
            for (int i = 0; i < 10; i++) {
                if (IsKeyPressed(KEY_ZERO + i)) {
                    renderLayers[i] = !renderLayers[i];
                }
            }

            UpdateCamera(&testCamera, CAMERA_FREE);

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

            BeginMode3D(testCamera);

                if (renderLayers[3]) {
                    drawOverlayZones();
                }
                if (renderLayers[4]) {
                    drawColliders();
                }

            //DrawCube({ 0, 3, 0 }, 2.0f, 2.0f, 2.0f, BLUE);
            //DrawCube({ 0, -3, 0 }, 2.0f, 2.0f, 2.0f, RED);

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