#include <raylib.h>
//#include <rlgl.h>
#include "extractor/background_extractor.h"
#include "extractor/floor_extractor.h"

#include <fstream>

int screenW = 1280;
int screenH = 800;

int main(void)
{
    char fname[50];
    for (int i = 0; i < 8; i++) {
        sprintf(fname, "ETAGE0%d", i);
        floorStruct* fs = loadFloorPak(fname);
        //saveFloorTxt(fname, fs);
    }

    InitWindow(screenW, screenH, "Open AITD");
    SetTargetFPS(60);

    extractBackground("CAMERA00", 0);
    extractBackground("CAMERA00", 1);
    extractBackground("CAMERA00", 2);

    //Format24bppRgb
    Image backgroundImage = GenImageChecked(screenW, screenH, 40, 40, ORANGE, YELLOW);
    Texture2D backgroundTexture = LoadTextureFromImage(backgroundImage);
    Texture2D texture = LoadTexture(ASSETS_PATH"test.png"); // Check README.md for how this works


    while (!WindowShouldClose())
    {
        BeginDrawing();

        //ClearBackground(RAYWHITE);
        //DrawTexture(backgroundTexture, 0, 0, WHITE);
        DrawTextureEx(backgroundTexture, { 0,0 }, 0, 4, WHITE);

        const int texture_x = screenW / 2 - texture.width / 2;
        const int texture_y = screenH / 2 - texture.height / 2;
        //DrawTexture(texture, texture_x, texture_y, WHITE);

        const char* text = "OMG! IT WORKS!";
        const Vector2 text_size = MeasureTextEx(GetFontDefault(), text, 20, 1);
        DrawText(text, screenW / 2. - text_size.x / 2, texture_y + texture.height + text_size.y + 10, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
