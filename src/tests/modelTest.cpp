#include <raylib.h>
#include <rlgl.h>
#include <string.h>
#include <string>
#include <iostream>
//#include "extractor/extractor.h"
//#include <renderer.h>

#include "../extractor/loaders/loaders.h"

namespace ModelTest {

    int screenW = 1280;
    int screenH = 960;

    //if (IsKeyDown(KEY_RIGHT)) {
    //    testPos.x += frameTime * 2;
    //}
    //if (IsKeyDown(KEY_LEFT)) {
    //    testPos.x -= frameTime * 2;
    //}
    //if (IsKeyDown(KEY_UP)) {
    //    testPos.z += frameTime * 2;
    //}
    //if (IsKeyDown(KEY_DOWN)) {
    //    testPos.z -= frameTime * 2;
    //}
    //for (int i = 0; i < 10; i++) {
    //    if (IsKeyPressed(KEY_ZERO + i)) {
    //        renderLayers[i] = !renderLayers[i];
    //    }
    //}
    //DrawPlane(testPos, { 10, 10 }, DARKGRAY);

    void runTest(void)
    {
        // Initialization
        //--------------------------------------------------------------------------------------
        const int screenWidth = 800;
        const int screenHeight = 450;

        InitWindow(screenWidth, screenHeight, "raylib [models] example - loading gltf animations");

        // Define the camera to look into our 3d world
        Camera camera = { 0 };
        camera.position = { 6.0f, 6.0f, 6.0f };    // Camera position
        camera.target = { 0.0f, 1.0f, 0.0f };      // Camera looking at point
        camera.up = { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
        camera.fovy = 60.0f;                                // Camera field-of-view Y
        camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

        // Load gltf model
        Model model = LoadModel("modeltest/model.gltf");
        Vector3 position = { 0.0f, 0.0f, 0.0f }; // Set model position

        // Load gltf model animations
        int animsCount = 0;
        unsigned int animIndex = 0;
        unsigned int animCurrentFrame = 0;
        ModelAnimation* modelAnimations = LoadModelAnimations("modeltest/model.gltf", &animsCount);
        ModelAnimation anim;

        SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
        //--------------------------------------------------------------------------------------

        // Main game loop
        while (!WindowShouldClose())        // Detect window close button or ESC key
        {
            // Update
            //----------------------------------------------------------------------------------
            UpdateCamera(&camera, CAMERA_ORBITAL);

            // Select current animation
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) animIndex = (animIndex + 1) % animsCount;
            else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) animIndex = (animIndex + animsCount - 1) % animsCount;

            // Update model animation
            if (animsCount) {
                anim = modelAnimations[animIndex];
                animCurrentFrame = (animCurrentFrame + 1) % anim.frameCount;
                UpdateModelAnimation(model, anim, animCurrentFrame);
            }
            //----------------------------------------------------------------------------------

            // Draw
            //----------------------------------------------------------------------------------
            BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
            DrawModel(model, position, 1.0f, WHITE);    // Draw animated model
            DrawGrid(10, 1.0f);
            EndMode3D();

            if (animsCount) {
                DrawText("Use the LEFT/RIGHT mouse buttons to switch animation", 10, 10, 20, GRAY);
                DrawText(TextFormat("Animation: %s", anim.name), 10, GetScreenHeight() - 20, 10, DARKGRAY);
            }

            EndDrawing();
            //----------------------------------------------------------------------------------
        }

        // De-Initialization
        //--------------------------------------------------------------------------------------
        UnloadModel(model);         // Unload model and meshes/material
        UnloadModelAnimations(modelAnimations, animsCount);

        CloseWindow();              // Close window and OpenGL context
        //--------------------------------------------------------------------------------------
    }

}