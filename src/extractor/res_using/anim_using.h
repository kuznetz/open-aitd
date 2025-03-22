#pragma once
#include <map>
#include <vector>
#include <algorithm>
#include "../structs/life.h"
#include "../structs/game_objects.h"

/* Calc map with lifeIds using in object */
class AnimUsing {
public:
    map<int, vector<int>> result;
    vector<LifeInstructions>* allLife;
    vector<gameObjectStruct>* gameObjs;
    map<int, vector<int>>* lifeUsing;

    AnimUsing(vector<LifeInstructions>* allLife, vector<gameObjectStruct>* gameObjs, map<int, vector<int>>* lifeUsing) {
        this->allLife = allLife;
        this->gameObjs = gameObjs;
        this->lifeUsing = lifeUsing;
        this->CollectObjectAnims();
        this->CollectInstrs();
    }

    void CollectObjectAnims() {
        for (int i = 0; i < gameObjs->size(); i++) {
            auto& gobj = (*gameObjs)[i];
            if (gobj.anim != -1) {
                addNewAnim(i, gobj.anim);
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
                if (instr.Actor == -1) {
                    auto& objIds = ObjectsUsingLife(lifeId);
                    for (int k = 0; k < objIds.size(); k++) {
                        addAnimsFromInstruct(objIds[k], instr);
                    }
                }
                else {
                    addAnimsFromInstruct(instr.Actor, instr);
                }
            }
        }
    }

    void addAnimsFromInstruct(int objectId, LifeInstruction& instr)
    {
        
        switch (instr.Type->Type)
        {
        //ANIM_RESET ??
        //ANIM_HYBRIDE_ONCE ??
        //ANIM_HYBRIDE_REPEAT ??        
        case LifeEnum::ANIM_ONCE:
            addNewAnim(objectId, instr.arguments[0].constVal);
            addNewAnim(objectId, instr.arguments[1].constVal);
            break;
        case LifeEnum::ANIM_ALL_ONCE:
            addNewAnim(objectId, instr.arguments[0].constVal);
            addNewAnim(objectId, instr.arguments[1].constVal);
            break;
        case LifeEnum::THROW:
            addNewAnim(objectId, instr.arguments[0].constVal);
            addNewAnim(objectId, instr.arguments[6].constVal);
            break;
        case LifeEnum::HIT:
            addNewAnim(objectId, instr.arguments[0].constVal);
            addNewAnim(objectId, instr.arguments[5].constVal);
            break;
        case LifeEnum::ANIM_MOVE:
            addNewAnim(objectId, instr.arguments[0].constVal);
            addNewAnim(objectId, instr.arguments[1].constVal);
            addNewAnim(objectId, instr.arguments[2].constVal);
            addNewAnim(objectId, instr.arguments[3].constVal);
            addNewAnim(objectId, instr.arguments[4].constVal);
            addNewAnim(objectId, instr.arguments[5].constVal);
            addNewAnim(objectId, instr.arguments[6].constVal);
            break;
        default:
            break;
        }
    }


    void addNewAnim(int objectId, int animId)
    {
        if (animId == -1) return;
        auto& res = result[objectId];
        if (std::find(res.begin(), res.end(), animId) != res.end()) return;
        res.push_back(animId);
    }

};
