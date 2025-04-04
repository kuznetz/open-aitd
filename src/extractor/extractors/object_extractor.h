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
        objJson["id"] = i;

        if (obj.boundsType == 4) {
            //static object
            objJson["static"] = json::object();
            auto& loc = objJson["static"];
            loc["stageId"] = obj.stageId;
            loc["roomId"] = obj.roomId;
            loc["staticIdx"] = obj.inventoryName;

            if (obj.body != -1) throw new exception("Static has body");
            if (obj.inventoryBody != -1) throw new exception("Static has inventoryBody");
        }
        else if (obj.stageId != -1) {
            //location                       
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

            loc["stageId"] = obj.stageId;
            loc["roomId"] = obj.roomId;
        }

        //model
        if (obj.body != -1) {
            objJson["model"] = json::object();
            objJson["model"]["id"] = obj.body;
            objJson["model"]["animId"] = obj.anim;
            objJson["model"]["animType"] = obj.animType;
            objJson["model"]["animInfo"] = obj.animInfo;
            objJson["model"]["boundsType"] = (obj.boundsType != 0)? obj.boundsType: 2;
        }

        //inventory
        if (obj.inventoryBody != -1) {
            objJson["invItem"] = json::object();
            objJson["invItem"]["ownerId"] = obj.ownerIdx;
            objJson["invItem"]["model"] = obj.inventoryBody;
            objJson["invItem"]["name"] = obj.inventoryName;
            objJson["invItem"]["life"] = obj.inventoryLife;
        }

        objJson["track"] = json::object();
        objJson["track"]["id"] = obj.trackNumber;
        objJson["track"]["mode"] = obj.trackMode;
        objJson["track"]["position"] = obj.positionInTrack;

        objJson["lifeMode"] = obj.lifeMode;
        objJson["life"] = obj.life;
        objJson["stageLife"] = obj.stageLife;

        objJson["flags"] = obj.flags;
        objJson["flags2"] = obj.flags2;

        outJson.push_back(objJson);
    }

    std::ofstream o(josnTo.c_str());
    o << std::setw(2) << outJson << std::endl;
}