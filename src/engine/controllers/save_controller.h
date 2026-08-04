#pragma once
#include "../resources/resources.h"
#include "../world/world.h"
#include "life_controller.h"
#include <stdexcept>
#include <filesystem>

using namespace std;

namespace openAITD {

    class SaveSlot {
    public:
        int id;
        string date;
        string location;
    };

    class SaveController {
    public:
        World* world;
        Resources* resources;
        LifeController* life;
        string saveDir = "./saves";

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

        string getCurrentDate() {
            auto now = chrono::system_clock::now();
            auto in_time_t = chrono::system_clock::to_time_t(now);
            tm bt;
            #ifdef _WIN32
                localtime_s(&bt, &in_time_t);
            #else
                localtime_r(&in_time_t, &bt);
            #endif
            ostringstream oss;
            oss << put_time(&bt, "%Y-%m-%d %H:%M");
            return oss.str();
        }

        void save(int slot) {
            auto& cVars = world->cVars;
            json outJson = json::object();

            if (world->followTarget) {
                outJson["follow"] = world->followTarget->id;
            }
            if (world->inHandObj) {
                outJson["inHand"] = world->inHandObj->id;
            }
            outJson["inDark"] = world->inDark;

            outJson["vars"] = json::array();
            for (int i = 0; i < world->vars.size(); i++) {
                outJson["vars"].push_back(world->vars[i]);
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
                outObj["location"]["stageId"] = gobj.getStageId();
                outObj["location"]["roomId"] = gobj.getRoomId();
                outObj["location"]["position"] = vector2json(gobj.getPosition());
                const auto& r = gobj.getOrigRotation();
                outObj["location"]["rotation2"] = json::array();
                outObj["location"]["rotation2"].push_back(r.x);
                outObj["location"]["rotation2"].push_back(r.y);
                outObj["location"]["rotation2"].push_back(r.z);
                // auto& r2 = gobj.location.rotOrig;
                // outObj["location"]["rotOrig"] = json::array();
                // outObj["location"]["rotOrig"].push_back(r2.x);
                // outObj["location"]["rotOrig"].push_back(r2.y);
                // outObj["location"]["rotOrig"].push_back(r2.z);

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
                outObj["hitObjectDamage"] = gobj.physics.hitObjectDamage;

                outJson["objects"].push_back(outObj);
            }

            string slotDir = saveDir + "/" + to_string(slot);
            filesystem::create_directories(slotDir);
            string path = slotDir + "/data.json";
            std::ofstream o(path);
            o << std::setw(2) << outJson << std::endl;

            json outSlotJson = json::object();
            outSlotJson["date"] = getCurrentDate();
            outSlotJson["location"] = "Stage "+to_string(world->curStageId)+" Room "+to_string(world->curRoomId);
            path = slotDir + "/slot.json";
            std::ofstream o2(path);
            o2 << std::setw(2) << outSlotJson << std::endl;

            path = slotDir + "/screen.png";
            resources->screen.saveScreenshot(path, 640, 480);
        }

        void load(int slot) {
            string path = this->saveDir + "/" + to_string(slot) + "/data.json";
            try {
                ifstream ifs(path);
                json inJson = json::parse(ifs);

                world->vars.resize(inJson["vars"].size());
                for (int i = 0; i < world->vars.size(); i++) {
                    world->vars[i] = inJson["vars"][i];
                }

                world->cVars.resize(inJson["cVars"].size());
                for (int i = 0; i < world->cVars.size(); i++) {
                    world->cVars[i] = inJson["cVars"][i];
                }

                world->gobjects.clear();
                world->gobjects.reserve(inJson["objects"].size());
                for (int i = 0; i < inJson["objects"].size(); i++) {
                    auto& inObj = inJson["objects"][i];
                    auto& gobj = world->gobjects.emplace_back(*resources);
                    gobj.id = i;
                    
                    gobj.setStage(
                        inObj["location"]["stageId"],
                        inObj["location"]["roomId"],
                        json2vector(inObj["location"]["position"])
                    );
                    auto& r = inObj["location"]["rotation2"];
                    gobj.setOrigRotation({ r[0], r[1], r[2] });
                    // auto& r2 = inObj["location"]["rotOrig"];
                    // gobj.location.rotOrig = { r2[0], r2[1], r2[2] };

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
                    gobj.physics.hitObjectDamage = inObj["hitObjectDamage"];
                }

                world->inventory.resize(inJson["inventory"].size());
                for (int i = 0; i < world->inventory.size(); i++) {
                    int objId = inJson["inventory"][i];
                    auto obj  = &world->gobjects[objId];
                    world->inventory[i] = obj;
                }

                int inHand = inJson["inHand"].get<int>();
                world->inHandObj = &world->gobjects[inHand];
                int followTarget = inJson["follow"].get<int>();
                world->followTarget = &world->gobjects[followTarget];
                world->inDark = inJson["inDark"];

                auto foll = world->followTarget;
                world->setCurStage(foll->getStageId(), foll->getRoomId());
                
                world->resources->backgrounds.setIsAltBackgrounds(!!world->cVars[12]);

            } catch(exception e) {
                string message = "Error loading : " + path;
                throw exception(message.c_str());
            }
        }

        std::vector<SaveSlot> listSlots() {
            std::vector<SaveSlot> result;
            if (!std::filesystem::exists(saveDir)) {
                return result;
            }

            for (const auto& entry : std::filesystem::directory_iterator(saveDir)) {
                if (!entry.is_directory()) continue;

                std::string dirName = entry.path().filename().string();
                int slotId = 0;
                try {
                    slotId = std::stoi(dirName);
                } catch (...) {
                    continue; // not a numeric directory name → skip
                }

                std::string slotJsonPath = entry.path().string() + "/slot.json";
                if (!std::filesystem::exists(slotJsonPath)) continue;

                try {
                    std::ifstream ifs(slotJsonPath);
                    json slotJson = json::parse(ifs);
                    SaveSlot slot;
                    slot.id = slotId;
                    slot.date = slotJson.value("date", "");
                    slot.location = slotJson.value("location", "");
                    result.push_back(slot);
                } catch (...) {
                    // skip slots with malformed or missing slot.json
                    continue;
                }
            }

            // optional: sort by slot id for consistent order
            std::sort(result.begin(), result.end(),
                    [](const SaveSlot& a, const SaveSlot& b) {
                        return a.id < b.id;
                    });

            return result;
        }

    };

}