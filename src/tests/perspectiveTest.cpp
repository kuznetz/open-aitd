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

    int curFloorId = 2;
    int curRoomId = 0;
    int curCameraId = 0;
    int curCollider = 0;
    floorStruct* curFloor = 0;
    roomStruct* curRoom = 0;
    cameraStruct* curCamera = 0;
    Texture2D backgroundTex;
    Texture2D maskTex;

    bool useFilter = false;
    bool renderLayers[10] = { true, true, false, true, true, true, true, true, true, true };

    Matrix projection;
    Vector3 cameraForw;
    float cameraAspect;
    float testFovX2 = 55.5f;
    float testFovY2 = 70.0f;
    float testFovK = 1;
    float testFovY = 55.5f;
    float testFovX = 70.0f;

    Camera3D testCamera = {
        { 0.0f, 5.0f, -10.0f }, // mainCamera position
        { 0.0f, 0.0f, 0.0f },   // mainCamera looking at point
        { 0.0f, 1.0f, 0.0f },   // mainCamera up vector (rotation towards target)
        56.5f,
        CAMERA_PERSPECTIVE
    };

    //************************************

    void rlLine2D(LegacyCamera::Vector2 v1, LegacyCamera::Vector2 v2) {
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

        rlLine2D(vecs[0], vecs[1]);
        rlLine2D(vecs[1], vecs[3]);
        rlLine2D(vecs[3], vecs[2]);
        rlLine2D(vecs[2], vecs[0]);

        rlLine2D(vecs[4], vecs[5]);
        rlLine2D(vecs[5], vecs[7]);
        rlLine2D(vecs[7], vecs[6]);
        rlLine2D(vecs[6], vecs[4]);

        rlLine2D(vecs[0], vecs[4]);
        rlLine2D(vecs[1], vecs[5]);
        rlLine2D(vecs[2], vecs[6]);
        rlLine2D(vecs[3], vecs[7]);
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

        //Quaternion q = QuaternionFromEuler(
        //    (float)curCamera->alpha * 2 * PI / 1024,
        //    -(float)curCamera->beta * 2 * PI / 1024,
        //    (float)curCamera->gamma * 2 * PI / 1024
        //);        
        //Matrix m = QuaternionToMatrix(q);
        //cameraForw = { m.m8,m.m9,m.m10 };
        //Vector3 cameraUp = { m.m4,m.m5,m.m6 };

        //MatrixIdentity();
        //Matrix m = MatrixRotateZYX({
        //    (float)curCamera->alpha * 2 * PI / 1024,
        //    -(float)curCamera->beta * 2 * PI / 1024,
        //    0//(float)curCamera->gamma * 2 * PI / 1024
        //});
        
        //Rotate YXZ
        rlPushMatrix();
        rlLoadIdentity();
        rlRotatef(-(float)curCamera->beta * 360 / 1024, 0, 1, 0);
        rlRotatef((float)curCamera->alpha * 360 / 1024, 1, 0, 0);
        rlRotatef(-(float)curCamera->gamma * 360 / 1024, 0, 0, 1);
        Matrix m2 = rlGetMatrixTransform();
        rlPopMatrix();

        //m2 = m;

        cameraForw = { m2.m8,m2.m9,m2.m10 };
        Vector3 cameraUp = { m2.m4,m2.m5,m2.m6 };

        testCamera.position = {
            -(float)curCamera->x / 100,
            (float)curCamera->y / 100,
            -(float)curCamera->z / 100,
        };
        testCamera.target = {
            testCamera.position.x + cameraForw.x,
            testCamera.position.y + cameraForw.y,
            testCamera.position.z + cameraForw.z,
        };
        testCamera.up = cameraUp;
        float perspective = (float)curCamera->nearDistance / 1000;
        testCamera.position = Vector3Add(testCamera.position, Vector3Scale(cameraForw, -perspective));
        //cameraAspect = (float)curCamera->fovY / (float)curCamera->fovX;
        //transformedY1 = ((yf * cameraFovY) / (float)zf) + cameraCenterY;
        //float frustumHeight = 1;
        //testFov = 2.0f * atan(frustumHeight * 0.5f / frustumHeight);
        

        //------------------------------
        projection = MatrixPerspective(testCamera.fovy * DEG2RAD, cameraAspect, CAMERA_CULL_DISTANCE_NEAR, CAMERA_CULL_DISTANCE_FAR);
        projection = { 0 };

        //testFovY = nearPlane * tan(testFovX00 * DEG2RAD * 0.5);
        //testFovX = nearPlane * tan(testFovY00 * DEG2RAD * 0.5);

        //double nearPlane = CAMERA_CULL_DISTANCE_NEAR / 2;
        double nearPlane = perspective / 2;
        double farPlane = CAMERA_CULL_DISTANCE_FAR * 100;
        double top = nearPlane * testFovY * testFovK;
        double right = nearPlane * testFovX * testFovK;// * cameraAspect;
        //nearDistance = (float)curCamera->nearDistance / 1000;

        // MatrixFrustum(-right, right, -top, top, near, far);
        float rl = (float)(right * 2);
        float tb = (float)(top * 2);
        float fn = (float)(farPlane - nearPlane);

        projection.m0 = ((float)nearPlane * 2.0f) / rl;
        projection.m5 = ((float)nearPlane * 2.0f) / tb;
        projection.m8 = (0) / rl;
        projection.m9 = (0) / tb;
        projection.m10 = -((float)farPlane + (float)nearPlane) / fn;
        projection.m11 = -1; //-1.0f
        projection.m14 = -((float)farPlane * (float)nearPlane * 2.0f) / fn;

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
            
            testFovY = (float)curCamera->fovY / 320;
            testFovX = (float)curCamera->fovX / 200;
            testFovK = 1;
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
        //    cameraForw.x, cameraForw.y, cameraForw.z
        //);

        Vector2 text_size;

        //sprintf((char*)text, "CAMPOS:%f %f %f",
        //    //return (x / (2 * tan(M_PI * fov / 360.f)));
        //    (float)curCamera->x / 1000,
        //    (float)curCamera->y / 1000,
        //    (float)curCamera->z / 1000
        //);
        //Vector2 text_size = MeasureTextEx(GetFontDefault(), (char*)text, 30, 1);
        //DrawText((char*)text, screenW / 2. - text_size.x / 2, screenH - (text_size.y*3 + 10), 30, WHITE);

        sprintf((char*)text, "EULER:%f %f %f",
            //return (x / (2 * tan(M_PI * fov / 360.f)));
            (float)curCamera->alpha / 1024,
            (float)curCamera->beta / 1024,
            (float)curCamera->gamma / 1024
        );
        text_size = MeasureTextEx(GetFontDefault(), (char*)text, 30, 1);
        DrawText((char*)text, screenW / 2. - text_size.x / 2, screenH - (text_size.y * 3 + 10), 30, WHITE);

        //sprintf((char*)text, "CAMFORW:%f %f %f",
        //    (float)cameraForw.x,
        //    (float)cameraForw.y,
        //    (float)cameraForw.z
        //);        
        //text_size = MeasureTextEx(GetFontDefault(), (char*)text, 30, 1);
        //DrawText((char*)text, screenW / 2. - text_size.x / 2, screenH - (text_size.y * 3 + 10), 30, WHITE);

        float startX = (float)curCamera->fovY / (320); //* p / (320);
        float startY = (float)curCamera->fovX / (200); //* p / (200);
        sprintf((char*)text, "FOV: %f %f -> %f %f",
            startX,
            startY,
            testFovY,
            testFovX
        );
        text_size = MeasureTextEx(GetFontDefault(), (char*)text, 30, 1);
        DrawText((char*)text, screenW / 2. - text_size.x / 2, screenH - (text_size.y * 2 + 10), 30, WHITE);

        float p = (float)curCamera->nearDistance / 1000;
        sprintf((char*)text, "DIFF: %f (P:%f, ASP: %f)",
            testFovK,
            p,
            cameraAspect
        );
        text_size = MeasureTextEx(GetFontDefault(), (char*)text, 30, 1);
        DrawText((char*)text, screenW / 2. - text_size.x / 2, screenH - (text_size.y * 1 + 10), 30, WHITE);

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

    void drawTestLeagcy() {
        //LegacyCamera::translateX = (curCamera->x) * 10;
        //LegacyCamera::translateY = (-curCamera->y) * 10;
        //LegacyCamera::translateZ = (-curCamera->z) * 10;        
        //
        //ZVStruct zv = curFloor->rooms[0].hardColTable[curCollider].zv;
        ////ZVStruct zv = { -7800, 7800, -2500, 0, -5300, -5000 };
        //DrawZVWires2(&zv, BLUE);
    }

    void runTest()
    {
        backgroundTex.id = 0;
        InitWindow(screenW, screenH, "Perspective Test");
        SetTargetFPS(60);
        changeCamera(2, 38);

        Image image = GenImageChecked(screenW, screenH, 10, 10, BLACK, WHITE);
        //Image image = LoadImage("data/mask_test.png");
        maskTex = LoadTextureFromImage(image);
        DisableCursor();

        while (!WindowShouldClose())
        {
            if (IsKeyPressed(KEY_LEFT_BRACKET)) {
                curCollider--;
            }
            if (IsKeyPressed(KEY_RIGHT_BRACKET)) {
                curCollider++;
            }
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
            //if (IsKeyPressed(KEY_KP_ADD)) {
            //    testFovX2 -= 0.5;
            //    testFovY = tan(testFovX2 * DEG2RAD * 0.5);
            //    setCamera(curCamera);
            //}
            //if (IsKeyPressed(KEY_KP_SUBTRACT)) {
            //    testFovX2 += 0.5f;
            //    testFovY = tan(testFovX2 * DEG2RAD * 0.5);
            //    setCamera(curCamera);
            //}
            if (IsKeyPressed(KEY_PAGE_UP)) {
                testFovK += 0.02f;
                //testFovK += 1.00f;
                //testFovX = tan(testFovY2 * DEG2RAD * 0.5);
                setCamera(curCamera);
            }
            if (IsKeyPressed(KEY_PAGE_DOWN)) {
                testFovK -= 0.02f;
                //testFovK -= 1.00f;
                //testFovX = tan(testFovY2 * DEG2RAD * 0.5);
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
            if (renderLayers[4]) {
                drawTestLeagcy();
            }

            BeginMode3D(testCamera);
            rlSetMatrixProjection(projection);

                if (renderLayers[3]) {
                    drawColliders();
                }

                //DrawSphere(Vector3Add(testCamera.position,cameraForw), 0.01f, GREEN);

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