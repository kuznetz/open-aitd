#include <raylib.h>
#include <rlgl.h>
#include <string.h>
#include <string>
#include <iostream>
#include "../extractor/floor_extractor.h"
#include <renderer.h>
#include <rcamera.h>
#include <raymath.h>
#include "legacy_camera.h"
#include <vector>

namespace PerspectiveTest {

    int screenW = 1280;
    int screenH = 960;
    //int screenH = 800;

    int curFloorId = 0;
    int curRoomId = 0;
    int curCameraId = 0;
    floorStruct* curFloor = 0;
    roomStruct* curRoom = 0;
    cameraStruct* curCamera = 0;
    Texture2D backgroundTex;
    Texture2D maskTex;

    bool useFilter = false;
    bool renderLayers[10] = { true, true, false, true, true, true, true, true, true, true };

    Matrix projection;
    Vector3 testVec;
    float testFov;
    Camera3D testCamera = {
        { 0.0f, 5.0f, -10.0f }, // mainCamera position
        { 0.0f, 0.0f, 0.0f },   // mainCamera looking at point
        { 0.0f, 1.0f, 0.0f },   // mainCamera up vector (rotation towards target)
        56.5f,
        CAMERA_PERSPECTIVE
    };

    //************************************

    void draw2dline(LegacyCamera::Vector2 v1, LegacyCamera::Vector2 v2) {
        rlVertex2f(v1.x / 320 * screenW, v1.y / 200 * screenH);
        rlVertex2f(v2.x / 320 * screenW, v2.y / 200 * screenH);
    }

    void DrawZVWires2(ZVStruct* zv, Color color)
    {
        LegacyCamera::Vector2 vecs[8];
        // Front face        
        vecs[0] = LegacyCamera::projectPoint(zv->ZVX1, zv->ZVY2, zv->ZVZ2); // Top left
        vecs[1] = LegacyCamera::projectPoint(zv->ZVX2, zv->ZVY2, zv->ZVZ2); // Top right
        vecs[2] = LegacyCamera::projectPoint(zv->ZVX1, zv->ZVY1, zv->ZVZ2); // Bottom left
        vecs[3] = LegacyCamera::projectPoint(zv->ZVX2, zv->ZVY1, zv->ZVZ2); // Bottom right
        // Back face
        vecs[4] = LegacyCamera::projectPoint(zv->ZVX1, zv->ZVY2, zv->ZVZ1); // Top left
        vecs[5] = LegacyCamera::projectPoint(zv->ZVX2, zv->ZVY2, zv->ZVZ1); // Top right
        vecs[6] = LegacyCamera::projectPoint(zv->ZVX1, zv->ZVY1, zv->ZVZ1); // Bottom left
        vecs[7] = LegacyCamera::projectPoint(zv->ZVX2, zv->ZVY1, zv->ZVZ1); // Bottom right

        rlBegin(RL_LINES);
        rlColor4ub(color.r, color.g, color.b, color.a);

        draw2dline(vecs[0], vecs[1]);
        draw2dline(vecs[1], vecs[3]);
        draw2dline(vecs[3], vecs[2]);
        draw2dline(vecs[2], vecs[0]);

        draw2dline(vecs[4], vecs[5]);
        draw2dline(vecs[5], vecs[7]);
        draw2dline(vecs[7], vecs[6]);
        draw2dline(vecs[6], vecs[4]);

        draw2dline(vecs[0], vecs[4]);
        draw2dline(vecs[1], vecs[5]);
        draw2dline(vecs[2], vecs[6]);
        draw2dline(vecs[3], vecs[7]);
        rlEnd();
    }

    //************************************

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

