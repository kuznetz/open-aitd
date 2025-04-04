#pragma once
#include <map>
#include <vector>
#include <algorithm>
#include "../structs/life.h"
#include "../structs/game_objects.h"

/* Calc map with lifes using in object */
class LifeUsing {
public:
    map<int, vector<int>> result;
    vector<LifeInstructions>* allLife;
    vector<gameObjectStruct>* gameObjs;

	LifeUsing(vector<LifeInstructions>* allLife, vector<gameObjectStruct>* gameObjs) {
        this->allLife = allLife;
        this->gameObjs = gameObjs;
        this->CollectObjectLifes();
        this->CollectConstInstrs();
	}

    void CollectObjectLifes() {
        for (int i = 0; i < gameObjs->size(); i++) {
            if ((*gameObjs)[i].life != -1) {
                addNewUse(i, (*gameObjs)[i].life);
            }
        }
    }

    void CollectConstInstrs() {
        for (int i = 0; i < allLife->size(); i++) {
            auto& life = (*allLife)[i];
            for (int j = 0; j < life.size(); j++) {
                auto& instr = life[j];
                if (instr.type->type != LifeEnum::SET_LIFE) continue;
                if (instr.Actor == -1) continue;
                auto& newLifeid = instr.arguments[0].constVal;
                if (newLifeid == -1) continue;
                addNewUse(instr.Actor, newLifeid);
            }
        }
    }

    void CollectContextInstrs(int objectId, int lifeId) {
        auto& life = (*allLife)[lifeId];
        for (int j = 0; j < life.size(); j++) {
            auto& instr = life[j];
            if (instr.type->type != LifeEnum::SET_LIFE) continue;
            if (instr.Actor != -1) continue;
            auto& newLifeid = instr.arguments[0].constVal;
            if (newLifeid == -1) continue;
            addNewUse(objectId, newLifeid);
        }
    }

    void addNewUse(int objectId, int lifeId)
    {
        auto& res = result[objectId];
        if (std::find(res.begin(), res.end(), lifeId) != res.end()) return;
        res.push_back(lifeId);
        CollectContextInstrs(objectId, lifeId);
    }

};

//if ((*gameObjs)[i].body != -1 && (*gameObjs)[i].anim != -1) {
//    //auto& ani = usedAnimations[gameObjs[i].body];
//    //ani.push_back(gameObjs[i].anim);
//}