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

        //location
        if (obj.floor != -1) {
            objJson["location"] = json::object();
            auto& loc = objJson["location"];

            json position = json::array();
            position.push_back((float)obj.x / 1000);
            position.push_back(-(float)obj.y / 1000);
            position.push_back((float)obj.z / 1000);
            loc["position"] = position;

            json rotation = json::array();
            rotation.push_back((float)obj.alpha * 360 / 1024);
            rotation.push_back((float)obj.beta * 360 / 1024);
            rotation.push_back((float)obj.gamma * 360 / 1024);
            loc["rotation"] = rotation;

            loc["stageId"] = obj.floor;
            loc["roomId"] = obj.room;
        }

        //model
        if (obj.body != -1) {
            objJson["model"] = json::object();
            objJson["model"]["id"] = obj.body;
            objJson["model"]["animId"] = obj.anim;
            objJson["model"]["animType"] = obj.animType;
            objJson["model"]["animInfo"] = obj.animInfo;
        }

        //inventory
        if (obj.foundBody != -1) {
            objJson["invItem"] = json::object();
            objJson["invItem"]["ownerId"] = obj.ownerIdx;
            objJson["invItem"]["model"] = obj.foundBody;
            objJson["invItem"]["name"] = obj.foundName;
            objJson["invItem"]["life"] = obj.foundLife;
        }

        objJson["trackMode"] = obj.trackMode;
        objJson["trackNumber"] = obj.trackNumber;
        //TODO: Convert trackPosition?
        objJson["trackPosition"] = obj.positionInTrack;

        objJson["lifeMode"] = obj.lifeMode;
        objJson["life"] = obj.life;

        objJson["flags"] = obj.flags;
        objJson["flags2"] = obj.flags2;
        objJson["field_6"] = obj.field_6;
        objJson["field_24"] = obj.field_24;

        outJson.push_back(objJson);
    }

    std::ofstream o(josnTo.c_str());
    o << std::setw(2) << outJson << std::endl;
}