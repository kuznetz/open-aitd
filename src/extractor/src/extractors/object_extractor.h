#pragma once
#include "../structs/int_types.h"
#include "../structs/game_objects.h"
#include "../../../names-decode/name_decoders.hpp"
#include "../../../raylib-cpp/raylib-cpp.h"
#include <stdio.h>
#include <string>
#include <fstream>
#include <iomanip>

#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>

namespace AITDExtractor {

    using namespace std;
    using namespace raylib;
    using json = nlohmann::json;

    inline const Matrix roomMatObj = MatrixRotateX(PI);

    inline Vector3 convertRotate(int alpha, int beta, int gamma) {
        // float a = (alpha+512) * 2*PI/1024;
        // float b = -beta * 2*PI/1024;
        // float c = -gamma * 2*PI/1024;
        float a = alpha * 2*PI/1024;
        float b = (512+beta) * 2*PI/1024;
        float c = gamma * 2*PI/1024;
        return {a, b, c};
    }

    inline void extractGameObjects(vector <gameObjectStruct> objects, string josnTo, const openAITD::NameDecoders& nameDec) {
        json outJson = json::array();
        for (int i = 0; i < objects.size(); i++)
        {
            auto& obj = objects[i];
            json objJson = json::object();
            objJson["id"] = i; //obj.id
            objJson["name"] = nameDec.obj.getName(i);

            bool hasAngle = obj.alpha || obj.beta || obj.gamma;

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
            else if (obj.stageId != -1 || hasAngle) {
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

                Vector3 rot = convertRotate(
                    obj.alpha, obj.beta, obj.gamma
                );
                json rotation = json::array();
                rotation.push_back(rot.x);
                rotation.push_back(rot.y);
                rotation.push_back(rot.z);
                loc["rotation"] = rotation;

                json rotationOrig = json::array();
                rotationOrig.push_back(obj.alpha);
                rotationOrig.push_back(obj.beta);
                rotationOrig.push_back(obj.gamma);
                loc["rotOrig"] = rotationOrig;

                loc["stageId"] = obj.stageId;
                loc["roomId"] = obj.roomId;
            }

            //model
            if (obj.body != -1) {
                objJson["model"] = json::object();
                objJson["model"]["id"] = obj.body;
                objJson["model"]["animId"] = obj.anim;
                objJson["model"]["animType"] = obj.animType; //Autoplay?
                objJson["model"]["animInfo"] = obj.animInfo;
                objJson["model"]["boundsType"] = (obj.boundsType != 0)? obj.boundsType: 2;
            }

            //inventory
            if (obj.inventoryBody != -1) {
                objJson["invItem"] = json::object();
                objJson["invItem"]["model"] = obj.inventoryBody;
                objJson["invItem"]["name"] = obj.inventoryName;
                objJson["invItem"]["life"] = obj.inventoryLife;
                objJson["invItem"]["flags"] = obj.invFlagsBits;
            }

            objJson["track"] = json::object();
            objJson["track"]["id"] = obj.trackNumber;
            objJson["track"]["mode"] = obj.trackMode;
            objJson["track"]["position"] = obj.positionInTrack;

            objJson["lifeMode"] = obj.lifeMode;
            objJson["life"] = obj.life;
            objJson["stageLife"] = obj.stageLife;

            objJson["flags"] = obj.flagsBits;

            outJson.push_back(objJson);
        }

        std::ofstream o(josnTo.c_str());
        o << std::setw(2) << outJson << std::endl;
    }

}