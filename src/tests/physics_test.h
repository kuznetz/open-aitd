#pragma once
#include <string>
#include "../engine/resources/stages.h"
#include "../engine/raylib.h"

namespace PhysicsTest {
    using namespace std;
    using namespace raylib;
    
    const int screenWidth = 1024;
    const int screenHeight = 768;

    Camera2D camera = { 0 };
    openAITD::Stage stage;

    struct Dynamic {
        Vector3 pos;
        Vector3 v;
        BoundingBox bb;
    };
    Dynamic dynamics[2] = {
        {{0.5,0,0}},
        {0,0,0.5}
    };
    Color colors[2] = { RED,GREEN };

    void DrawBounds(BoundingBox bb, Color c) {        
        DrawRectangleLinesEx({
            bb.min.x, bb.min.z,
            bb.max.x - bb.min.x,
            bb.max.z - bb.min.z
        }, 1.f/25, c);
    }

    void setBB(Dynamic& d) {
        d.bb = { { d.pos.x - 0.2f, 0, d.pos.z - 0.2f }, { d.pos.x + 0.2f, 1.5, d.pos.z + 0.2f } };
    }

    //bool CollBoxToBox0(BoundingBox& b1_0, Vector3& v, BoundingBox& b2) {
    //    if (v.x == 0 && v.z == 0) return false;
    //    BoundingBox b1 = { Vector3Add(b1_0.min, v), Vector3Add(b1_0.max, v) };
    //    if (
    //        (b1.min.x > b2.max.x || b1.max.x < b2.min.x) &&
    //        (b2.min.x > b1.max.x || b2.max.x < b1.min.x)
    //        )  return false;
    //    if (
    //        (b1.min.y > b2.max.y || b1.max.y < b2.min.y) &&
    //        (b2.min.y > b1.max.y || b2.max.y < b1.min.y)
    //        )  return false;
    //    if (
    //        (b1.min.z > b2.max.z || b1.max.z < b2.min.z) &&
    //        (b2.min.z > b1.max.z || b2.max.z < b1.min.z)
    //        )  return false;

    //    float revX = 0;
    //    float revZ = 0;
    //    if (v.x != 0) {
    //        revX = (v.x < 0) ? (b2.max.x - b1.min.x) : (b2.min.x - b1.max.x);
    //    }
    //    if (v.z != 0) {
    //        revZ = (v.z < 0) ? (b2.max.z - b1.min.z) : (b2.min.z - b1.max.z);
    //    }

    //    if (abs(revX) < abs(revZ)) {
    //        v.x += revX;
    //    }
    //    else {
    //        v.z += revZ;
    //    }
    //    return true;
    //}

    Vector3 CalculateMTV(const BoundingBox& b1, const BoundingBox& b2)
    {
        float overlapX = min(b1.max.x, b2.max.x) - max(b1.min.x, b2.min.x);
        float overlapZ = min(b1.max.z, b2.max.z) - max(b1.min.z, b2.min.z);

        if (overlapX <= 0 || overlapZ <= 0) {
            return { 0,0,0 };
        }

        bool pushAlongX = abs(overlapX) < abs(overlapZ);

        Vector3 mtv = { 0,0,0 };
        if (pushAlongX) {
            mtv.x = overlapX * ((b1.min.x + b1.max.x) / 2 >= (b2.min.x + b2.max.x) / 2 ? -1 : 1);
        }
        else {
            mtv.z = overlapZ * ((b1.min.z + b1.max.z) / 2 >= (b2.min.z + b2.max.z) / 2 ? -1 : 1);
        }

        return mtv;
    }

    bool CollBoxToBox(BoundingBox& b1_0, Vector3& v, BoundingBox& b2) {
        if (v.x == 0 && v.z == 0) return false;
        BoundingBox b1 = { Vector3Add(b1_0.min, v), Vector3Add(b1_0.max, v) };

        if (b1.max.x < b2.min.x || b1.min.x > b2.max.x)  return false;
        if (b1.max.y < b2.min.y || b1.min.y > b2.max.y)  return false;
        if (b1.max.z < b2.min.z || b1.min.z > b2.max.z)  return false;

        auto& mtv = CalculateMTV(b1, b2);
        v.x -= mtv.x;
        v.z -= mtv.z;
        return true;
    }

    inline void runTest()
    {
        stage.load("data/stages/0");
        auto& colls = stage.rooms[0].colliders;

        camera.target = { 0, 0 };
        camera.offset = { screenWidth / 2.0f, screenHeight / 2.0f };
        camera.rotation = 0.0f;
        camera.zoom = 50.0f;

        InitWindow(screenWidth, screenHeight, "PhysicsTest");
        SetTargetFPS(60);
        while (!WindowShouldClose())
        {
            float timeDelta = GetFrameTime();
            
            dynamics[0].v = { 0,0,0 };
            if (IsKeyDown(KEY_RIGHT)) {
                dynamics[0].v.x = timeDelta * 3;
            }
            if (IsKeyDown(KEY_LEFT)) {
                dynamics[0].v.x = timeDelta * -3;
            }
            if (IsKeyDown(KEY_UP)) {
                dynamics[0].v.z = timeDelta * -3;
            }
            if (IsKeyDown(KEY_DOWN)) {
                dynamics[0].v.z = timeDelta * 3;
            }

            dynamics[1].v = { 0,0,0 };
            if (IsKeyDown(KEY_RIGHT_BRACKET)) {
                dynamics[1].v.x = timeDelta * 3;
            }
            if (IsKeyDown(KEY_LEFT_BRACKET)) {
                dynamics[1].v.x = timeDelta * -3;
            }

            BeginDrawing();
            ClearBackground(BLACK);

            BeginMode2D(camera);

            for (int i = 0; i < 2; i++) {
                auto& d = dynamics[i];
                for (int j = 0; j < colls.size(); j++) {
                    CollBoxToBox(d.bb, d.v, colls[j].bounds);
                }
                for (int j = 0; j < 2; j++) {
                    if (i == j) continue;
                    CollBoxToBox(d.bb, d.v, dynamics[j].bb);
                }
                dynamics[i].pos += dynamics[i].v;
                setBB(dynamics[i]);
            }

            for (int i = 0; i < colls.size(); i++) {
                auto& coll = colls[i];
                DrawBounds(coll.bounds, GRAY);
            }

            for (int i = 0; i < 2; i++) {
                DrawBounds(dynamics[i].bb, colors[i]);
            }

            EndMode2D();
            //DrawFPS(5, 5);
            EndDrawing();
        }

        CloseWindow();
    }

}