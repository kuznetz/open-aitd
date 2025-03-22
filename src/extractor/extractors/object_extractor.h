#pragma once
#include "../structs/int_types.h"
#include "../structs/game_objects.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>

#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

inline vector <gameObjectStruct> loadGameObjects(string from) {
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

        obj.field_6 = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.foundBody = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.foundName = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.flags2 = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.foundLife = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.x = READ_LE_S16(pObjectData + 0);
        obj.y = READ_LE_S16(pObjectData + 2);
        obj.z = READ_LE_S16(pObjectData + 4);
        pObjectData += 6;

        obj.alpha = READ_LE_S16(pObjectData + 0);
        obj.beta = READ_LE_S16(pObjectData + 2);
        obj.gamma = READ_LE_S16(pObjectData + 4);
        pObjectData += 6;

        obj.floor = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.room = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.lifeMode = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.life = READ_LE_S16(pObjectData);
        pObjectData += 2;

        obj.field_24 = READ_LE_S16(pObjectData);
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

inline void extractGameObjects(vector <gameObjectStruct> objects, string josnTo) {
    json outJson = json::array();
    for (int i = 0; i < objects.size(); i++)
    {
        auto& obj = objects[i];
        json objJson = json::object();
        
        objJson["ownerIdx"] = obj.ownerIdx;
        objJson["body"] = obj.body;
        objJson["flags"] = obj.flags;
        objJson["field_6"] = obj.field_6;
        objJson["foundBody"] = obj.foundBody;
        objJson["foundName"] = obj.foundName;
        objJson["flags2"] = obj.flags2;
        objJson["foundLife"] = obj.foundLife;

        json position = json::array();
        position.push_back((float)obj.x / 1000);
        position.push_back(-(float)obj.y / 1000);
        position.push_back((float)obj.z / 1000);
        objJson["position"] = position;

        json rotation = json::array();
        rotation.push_back((float)obj.alpha * 360 / 1024);
        rotation.push_back((float)obj.beta * 360 / 1024);
        rotation.push_back((float)obj.gamma * 360 / 1024);
        objJson["rotation"] = rotation;

        objJson["floor"] = obj.floor;
        objJson["room"] = obj.room;
        objJson["lifeMode"] = obj.lifeMode;
        objJson["life"] = obj.life;
        objJson["field_24"] = obj.field_24;
        objJson["anim"] = obj.anim;
        objJson["frame"] = obj.frame;
        objJson["animType"] = obj.animType;
        objJson["animInfo"] = obj.animInfo;
        objJson["trackMode"] = obj.trackMode;
        objJson["trackNumber"] = obj.trackNumber;
        objJson["positionInTrack"] = obj.positionInTrack;

        outJson.push_back(objJson);
    }

    std::ofstream o(josnTo.c_str());
    o << std::setw(2) << outJson << std::endl;
}