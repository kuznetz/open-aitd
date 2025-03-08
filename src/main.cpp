#include <cameraTester.h>
#include <cassert>
#include <cstdlib>
#include <filesystem>

#include "extractor/floor_extractor.h"
#include "extractor/background_extractor.h"

void extractAllData() {
    char str[100];
    char str2[100];
    for (int fl = 7; fl < 8; fl++) {
        //std::filesystem::create_directories("original");
        //std::filesystem::create_directories("backgrounds");
        sprintf(str, "original/ETAGE%02d", fl);
        auto curFloor = loadFloorPak(str);
        for (int cam = 0; cam < curFloor->cameras.size(); cam++) {
            std::filesystem::create_directories("backgrounds");
            sprintf(str, "original/CAMERA%02d", fl );
            sprintf(str2, "backgrounds/%d_%d.png", fl, cam);
            extractBackground(str, cam, str2);
        }
        delete curFloor;
    }    
}

int main(void)
{
    //extractAllData();

    //char fname[50];
    //for (int i = 0; i < 8; i++) {
    //    sprintf(fname, "ETAGE0%d", i);
    //    floorStruct* fs = loadFloorPak(fname);
    //    //saveFloorTxt(fname, fs);
    //}
    //floorStruct* fs = loadFloorPak("ETAGE01");

    //extractBackground("CAMERA00", 0);
    //extractBackground("CAMERA00", 1);
    //extractBackground("CAMERA00", 2);

    //Format24bppRgb
    
    //Image backgroundImage = GenImageChecked(screenW, screenH, 40, 40, ORANGE, YELLOW);
    //Texture2D backgroundTexture = LoadTextureFromImage(backgroundImage);

    //Texture2D texture = LoadTexture(ASSETS_PATH"test.png"); // Check README.md for how this works

    runCameraTester();

    /*
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(DARKGRAY);
        //    //DrawTexture(backgroundTexture, 0, 0, WHITE);
        //     DrawTextureEx(backgroundTexture, { 0,0 }, 0, 1, WHITE);
        //    DrawTexture
        //    //const int texture_x = screenW / 2 - texture.width / 2;
        //    //const int texture_y = screenH / 2 - texture.height / 2;
        //    //DrawTexture(texture, texture_x, texture_y, WHITE);

        setCamera(fs->cameras[0]);
        BeginMode3D(mainCamera);
            renderDebug();
            //DrawCube( {0, 0, 0 }, 2.0f, 2.0f, 2.0f, RED);
            //DrawGrid(10, 1.0f);
        EndMode3D();

        const char* text = "OMG! IT WORKS!";
        const Vector2 text_size = MeasureTextEx(GetFontDefault(), text, 20, 1);
        DrawText(text, screenW / 2. - text_size.x / 2, screenH - (text_size.y + 10), 20, BLACK);
        EndDrawing();
    }*/

    return 0;
}
