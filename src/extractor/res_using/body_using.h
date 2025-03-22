#pragma once
#include <map>
#include <vector>
#include <algorithm>
#include "../structs/life.h"
#include "../structs/game_objects.h"

/* Calc map with lifeIds using in object */
class BodyUsing {
public:
    map<int, vector<int>> result;
    vector<LifeInstructions>* allLife;
    vector<gameObjectStruct>* gameObjs;
    map<int, vector<int>>* lifeUsing;

	BodyUsing(vector<LifeInstructions>* allLife, vector<gameObjectStruct>* gameObjs, map<int, vector<int>>* lifeUsing) {
        this->allLife = allLife;
        this->gameObjs = gameObjs;
        this->lifeUsing = lifeUsing;
        this->CollectObjectBodies();
        this->CollectInstrs();
    }

    void CollectObjectBodies() {
        for (int i = 0; i < gameObjs->size(); i++) {
            auto& gobj = (*gameObjs)[i];
            if (gobj.body != -1) {
                addNewBody(i, gobj.body);
            }
        }    
    }

    vector<int> ObjectsUsingLife(int lifeId) {
        vector<int> result;
        for (auto it = lifeUsing->begin(); it != lifeUsing->end(); it++) {
            auto& objId = it->first;
            auto& lifeIds = it->second;
            if (std::find(lifeIds.begin(), lifeIds.end(), lifeId) == lifeIds.end()) continue;
            result.push_back(objId);
        }
        return result;
    }

    void CollectInstrs() {
        for (int lifeId = 0; lifeId < allLife->size(); lifeId++) {
            auto& life = (*allLife)[lifeId];
            for (int j = 0; j < life.size(); j++) {
                auto& instr = life[j];
                if (instr.Type->Type != LifeEnum::BODY) continue;
                auto& newBodyid = instr.arguments[0].constVal;
                if (newBodyid == -1) continue;
                if (instr.Actor == -1) {
                    auto& objIds = ObjectsUsingLife(lifeId);
                    for (int k = 0; k < objIds.size(); k++) {
                        addNewBody(objIds[k], newBodyid);
                    }
                }
                else {
                    addNewBody(instr.Actor, newBodyid);
                }
            }
        }
    }

    void addNewBody(int objectId, int bodyId)
    {
        auto& res = result[objectId];
        if (std::find(res.begin(), res.end(), bodyId) != res.end()) return;
        res.push_back(bodyId);
    }

};
