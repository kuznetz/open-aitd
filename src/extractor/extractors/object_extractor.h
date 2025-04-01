#pragma once
#include "../structs/int_types.h"
#include "../structs/game_objects.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>
#include <raymath.h>

#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

inline Matrix roomMatObj;

inline Quaternion convertRotate(int alpha, int beta, int gamma) {
    Matrix mx = MatrixRotateX(alpha * 2.f * PI / 1024);
    Matrix my = MatrixRotateY((beta + 512) * 2.f * PI / 1024);
    Matrix mz = MatrixRotateZ(gamma * 2.f * PI / 1024);
    Matrix matRotation = MatrixMultiply(MatrixMultiply(my, mx), mz);
    matRotation = MatrixTranspose(matRotation);
    return QuaternionFromMatrix(matRotation);
}

inline void extractGameObjects(vector <gameObjectStruct> objects, string josnTo) {
    roomMatObj = MatrixRotateX(PI);

    json outJson = json::array();
    for (int i = 0; i < objects.size(); i++)
    {
        auto& obj = objects[i];
        json objJson = json::object();

        //location
        if (obj.floor != -1) {
            objJson["location"] = json::object();
            auto& loc = objJson["location"];

            Vector3 v = Vector3Transform({
                obj.x / 1000.f,
                obj.y / 1000.f,
                obj.z / 1000.f,
            }, roomMatObj);
            json position = json::array();
            position.push_back(v.x);
            position.push_back(v.y);
            position.push_back(v.z);
            loc["position"] = position;

            Vector4 q = QuaternionTransform(convertRotate(
                obj.alpha, obj.beta, obj.gamma
            ), roomMatObj);
            json rotation = json::array();
            rotation.push_back(q.x);
            rotation.push_back(q.y);
            rotation.push_back(q.z);
            rotation.push_back(q.w);
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