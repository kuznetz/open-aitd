#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <map>

#include "pak/pak.h"
#include "loaders/loaders.h"
#include "extractors/background_extractor.h"
#include "extractors/object_extractor.h"
#include "extractors/floor_extractor.h"
#include "extractors/mask_extractor.h"
#include "extractors/sound_extractor.h"
#include "extractors/model_extractor.h"
#include "life/life_extractor.h"

inline void processStages() {
    char floordir[100];
    char cameradir[100];
    char str[100];
    char str2[100];

    int tmp = 3;
    for (int fl = 0; fl < 8; fl++) {
        //std::filesystem::create_directories("original");
        //std::filesystem::create_directories("backgrounds");
        sprintf(str, "original/ETAGE%02d.PAK", fl);
        auto& curFloor = loadFloorPak(str);

        sprintf(floordir, "data/floor_%02d", fl);
        std::filesystem::create_directories(floordir);
        sprintf(str, "%s/scene", floordir);
        sprintf(str2, "%s.json", str);
        if (!std::filesystem::exists(str2)) {
            saveFloorGLTF(curFloor, str);
        }

        sprintf(str, "original/CAMERA%02d.PAK", fl);
        PakFile camPak(str);
        for (int cam = 0; cam < curFloor.cameras.size(); cam++) {
            //background
            sprintf(cameradir, "%s/camera_%02d", floordir, cam);
            std::filesystem::create_directories(cameradir);
            sprintf(str2, "%s/background.png", cameradir);
            if (!std::filesystem::exists(str2)) {
                auto& data = camPak.readBlock(cam);
                extractBackground(data.data(), str2);
            }

            //overlays
            for (int vw = 0; vw < curFloor.cameras[cam].viewedRoomTable.size(); vw++) {
                auto curVw = &curFloor.cameras[cam].viewedRoomTable[vw];
                for (int ovl = 0; ovl < curVw->overlays_V1.size(); ovl++) {
                    auto polys = &curVw->overlays_V1[ovl].polygons;
                    sprintf(str, "%s/mask_%02d_%02d.png", cameradir, vw, ovl);
                    if (!std::filesystem::exists(str)) {
                        extractV1Mask(polys, str);
                    }
                }
            }
        }
    }
}

void processModels() {
    PakFile bodyPak("original/LISTBODY.PAK");
    PakFile body2Pak("original/LISTBOD2.PAK");
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

        string str = string("data/models/") + to_string(i);
        if (!std::filesystem::exists(str)) {
            auto& model = loadModel((char*)testBody.data(), h.uncompressedSize);
            std::filesystem::create_directories(str);
            saveModelGLTF(model, {}, string(str));
        }

        if (!bodiesEq) {
            str = string("data/models/") + to_string(i) + "_alt";
            if (!std::filesystem::exists(str)) {
                auto& model2 = loadModel((char*)testBody2.data(), h2.uncompressedSize);
                std::filesystem::create_directories(str);
                saveModelGLTF(model2, {}, string(str));
            }
        }
    }
}

void processScripts() {
    auto& gameObjs = loadGameObjects("original/OBJETS.ITD");

    PakFile lifePak("original/LISTLIFE.PAK");
    vector<LifeInstructions> allLifes;
    vector<vector<LifeNode>> lifesNodes;
    //int i2 = 514;
    for (int i = 0; i < lifePak.headers.size(); i++)
    {
        auto& data = lifePak.readBlock(i);
        auto& life = loadLife(data.data(), lifePak.headers[i].uncompressedSize);
        allLifes.push_back(life);

    }

    //for allLifes
    //LifeInstructionsP lifep;
    //auto lifeData = life.data();
    //for (int j = 0; j < life.size(); j++) {
    //    lifep.push_back(lifeData + j);
    //}
    //auto& nodes = lifeOptimize(lifep);
    //lifesNodes.push_back(nodes);

}

void processSounds() {
    PakFile soundsPak("original/LISTSAMP.PAK");
    for (int i2 = 0; i2 < soundsPak.headers.size(); i2++) {
        auto& data = soundsPak.readBlock(i2);
        auto& voc = loadVoc((char*)data.data(), soundsPak.headers[i2].uncompressedSize);
        auto s = string("data/sounds");
        std::filesystem::create_directories(s);
        s += "/" + to_string(i2) + ".wav";
        writeWav(&voc, s);
    }
}

void extractAllData() {
    //processStages();
    //processModels();
    //extractVars("original", "data/vars.json");

    //PakFile animPak("original/LISTANIM.PAK");
    //vector<Animation> anims;
    //for (int i = 0; i < animPak.headers.size(); i++)
    //{
    //    auto& data = animPak.readBlock(i);
    //    auto& anim = loadAnimation(data.data());
    //    anims.push_back(anim);
    //}

    PakFile animPak("original/LISTANIM.PAK");
    Animation anim1;
    Animation anim2;
    {
        auto& data = animPak.readBlock(0);
        anim1 = loadAnimation(data.data());
    }
    {
        auto& data = animPak.readBlock(1);
        anim2 = loadAnimation(data.data());
    }
    vector<Animation*> chestAni = {&anim1};

    PakFile bodyPak("original/LISTBODY.PAK");
    auto h = bodyPak.headers[1];
    auto& testBody = bodyPak.readBlock(1);
    auto& model = loadModel((char*)testBody.data(), h.uncompressedSize);
    saveModelGLTF(model, { &anim1 }, "data/test");
}