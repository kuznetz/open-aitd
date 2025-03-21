#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <map>

#include "pak/pak.h"
#include "loaders/floor_loader.h"
#include "extractors/background_extractor.h"
#include "extractors/game_objects.h"
#include "extractors/floor_extractor.h"
#include "extractors/mask_extractor.h"
#include "extractors/sound_extractor.h"
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


typedef map<int, vector<int>> AnimationsMap;

void CollectLifeAnims(AnimationsMap& aniMap, LifeInstruction& inst) {
    int body = -1;
    int anim = -1;
    switch (inst.Type->Type)
    {
    //THROW 1 2 7
    //HIT 1 2 6
    //ANIM_ALL_ONCE - 
    //ANIM_MOVE - payer move
    //ANIM_SOUND
    //ANIM_RESET ??
    //ANIM_HYBRIDE_ONCE ??
    //ANIM_HYBRIDE_REPEAT ??

    case LifeEnum::ANIM_ONCE:
        body = inst.arguments[0].constVal;
        anim = inst.arguments[1].constVal;
        break;
    default:
        break;
    }
    if (body != -1 && anim != -1) {
        auto& ani = aniMap[body];
        ani.push_back(anim);
    }
}

void extractAllData() {
    //processStages();

    map<int,vector<int>> usedAnimations;
    map<int, vector<int>> objectLifes;

    //writeWAVDemo();
    PakFile soundsPak("original/LISTSAMP.PAK");
    for (int i = 0; i < soundsPak.headers.size(); i++) {
        auto& data = soundsPak.readBlock(i);
        auto& voc = loadVoc((char*)data.data(), soundsPak.headers[i].uncompressedSize);
        auto s = string("data/sounds");
        std::filesystem::create_directories(s);
        s += "/" + to_string(i) + ".wav";
        writeWav(&voc, s);
    }

    auto& gameObjs = loadGameObjects("original/OBJETS.ITD");
    for (int i = 0; i < gameObjs.size(); i++) {
        if (gameObjs[i].body != -1 && gameObjs[i].anim != -1) {
            //auto& ani = usedAnimations[gameObjs[i].body];
            //ani.push_back(gameObjs[i].anim);
        }
        if (gameObjs[i].life != -1) {
            auto& ani = objectLifes[i];
            ani.push_back(gameObjs[i].life);
        }
    }

    PakFile lifePak("original/LISTLIFE.PAK");
    vector<LifeInstructions> allLifes;
    vector<vector<LifeNode>> lifesNodes;
    //int i = 514;
    for (int i = 0; i < lifePak.headers.size(); i++)
    {
        auto& data = lifePak.readBlock(i);
        auto& life = loadLife(data.data(), lifePak.headers[i].uncompressedSize);
        
        for (int j = 0; j < life.size(); j++) {
            CollectLifeAnims(usedAnimations, life[j]);

        }

        //allLifes.push_back(life);
        //LifeInstructionsP lifep;
        //auto lifeData = life.data();
        //for (int j = 0; j < life.size(); j++) {
        //    lifep.push_back(lifeData + j);
        //}
        //auto& nodes = lifeOptimize(lifep);
        //lifesNodes.push_back(nodes);
    }
    printf("done.");
    


    //extractLife("original/LISTLIFE.PAK", "data/scripts.lua");


    //"data/objects.json"

    //extractVars("original", "data/vars.json");


    /*char* srcFN2 = "original/LISTBOD2.PAK";
    int filesNum2 = PAK_getNumFiles(srcFN2);
    for (int i = 0; i < filesNum2; i++)
    {
        int size = getPakSize(srcFN2, i);
        char* testBody = loadPak(srcFN2, i);
        loadModel(testBody, size);
        delete testBody;
    }*/



    //PakFile lifePak("original/LISTBODY.PAK");
    ////int i = 1;
    //for ( int i = 0; i < lifePak.headers.size(); i++ )
    //{
    //    auto h = lifePak.headers[i];
    //    auto& testBody = lifePak.readBlock(i);
    //    auto& model = loadModel((char*)testBody.data(), h.uncompressedSize);
    //    sprintf(str, "data/models/LISTBODY_%d", i);
    //    std::filesystem::create_directories(str);
    //    saveModelGLTF(model, string(str));
    //}
}