#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <map>

#include "pak/pak.h"
#include "loaders/loaders.h"
#include "extractors/background_extractor.h"
#include "extractors/object_extractor.h"
//#include "extractors/floor_extractor.h"
#include "extractors/floor_extractor_2.h"
#include "extractors/mask_extractor.h"
#include "extractors/sound_extractor.h"
#include "extractors/model_extractor.h"
//#include "extractors/skeleton_extractor.h"
#include "extractors/track_extractor.h"
#include "extractors/vars_extractor.h"
#include "life/life_extractor.h"

namespace AITDExtractor {
    vector <gameObjectStruct> gameObjs;
    vector <floorStruct> stages;

    inline void processStages() {
        char floordir[100];
        char cameradir[100];
        char str[100];
        char str2[100];

        int tmp = 3;
        for (int fl = 0; fl < stages.size(); fl++) {
            auto& curFloor = stages[fl];

            sprintf(floordir, "data/stages/%d", fl);
            std::filesystem::create_directories(floordir);
            sprintf(str, "%s/stage", floordir);
            sprintf(str2, "%s.json", str);
            if (!std::filesystem::exists(str2)) {
                saveFloorGLTF(fl, curFloor, gameObjs, str);
            }

            sprintf(str, "original/CAMERA%02d.PAK", fl);
            PakFile camPak(str);
            for (int cam = 0; cam < curFloor.cameras.size(); cam++) {
                //background
                sprintf(cameradir, "%s/camera_%d", floordir, cam);
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
                        sprintf(str, "%s/mask_%d_%d.png", cameradir, vw, ovl);
                        if (!std::filesystem::exists(str)) {
                            extractV1Mask(polys, str);
                        }
                    }
                }
            }
        }
    }

    void processModels() {
        std::ifstream ifs("data/animation_links.json");
        json animLinks = json::parse(ifs);

        PakFile animPak("original/LISTANIM.PAK");
        PakFile anim2Pak("original/LISTANI2.PAK");
        vector<Animation> anims;
        vector<Animation> anims2;
        for (int i = 0; i < animPak.headers.size(); i++) {
            auto& block = animPak.readBlock(i);
            anims.push_back(loadAnimation(i, block.data()));
            auto& block2 = anim2Pak.readBlock(i);
            anims2.push_back(loadAnimation(i, block2.data()));
        }

        PakFile bodyPak("original/LISTBODY.PAK");
        PakFile body2Pak("original/LISTBOD2.PAK");
        //int bodyId = 2;
        for (int i = 0; i < bodyPak.headers.size(); i++)
        {
            auto h = bodyPak.headers[i];
            auto& testBody = bodyPak.readBlock(i);

            auto h2 = body2Pak.headers[i];
            auto& testBody2 = body2Pak.readBlock(i);

            bool altBody = h2.uncompressedSize != h.uncompressedSize;
            if (!altBody) {
                altBody = !!memcmp(testBody.data(), testBody2.data(), testBody.size());
            }

            std::vector<int> curAnims;
            for (int j = 0; j < animLinks.size(); j++) {
                int modelId = animLinks[j]["modelId"];
                if (modelId == i) {
                    curAnims = animLinks[j]["anims"].get<std::vector<int>>();
                    break;
                }
            }

            string outDir = string("data/models/") + to_string(i);
            if (!std::filesystem::exists(outDir)) {
                auto& model = loadModel((char*)testBody.data(), h.uncompressedSize);
                vector<Animation*> animations;
                for (int j = 0; j < curAnims.size(); j++) {
                    animations.push_back(&anims[curAnims[j]]);
                }
                saveModelGLTF(model, animations, string(outDir));
            }

            if (altBody) {
                outDir = string("data/models/") + to_string(i) + "_alt";
                if (!std::filesystem::exists(outDir)) {
                    auto& model2 = loadModel((char*)testBody2.data(), h2.uncompressedSize);
                    vector<Animation*> animations;
                    for (int j = 0; j < curAnims.size(); j++) {
                        animations.push_back(&anims2[curAnims[j]]);
                    }
                    saveModelGLTF(model2, animations, string(outDir));
                }
            }
        }
    }

    void processScripts() {
        extractVars("original", "data/vars.json");

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

    void processTracks() {
        PakFile trackPak("original/LISTTRAK.PAK");
        for (int i = 0; i < trackPak.headers.size(); i++)
        {
            auto& data = trackPak.readBlock(i);
            auto& track = loadTrack(data.data(), trackPak.headers[i].uncompressedSize);
            auto s = string("data/tracks");
            std::filesystem::create_directories(s);
            extractTrack(track, s + "/" + to_string(i) + ".json");
            //allTracks.push_back(track);
        }
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

    void animTest() {
        //Quaternion.identity
        int modelId = 12;
        int animId = 68;
        //int modelId = 11;
        //int animId = 11;

        PakFile animPak("original/LISTANIM.PAK");
        Animation anim1;
        Animation anim2;
        {
            auto& data = animPak.readBlock(animId);
            anim1 = loadAnimation(animId, data.data());
        }
        {
            auto& data = animPak.readBlock(16);
            anim2 = loadAnimation(0, data.data());
        }

        PakFile bodyPak("original/LISTBODY.PAK");
        auto h = bodyPak.headers[modelId];
        auto& testBody = bodyPak.readBlock(modelId);
        auto& model = loadModel((char*)testBody.data(), h.uncompressedSize);
        saveModelGLTF(model, { &anim1 }, "data/test");
    }

    void extractAllData() {
        gameObjs = loadGameObjects("original/OBJETS.ITD");
        if (!std::filesystem::exists("data/objects.json")) {
            extractGameObjects(gameObjs, "data/objects.json");
        }
        for (int fl = 0; fl < 8; fl++) {
            char str[100];
            sprintf(str, "original/ETAGE%02d.PAK", fl);
            stages.push_back(loadFloorPak(str));
        }

        processStages();
        processModels();
        processScripts();
        processTracks();
        processSounds();


        //PakFile animPak("original/LISTANIM.PAK");
        //vector<Animation> anims;
        //for (int j = 0; j < animPak.headers.size(); j++)
        //{
        //    auto& data = animPak.readBlock(j);
        //    auto& anim = loadAnimation(data.data());
        //    anims.push_back(anim);
        //}

    }

}