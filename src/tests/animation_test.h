#pragma once
#include "../engine/resources/model.h"
#include "../engine/raylib.h"

//using namespace std;

namespace AnimationTest {
    using namespace raylib;

    const char* modelPath = "data/models/12_alt/model.gltf";
    const int fps = 144;

    inline void runTest()
    {
        InitWindow(1024, 768, "Animation Test");
        //ToggleBorderlessWindowed();
        DisableCursor();
        SetTargetFPS(fps);

        Camera camera = { 0 };
        camera.position = { 6.0f, 6.0f, 6.0f };    // Camera position
        camera.target = { 0.0f, 2.0f, 0.0f };      // Camera looking at point
        camera.up = { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
        camera.fovy = 60.0f;                                // Camera field-of-view Y
        camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

        raylib::Model model = LoadModel(modelPath);
        
        int animsCount = 0;
        unsigned int animIndex = 39;//0;

        openAITD::Model anim2;
        anim2.load(modelPath);
        anim2.bakePoses(fps);

        Transform* oldPose = new Transform[anim2.bones.size()];
        Transform* newPose;
        Transform* lerpPose = new Transform[anim2.bones.size()];
        Transform* curPose = 0;
        
        float transition = 0;
        float duration = 0;
        float animTime = 0;

        unsigned int animCurrentFrame = 0;
        unsigned int animCurrentFrame2 = 0;
        ModelAnimation* modelAnimations = LoadModelAnimations(modelPath, &animsCount);

        float time = 0;

        while (!WindowShouldClose())
        {
            //UpdateCamera(&camera, CAMERA_ORBITAL);
            UpdateCamera(&camera, CAMERA_FREE);

            // Update model animation
            time += GetFrameTime();
            Vector3 rootMotion = { 0,0,0 };

            if (anim2.skin) {

                animTime += GetFrameTime();
                duration = anim2.animations[animIndex].duration;
                transition = anim2.animations[animIndex].transition;

                if (duration > 0) {
                    if (animTime >= duration) {
                        if (curPose) memcpy(oldPose, curPose, sizeof Transform * anim2.bones.size());
                        while (animTime >= duration) animTime -= duration;
                    }
                }
                else {
                    animTime = 0;
                }

                if (IsKeyPressed(KEY_LEFT_BRACKET)) {
                    if (curPose) memcpy(oldPose, curPose, sizeof Transform * anim2.bones.size());
                    animTime = 0;
                    animIndex = (animIndex + 1) % animsCount;
                }
                else if (IsKeyPressed(KEY_RIGHT_BRACKET)) {
                    if (curPose) memcpy(oldPose, curPose, sizeof Transform * anim2.bones.size());
                    animTime = 0;
                    animIndex = (animIndex + animsCount - 1) % animsCount;
                }

                //old
                ModelAnimation& anim = modelAnimations[animIndex];
                animCurrentFrame = (int)(animTime * 60) % anim.frameCount;
                UpdateModelAnimation(model, anim, animCurrentFrame);
                rootMotion = anim.framePoses[animCurrentFrame][0].translation;
                //new

                animCurrentFrame2 = (int)(animTime * fps) % anim2.animations[animIndex].bakedPoses.size();
                if (oldPose && (duration > 0) && animTime <= transition) {
                    newPose = anim2.animations[animIndex].bakedPoses[animCurrentFrame2].data();
                    newPose[0].translation = { 0,0,0 };
                    anim2.PoseLerp(lerpPose, oldPose, newPose, animTime / transition);
                    //anim2.CalcPoseByTime(newPose, animIndex, 0);
                    curPose = lerpPose;
                }
                else {
                    curPose = anim2.animations[animIndex].bakedPoses[animCurrentFrame2].data();
                    curPose[0].translation = { 0,0,0 };
                    //anim2.CalcPoseByTime(curPose, animIndex, animTime);
                }

                anim2.ApplyPose(curPose);

            }

            BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

            rlPushMatrix();
            rlTranslatef(-rootMotion.x, -rootMotion.y, -rootMotion.z);
            DrawModel(model, { -0.5f, 0.0f, 0.0f }, 1.0f, WHITE);
            rlPopMatrix();

            rlPushMatrix();
            rlTranslatef(0.5f, 0, 0);
            anim2.Render();
            rlPopMatrix();

            DrawGrid(10, 1.0f);
            EndMode3D();

            if (anim2.skin) {
                ModelAnimation& anim = modelAnimations[animIndex];
                DrawText(TextFormat("Animation: %s", anim.name), 10, GetScreenHeight() - 20, 10, DARKGRAY);
            }
            DrawFPS(10, 10);

            EndDrawing();
        }

        UnloadModel(model);
        CloseWindow();
    }

}