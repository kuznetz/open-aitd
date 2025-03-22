#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <map>

#include "pak/pak.h"
#include "loaders/loaders.h"
#include "life/life_extractor.h"
#include "res_using/life_using.h"
#include "res_using/body_using.h"
#include "res_using/anim_using.h"

struct ModelAinmsStruct {
    bool hasBones = false;
    vector<int> anims;
};

void linkAnimation() {
    auto& gameObjs = loadGameObjects("original/OBJETS.ITD");

    PakFile lifePak("original/LISTLIFE.PAK");
    vector<LifeInstructions> allLifes;
    //int i2 = 514;
    for (int i = 0; i < lifePak.headers.size(); i++)
    {
        auto& data = lifePak.readBlock(i);
        auto& life = loadLife(data.data(), lifePak.headers[i].uncompressedSize);
        allLifes.push_back(life);
    }

    PakFile bodyPak("original/LISTBODY.PAK");
    PakFile body2Pak("original/LISTBOD2.PAK");
    vector<ModelAinmsStruct> modelAnims(bodyPak.headers.size());

    //int bodyId = 2;
    for (int i = 0; i < bodyPak.headers.size(); i++)
    {
        auto h = bodyPak.headers[i];
        auto& testBody = bodyPak.readBlock(i);

        auto h2 = body2Pak.headers[i];
        auto& testBody2 = body2Pak.readBlock(i);

        bool bodiesEq = h2.uncompressedSize == h.uncompressedSize;
        if (bodiesEq) {
            bodiesEq = !memcmp(testBody.data(), testBody2.data(), testBody.size());
        }

        auto& model = loadModel((char*)testBody.data(), h.uncompressedSize);
        modelAnims[i].hasBones = model.bones.size();

        //if (!bodiesEq) {
        //    str = string("data/models/") + to_string(i) + "_alt";
        //    if (!std::filesystem::exists(str)) {
        //        auto& model2 = loadModel((char*)testBody2.data(), h2.uncompressedSize);
        //        std::filesystem::create_directories(str);
        //        saveModelGLTF(model2, string(str));
        //    }
        //}
    }

    LifeUsing lifeUse(&allLifes, &gameObjs);
    auto& lifeUse2 = lifeUse.result;

    BodyUsing bodyUse(&allLifes, &gameObjs, &lifeUse2);
    auto& bodyUse2 = bodyUse.result;

    AnimUsing animUse(&allLifes, &gameObjs, &lifeUse2);
    auto& animUse2 = animUse.result;

    for (int bodyId = 0; bodyId < bodyPak.headers.size(); bodyId++) {
        vector<int> bodyObjs;
        for (auto it = bodyUse2.begin(); it != bodyUse2.end(); it++) {
            auto& objId = it->first;
            auto& bodyIds = it->second;
            if (std::find(bodyIds.begin(), bodyIds.end(), bodyId) == bodyIds.end()) continue;
            bodyObjs.push_back(objId);

        }
    }

}