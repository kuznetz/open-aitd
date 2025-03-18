#pragma once
#include "structs/int_types.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>

#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

struct actorFlags
{
	u16 flag0x0001 : 1;
	u16 flag0x0002 : 1;
	u16 flag0x0004 : 1;
	u16 flag0x0008 : 1;
	u16 flag0x0010 : 1;
	u16 flag0x0020 : 1;
	u16 flag0x0040 : 1;
	u16 tackable : 1;
	u16 flag0x0100 : 1;
	u16 flag0x0200 : 1;
	u16 flag0x0400 : 1;
	u16 flag0x0800 : 1;
	u16 flag0x1000 : 1;
	u16 flag0x2000 : 1;
	u16 flag0x4000 : 1;
	u16 flag0x8000 : 1;
};

struct gameObjectStruct
{
	short int ownerIdx;
	short int body;
	union
	{
		short int flags;
		actorFlags bitField;
	};
	short int field_6;
	short int foundBody;
	short int foundName;
	short int flags2;
	short int foundLife;
	short int x;
	short int y;
	short int z;
	short int alpha;
	short int beta;
	short int gamma;
	short int stage;
	short int room;
	short int lifeMode;
	short int life;
	short int field_24;
	short int anim;
	short int frame;
	short int animType;
	short int animInfo;
	short int trackMode;
	short int trackNumber;
	short int positionInTrack;

	// AITD2
	short int mark;
};

void extractObjects(string from, string josnTo) {
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
    
    //objectTable = (objectStruct*)malloc(maxObjects * sizeof(objectStruct));
    json outJson = json::array();
    
    for (int i = 0; i < maxObjects; i++)
    {       
        json objJson = json::object();
        objJson["ownerIdx"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["body"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["flags"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["field_6"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["foundBody"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["foundName"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["flags2"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["foundLife"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        json position = json::array();
        position.push_back((float)READ_LE_S16((pObjectData + 0)) / 1000);
        position.push_back(-(float)READ_LE_S16((pObjectData + 2)) / 1000);
        position.push_back((float)READ_LE_S16((pObjectData + 4)) / 1000);
        objJson["position"] = position;
        pObjectData += 6;

        json rotation = json::array();
        rotation.push_back((float)READ_LE_S16((pObjectData + 0)) * 360 / 1024);
        rotation.push_back((float)READ_LE_S16((pObjectData + 2)) * 360 / 1024);
        rotation.push_back((float)READ_LE_S16((pObjectData + 4)) * 360 / 1024);
        objJson["rotation"] = rotation;
        pObjectData += 6;

        objJson["floor"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["room"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["lifeMode"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["life"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["field_24"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["anim"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["frame"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["animType"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["animInfo"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["trackMode"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["trackNumber"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        objJson["positionInTrack"] = READ_LE_S16(pObjectData);
        pObjectData += 2;

        //if (gameId >= JACK)
        //{
        //    objJson["mark"] = READ_LE_S16(pObjectData);
        //    pObjectData += 2;
        //}
        outJson.push_back(objJson);
    }

    delete[] pObjectDataStart;
    std::ofstream o(josnTo.c_str());
    o << std::setw(2) << outJson << std::endl;
}