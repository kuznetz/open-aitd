#include <raylib.h>
//#include <rlgl.h>
//#include "camera.hpp"
#include "extractor/extractor.hpp"
#include "extractor/pak/floor.h"

#include <fstream>

int screenW = 1280;
int screenH = 800;

void writeFloor(char* filename, floorStruct* fs) {
    char fname[50];
    sprintf(fname, "%s.txt", filename);
    std::ofstream myfile;
    myfile.open(fname);

    for (int i = 0; i < fs->cameraCount; i++) {
        cameraStruct* cam = &fs->cameras[i];
        myfile << "CAMERA:\n";
        myfile << " XYZ:" << cam->x << " " << cam->y << " " << cam->z << "\n";
        for (int i2 = 0; i2 < cam->numViewedRooms; i2++) {
            cameraViewedRoomStruct* vw = &cam->viewedRoomTable[i2];
            myfile << " VIEW:\n";
            for (int i3 = 0; i3 < vw->numV1Mask; i3++) {
                cameraMaskV1Struct* mask = &vw->V1masks[i3];
                myfile << "  MASK_V1:\n";
                for (int i4 = 0; i4 < mask->numZone; i4++) {
                    myfile << "ZONE:" << mask->zones[i4].zoneX1 << mask->zones[i4].zoneZ1 << mask->zones[i4].zoneX2 << mask->zones[i4].zoneZ2 << "\n";
                }
            }
        }
    }
    myfile.close();    
}

int main(void)
{
    char fname[50];
    for (int i = 0; i < 8; i++) {
        sprintf(fname, "ETAGE0%d", i);
        floorStruct* fs = loadFloor(fname);
        writeFloor(fname, fs);
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
