#pragma once
#include <raylib.h>
#include <raymath.h>
#include <string>

namespace PosRelTest {

    using namespace std;

    struct ZVStruct
    {
        int ZVX1;
        int ZVX2;
        int ZVY1;
        int ZVY2;
        int ZVZ1;
        int ZVZ2;
    };

    struct Data {
        Vector2 p1 = { 400, 300 };
        Rectangle r1;
        ZVStruct actor1zv;
        float beta1 = 0;

        Vector2 p2 = { 400, 400 };
        Vector2 p2rot;
        Rectangle r2;
    };

    inline Data data;

    inline int getPosRelTable[] = { 4,1,8,2,4,1,8,0 };
    inline int getPosRel(ZVStruct actor1zv, int beta1, Vector2 actor2)
    {
        //int beta1 = actor1->beta;
        //ZVStruct localZv;

        int counter = 3;
        if (beta1 >= 0x80 && beta1 < 0x180)
        {
            counter = 2;
        }
        if (beta1 >= 0x180 && beta1 < 0x280)
        {
            counter = 1;
        }
        if (beta1 >= 0x280 && beta1 < 0x380)
        {
            counter = 0;
        }


        //copyZv(&actor2->zv, &localZv);
        /*if (actor1->room != actor2->room)
        {
            getZvRelativePosition(&localZv, actor2->room, actor1->room);
        }*/

        int centerX = actor2.x;
        int centerZ = actor2.y;

        if (actor1zv.ZVZ2 >= centerZ && actor1zv.ZVZ1 <= centerZ)
        {
            if (actor1zv.ZVX2 < centerX)
            {
                counter++;
            }
            else
            {
                if (actor1zv.ZVX1 <= centerX)
                {
                    return(0);
                }
                else
                {
                    counter += 3;
                }
            }
        }
        else
            if (actor1zv.ZVX2 >= centerX || actor1zv.ZVX1 <= centerX)
            {
                if (actor1zv.ZVZ2 < centerZ)
                {
                    counter += 2;
                }
                else
                {
                    if (actor1zv.ZVZ1 <= centerZ)
                    {
                        return(0);
                    }
                }
            }
            else
            {
                return(0);
            }

        return(getPosRelTable[counter]);
    }

    inline int getPosRel2(Vector2 p1, float beta1, Vector2 p2)
    {
        Vector2 v = Vector2Rotate(Vector2Subtract(p2, p1), (beta1 * -DEG2RAD));
        Vector2 p2rot = Vector2Add(p1, v);
        data.p2rot = p2rot;
        int res2 = 0;
        if (p2rot.y < (p1.y - 10)) {
            res2 = 2;
        }
        else if (p2rot.y > (p1.y + 10)) {
            res2 = 1;
        }
        else if (p2rot.x < (p1.x - 30)) {
            res2 = 8;
        }
        else if (p2rot.x > (p1.x + 30)) {
            res2 = 4;
        }
        return res2;
    }

    inline void runTest()
    {
        InitWindow(800, 600, "getPosRel Test");
        SetTargetFPS(60);
        while (!WindowShouldClose())
        {
            float timeDelta = GetFrameTime();
            if (IsKeyDown(KEY_RIGHT)) {
                data.p2.x += timeDelta * 100;
            }
            if (IsKeyDown(KEY_LEFT)) {
                data.p2.x -= timeDelta * 100;
            }
            if (IsKeyDown(KEY_UP)) {
                data.p2.y -= timeDelta * 100;
            }
            if (IsKeyDown(KEY_DOWN)) {
                data.p2.y += timeDelta * 100;
            }
            if (IsKeyDown(KEY_RIGHT_BRACKET)) {
                data.beta1 += timeDelta * 180;
                if (data.beta1 > 360) data.beta1 -= 360;
            }
            if (IsKeyDown(KEY_LEFT_BRACKET)) {
                data.beta1 -= timeDelta * 180;
                if (data.beta1 < 0) data.beta1 += 360;
            }

            data.r2 = { data.p2.x - 5, data.p2.y - 5, 10, 10 };            

            data.r1 = { data.p1.x, data.p1.y, 60, 20 };
            //data.r1 = { data.p1.x - 30, data.p1.y - 10, 60, 20 };
            data.actor1zv = { (int)(data.p1.x - 30), (int)(data.p1.x + 30), 0,0, (int)(data.p1.y - 30), (int)(data.p1.y + 30) };

            int res = getPosRel(data.actor1zv, (int)(data.beta1*1024/360) % 1024, data.p2);

            //---
            //Vector2 forw = Vector2Rotate({ 0,-1 }, -(data.beta1*DEG2RAD));
            //Vector2 forw2 = Vector2Add(data.p1, Vector2Scale(forw, 20));
            int res2 = getPosRel2(data.p1, data.beta1, data.p2);
            //---

            BeginDrawing();
            ClearBackground(DARKGRAY);


            DrawRectanglePro(data.r1, { 30,10 }, data.beta1, DARKGREEN);
            DrawCircle(data.r1.x, data.r1.y, 5, GREEN);
            DrawRectangleLines(data.actor1zv.ZVX1, data.actor1zv.ZVZ1, data.actor1zv.ZVX2 - data.actor1zv.ZVX1, data.actor1zv.ZVZ2 - data.actor1zv.ZVZ1, WHITE);
            DrawCircle(data.p2.x, data.p2.y, 5, RED);
            
            DrawRectanglePro({ data.p1.x, data.p1.y - 200, 60, 20 }, { 30,10 }, 0, GRAY);
            DrawCircle(data.p2rot.x, data.p2rot.y - 200, 5, GRAY);


            string s = string("OLD: ") + to_string(res) + " NEW: " + to_string(res2);
            DrawText(s.c_str(), 5, 5, 24, WHITE);

            //DrawFPS(5, 5);
            EndDrawing();
        }

        CloseWindow();
    }

}