    void setCamera(cameraStruct* curCamera) {        
        LegacyCamera::setupCamera(curCamera);

        Quaternion q = QuaternionFromEuler(
            (float)curCamera->alpha * 2 * PI / 1024,
            -(float)curCamera->beta * 2 * PI / 1024,
            -(float)curCamera->gamma * 2 * PI / 1024
        );
        testVec = { 0,0,1 };
        testVec = Vector3RotateByQuaternion(testVec, q);
        //testVec = Vector3Negate(testVec);

        testCamera.position = {
            -(float)curCamera->x / 100,
            (float)curCamera->y / 100,
            -(float)curCamera->z / 100,
        };
        testCamera.target = {
            testCamera.position.x + testVec.x,
            testCamera.position.y + testVec.y,
            testCamera.position.z + testVec.z,
        };        
        float xx = (float)curCamera->fovX / (float)curCamera->fovY;
        //transformedY1 = ((yf * cameraFovY) / (float)zf) + cameraCenterY;
        //float frustumHeight = 1;
        //testFov = 2.0f * atan(frustumHeight * 0.5f / frustumHeight);
        projection = MatrixPerspective(testCamera.fovy * DEG2RAD, xx, CAMERA_CULL_DISTANCE_NEAR, CAMERA_CULL_DISTANCE_FAR);
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
            setCamera(curCamera);
        }

        //Image backgroundImage = GenImageChecked(screenW, screenH, 40, 40, ORANGE, YELLOW);
        //Texture2D backgroundTexture = LoadTextureFromImage(backgroundImage);

        curFloorId = floor;
        curRoomId = 0;
        curCameraId = camera;
    }

    void drawDebugText() {
        char text[100] = "";
        //sprintf((char*)text, "FLOOR:%d CAMERA:%d %f %f %f", curFloorId, curCameraId, testCamera.position.x, testCamera.position.y, testCamera.position.z);
        //Vector3 tCamVec = Vector3Normalize( Vector3Add(testCamera.target, Vector3Negate(testCamera.position)));
        
        //sprintf((char*)text, "%f %f %f = %f %f %f", 
        //    tCamVec.x, tCamVec.y, tCamVec.z,
        //    testVec.x, testVec.y, testVec.z
        //);

        float p = (float)curCamera->perspective / 100;
        sprintf((char*)text, "FOV:%f, %f %f %f %f", 
            testCamera.fovy,
            p,
            (float)curCamera->fovX / 100,
            (float)curCamera->fovY / 100,
            testFov
        );
        const Vector2 text_size = MeasureTextEx(GetFontDefault(), (char*)text, 30, 1);
        DrawText((char*)text, screenW / 2. - text_size.x / 2, screenH - (text_size.y + 10), 30, WHITE);
    }

    void getRoomsIn() {

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
            //DrawSphere(RoomV, 0.02f, GREEN);
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

    void drawCollidersLeagcy() {
        for (int i = 0; i < curCamera->viewedRoomTable.size(); i++) {
            auto vw = &curCamera->viewedRoomTable[i];
            auto curRoom = &curFloor->rooms[vw->viewedRoomIdx];
            //DrawSphere(RoomV, 0.02f, GREEN);
            LegacyCamera::setupCameraRoom(curCamera, curRoom);
            for (int colIdx = 0; colIdx < curRoom->hardColTable.size(); colIdx++) {
                auto col = &curRoom->hardColTable[colIdx];
                DrawZVWires2(&col->zv, BLUE);
            }
        }
    }

    void runTest()
    {
        backgroundTex.id = 0;
        InitWindow(screenW, screenH, "Perspective Test");
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
            if (IsKeyPressed(KEY_KP_MULTIPLY)) {
                testCamera.fovy -= 0.1f;
                setCamera(curCamera);
            }
            if (IsKeyPressed(KEY_KP_ADD)) {
                testCamera.fovy -= 0.5;
                setCamera(curCamera);
            }
            if (IsKeyPressed(KEY_KP_SUBTRACT)) {
                testCamera.fovy += 0.5f;
                setCamera(curCamera);
            }
            

            //UpdateCamera(&testCamera, CAMERA_FREE);

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
                drawCollidersLeagcy();
            }            

            BeginMode3D(testCamera);
            rlSetMatrixProjection(projection);

                if (renderLayers[3]) {
                    drawColliders();
                }

                DrawSphere(Vector3Add(testCamera.position,testVec), 0.01f, GREEN);

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