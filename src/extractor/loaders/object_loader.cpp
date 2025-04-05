#include "../structs/int_types.h"
#include "../structs/game_objects.h"
#include <vector>
#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>

using namespace std;

vector <gameObjectStruct> loadGameObjects(string from) {
    auto fHandle = fopen(from.c_str(), "rb");
    //if (!fHandle) theEnd(0, "OBJETS.ITD");
    fseek(fHandle, 0, SEEK_END);
    int objectDataSize = ftell(fHandle);
    fseek(fHandle, 0, SEEK_SET);
    u8* pObjectDataStart = new u8[objectDataSize];
    if (fread(pObjectDataStart, objectDataSize, 1, fHandle) != 1)
        throw new exception("Error reading OBJETS.ITD");
    fclose(fHandle);

    u8* pObjectData = pObjectDataStart;
    int maxObjects = READ_LE_U16(pObjectData);
    pObjectData += 2;

    vector <gameObjectStruct> objects;

    for (int i = 0; i < maxObjects; i++)
    {
        gameObjectStruct obj;
        obj.ownerIdx = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.body = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.flags = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.boundsType = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.inventoryBody = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.inventoryName = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.invFlags = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.inventoryLife = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.x = READ_LE_S16(pObjectData + 0);
        obj.y = READ_LE_S16(pObjectData + 2);
        obj.z = READ_LE_S16(pObjectData + 4);
        pObjectData += 6;

        obj.alpha = READ_LE_S16(pObjectData + 0);
        obj.beta = READ_LE_S16(pObjectData + 2);
        obj.gamma = READ_LE_S16(pObjectData + 4);
        pObjectData += 6;

        obj.stageId = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.roomId = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.lifeMode = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.life = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.stageLife = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.anim = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.frame = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.animType = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.animInfo = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.trackMode = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.trackNumber = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.positionInTrack = READ_LE_S16(pObjectData);
        pObjectData += 2;

        //if (gameId >= JACK)
        //{
        //    objJson["mark"] = READ_LE_S16(pObjectData);
        //    pObjectData += 2;
        //}
        objects.push_back(obj);
    }

    delete[] pObjectDataStart;
    return objects;
}