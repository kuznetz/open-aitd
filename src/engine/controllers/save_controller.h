#pragma once
#include "../resources/resources.h"
#include "../world/world.h"
#include "life_controller.h"

using namespace std;

namespace openAITD {

    class SaveController {
    public:
        World* world;
        Resources* resources;
        LifeController* life;

        SaveController(World* world, LifeController* life) {
            this->world = world;
            this->resources = world->resources;
            this->life = life;
            
        }

        json vector2json(Vector3 vec) {
            json r = json::array();
            r.push_back(vec.x);
            r.push_back(vec.y);
            r.push_back(vec.z);
            return r;
        }

        Vector3 json2vector(json vec) {
            return { vec[0], vec[1], vec[2] };
        }

        void save(int slot) {
            auto& cVars = world->cVars;

            json outJson = json::object();

            outJson["follow"] = world->followTarget->id;
            outJson["inHand"] = world->inHandObj->id;

            outJson["vars"] = json::array();
            auto& vars = life->dumpVars();
            for (int i = 0; i < vars.size(); i++) {
                outJson["vars"].push_back(vars[i]);
            }

            outJson["cVars"] = json::array();
            for (int i = 0; i < world->cVars.size(); i++) {
                outJson["cVars"].push_back(world->cVars[i]);
            }

            outJson["inventory"] = json::array();
            for (int i = 0; i < world->inventory.size(); i++) {
                outJson["inventory"].push_back(world->inventory[i]->id);
            }

            outJson["objects"] = json::array();
            for (int i = 0; i < world->gobjects.size(); i++) {
                auto& gobj = world->gobjects[i];
                auto outObj = json::object();

                outObj["location"] = json::object();
                outObj["location"]["stageId"] = gobj.location.stageId;
                outObj["location"]["roomId"] = gobj.location.roomId;
                outObj["location"]["position"] = vector2json(gobj.location.position);
                auto& r = gobj.location.rotation;
                outObj["location"]["rotation"] = json::array();
                outObj["location"]["rotation"].push_back(r.x);
                outObj["location"]["rotation"].push_back(r.y);
                outObj["location"]["rotation"].push_back(r.z);
                outObj["location"]["rotation"].push_back(r.w);

                outObj["animation"] = json::object();
                outObj["animation"]["id"] = gobj.animation.id;
                outObj["animation"]["nextId"] = gobj.animation.nextId;
                outObj["animation"]["flags"] = gobj.animation.flags;

                outObj["track"] = json::object();
                outObj["track"]["id"] = gobj.track.id;
                outObj["track"]["pos"] = gobj.track.pos;
                outObj["track"]["mode"] = gobj.track.mode;

                outObj["invItem"] = json::object();
                outObj["invItem"]["nameId"] = gobj.invItem.nameId;
                outObj["invItem"]["modelId"] = gobj.invItem.modelId;
                outObj["invItem"]["lifeId"] = gobj.invItem.lifeId;
                outObj["invItem"]["flags"] = gobj.invItem.flags;
                
                outObj["modelId"] = gobj.modelId;
                outObj["boundsType"] = gobj.boundsType;
                outObj["flags"] = gobj.flags;
                outObj["stageLifeId"] = gobj.stageLifeId;
                outObj["lifeId"] = gobj.lifeId;
                outObj["lifeMode"] = gobj.lifeMode;
                outObj["chrono"] = world->chrono - gobj.chrono;

                outJson["objects"].push_back(outObj);
            }

            string path = "saves/" + to_string(slot) + ".json";
            std::ofstream o(path);
            o << std::setw(2) << outJson << std::endl;
        }

        void load(int slot) {
            string path = "saves/" + to_string(slot) + ".json";
            ifstream ifs(path);
            json inJson = json::parse(ifs);

            world->vars.resize(inJson["vars"].size());
            for (int i = 0; i < world->vars.size(); i++) {
                world->vars[i] = inJson["vars"][i];
            }
            life->reloadVars();

            world->cVars.resize(inJson["cVars"].size());
            for (int i = 0; i < world->cVars.size(); i++) {
                world->cVars[i] = inJson["cVars"][i];
            }

            world->gobjects.resize(inJson["objects"].size());
            for (int i = 0; i < world->gobjects.size(); i++) {
                auto& inObj = inJson["objects"][i];
                auto& gobj = world->gobjects[i];
                gobj.id = i;
                
                gobj.location.stageId = inObj["location"]["stageId"];
                gobj.location.roomId = inObj["location"]["roomId"];
                gobj.location.position = json2vector(inObj["location"]["position"]);
                auto& r = inObj["location"]["rotation"];
                gobj.location.rotation = { r[0], r[1], r[2], r[3] };

                gobj.animation.id = inObj["animation"]["id"];
                gobj.animation.nextId = inObj["animation"]["nextId"];
                gobj.animation.flags = inObj["animation"]["flags"];

                gobj.track.id = inObj["track"]["id"];
                gobj.track.pos = inObj["track"]["pos"];
                gobj.track.mode = inObj["track"]["mode"];

                gobj.invItem.nameId = inObj["invItem"]["nameId"];
                gobj.invItem.modelId = inObj["invItem"]["modelId"];
                gobj.invItem.lifeId = inObj["invItem"]["lifeId"];
                gobj.invItem.flags = inObj["invItem"]["flags"];

                gobj.modelId = inObj["modelId"];
                gobj.boundsType = inObj["boundsType"];
                gobj.flags = inObj["flags"];
                gobj.stageLifeId = inObj["stageLifeId"];
                gobj.lifeId = inObj["lifeId"];
                gobj.lifeMode = inObj["lifeMode"];
                gobj.chrono = world->chrono + inObj["chrono"];
            }

            world->inventory.resize(inJson["inventory"].size());
            for (int i = 0; i < world->inventory.size(); i++) {
                world->inventory[i] = &world->gobjects[inJson["inventory"][i]];
            }

            world->inHandObj = &world->gobjects[inJson["inHand"]];
            world->followTarget = &world->gobjects[inJson["follow"]];

            auto foll = world->followTarget;
            world->setCurRoom(foll->location.stageId, foll->location.roomId);
            //followCameraProcess
        }

    };

}