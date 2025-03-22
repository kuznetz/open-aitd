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