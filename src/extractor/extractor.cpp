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


void extractAllData() {
    //processStages();

    map<int,vector<int>> usedAnimations;

    auto& gameObjs = loadGameObjects("original/OBJETS.ITD");
    for (int i = 0; i < gameObjs.size(); i++) {
        if (gameObjs[i].body != -1 && gameObjs[i].anim != -1) {
            auto& ani = usedAnimations[gameObjs[i].body];
            ani.push_back(gameObjs[i].anim);
        }
    }

    PakFile lifePak("original/LISTLIFE.PAK");
    vector<vector<LifeInstruction>> lifes;
    vector<vector<LifeNode>> lifesNodes;
    //int i = 514;
    for (int i = 0; i < lifePak.headers.size(); i++)
    {
        auto& data = lifePak.readBlock(i);
        lifes.push_back(loadLife(data.data(), lifePak.headers[i].uncompressedSize));
        auto& life = lifes.back();

        LifeInstructionsP lifep;
        auto lifeData = life.data();
        for (int j = 0; j < life.size(); j++) {
            lifep.push_back(lifeData + j);
        }
        auto& nodes = lifeOptimize(lifep);
        lifesNodes.push_back(nodes);
    }
    


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