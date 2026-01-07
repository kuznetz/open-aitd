#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <map>

#include "pak/pak.h"
#include "loaders/loaders.h"

#include "./extractor_path.h"
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

Matrix roomMatrix;

namespace AITDExtractor {

    vector <gameObjectStruct> gameObjs;
    vector <floorStruct> stages;

    inline string intToStrWidth(int number, int width) {
        std::stringstream ss;
        ss << std::setw(width) << std::setfill('0') << number;
        return ss.str();
    }

    inline void processStages() {
        string floordir;
        string cameradir;
        string str;

        int tmp = 3;
        for (int fl = 0; fl < stages.size(); fl++) {
            auto& curFloor = stages[fl];
            floordir = ExtractorPath::data + "/stages/" + to_string(fl);

            std::filesystem::create_directories(floordir);
            floordir = floordir;
            if (!std::filesystem::exists(floordir + "/stage.gltf")) {
                saveFloorGLTF(fl, curFloor, gameObjs, floordir);
            }

            string origCamera = ExtractorPath::original + "/CAMERA" + intToStrWidth(fl,2) + ".PAK";
            PakFile camPak(origCamera);
            for (int cam = 0; cam < curFloor.cameras.size(); cam++) {
                //background
                cameradir = floordir + "/camera_" + to_string(cam);
                std::filesystem::create_directories(cameradir);
                str = cameradir + "/background.png";
                if (!std::filesystem::exists(str)) {
                    auto& data = camPak.readBlock(cam);
                    extractBackground(data.data(), str.c_str());
                }

                //overlays
                for (int vw = 0; vw < curFloor.cameras[cam].viewedRoomTable.size(); vw++) {
                    auto curVw = &curFloor.cameras[cam].viewedRoomTable[vw];
                    for (int ovl = 0; ovl < curVw->overlays_V1.size(); ovl++) {
                        auto polys = &curVw->overlays_V1[ovl].polygons;
                        str = cameradir + "/mask_"+to_string(vw)+"_"+to_string(ovl)+".png";
                        if (!std::filesystem::exists(str)) {
                            extractV1Mask(polys, str.c_str());
                        }
                    }
                }
            }
        }
    }

    void processModels() {
        std::ifstream ifs("newdata/animation_links.json");
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

    void processTestScript() {
        PakFile lifePak("original/LISTLIFE.PAK");
        vector<LifeInstructions> allLifes;
        //int i2 = 514;
        for (int i = 0; i < lifePak.headers.size(); i++)
        {
            auto& data = lifePak.readBlock(i);
            auto& life = loadLife(data.data(), lifePak.headers[i].uncompressedSize);
            allLifes.push_back(life);

        }

        int n = 0;
        LifeInstructionsP lifep;
        auto& life = allLifes[n];
        auto lifeData = life.data();
        for (int j = 0; j < life.size(); j++) {
            lifep.push_back(lifeData + j);
        }
        auto& nodes = lifeOptimize(lifep);

        ofstream out(string("data/life_")+to_string(n)+".lua", ios::trunc | ios::out);
        out << "function life_" << n << "(obj)\n";
        writeLifeNodes(out, 1, nodes);
        out << "end\n\n";
        out.close();

        /*ofstream out("data/all_life.lua", ios::trunc | ios::out);
        for (int j = 0; j < lifesNodes.size(); j++)
        {
            out << "function life_" << j << "(obj)\n";
            writeLifeNodes(out, 1, lifesNodes[j]);
            out << "end\n\n";
        }
        out.close();*/
    }

    void processScripts() {
        if (!std::filesystem::exists("data/vars.json")) {
            extractVars("original", "data/vars.json");
        }

        PakFile lifePak("original/LISTLIFE.PAK");
        vector<LifeInstructions> allLifes;
        //int i2 = 514;
        for (int i = 0; i < lifePak.headers.size(); i++)
        {
            auto& data = lifePak.readBlock(i);
            auto& life = loadLife(data.data(), lifePak.headers[i].uncompressedSize);
            allLifes.push_back(life);

        }

        //for allLifes
        vector<vector<LifeNode>> lifesNodes;
        LifeInstructionsP lifep;
        for (int i = 0; i < allLifes.size(); i++) {
            auto& life = allLifes[i];
            auto lifeData = life.data();
            lifep.clear();
            for (int j = 0; j < life.size(); j++) {
                lifep.push_back(lifeData + j);
            }
            auto& nodes = lifeOptimize(lifep);
            lifesNodes.push_back(nodes);
        }

        const char* filename = "data/scripts.lua";
        if (!std::filesystem::exists(filename)) {
            ofstream out(filename, ios::trunc | ios::out);
            for (int j = 0; j < lifesNodes.size(); j++)
            {
                out << "function life_" << j << "(obj)\n";
                writeLifeNodes(out, 1, lifesNodes[j]);
                out << "end\n\n";
            }
            out.flush();
            out.close();
        }

    }

    void processTracks() {
        PakFile trackPak("original/LISTTRAK.PAK");
        auto dir = string("data/tracks");
        std::filesystem::create_directories(dir);
        for (int i = 0; i < trackPak.headers.size(); i++)
        {
            auto& data = trackPak.readBlock(i);
            auto& track = loadTrack(data.data(), trackPak.headers[i].uncompressedSize);
            auto s = dir + "/" + to_string(i) + ".json";
            if (std::filesystem::exists(s)) continue;
            extractTrack(track, s);
            //allTracks.push_back(track);
        }
    }

    void processSounds() {
        PakFile soundsPak("original/LISTSAMP.PAK");
        auto dir = string("data/sounds");
        std::filesystem::create_directories(dir);
        for (int i2 = 0; i2 < soundsPak.headers.size(); i2++) {
            auto& data = soundsPak.readBlock(i2);
            auto& voc = loadVoc((char*)data.data(), soundsPak.headers[i2].uncompressedSize);
            auto s = dir + "/" + to_string(i2) + ".wav";
            if (std::filesystem::exists(s)) continue;
            writeWav(&voc, s);
        }
    }

    void proocessMusics() {

    }

    void processTexts() {
        string dirname = "data/texts";
        if (!std::filesystem::exists(dirname)) {
            std::filesystem::create_directories(dirname);
            
            PakFile textsPak("original/ENGLISH.PAK");
            auto& data = textsPak.readBlock(0);
            //data.resize(data.size() + 1);
            data[data.size() - 1] = '\0';
            for (int i = 0; i < data.size(); i++) {
                if (data[i] == 13) data[i] = 10;
            }
            string filename = dirname + "/english.txt";
            ofstream out(filename.c_str(), ios::trunc | ios::out);
            out << (char*)data.data() << endl;
            out.close();
        }
    }

    void processPicture(PakFile& picsPak, int id, string outPath) {
        if (!std::filesystem::exists(outPath)) {
            auto& data = picsPak.readBlock(id);
            extractBackground(data.data(), outPath.c_str());
        }
    }

    void processPictures() {
        PakFile picsPak("original/ITD_RESS.PAK");

        string dir = "data/pictures";
        std::filesystem::create_directories(dir);
        processPicture(picsPak, 6, dir + "/6.png");
        processPicture(picsPak, 7, dir + "/7.png");
        processPicture(picsPak, 8, dir + "/8.png");
        processPicture(picsPak, 11, dir + "/11.png");
        processPicture(picsPak, 12, dir + "/12.png");

        dir = "data/characters";
        std::filesystem::create_directories(dir);
        processPicture(picsPak, 10, dir + "/0.png");
        processPicture(picsPak, 14, dir + "/1.png");

        dir = "data/add_bg";
        std::filesystem::create_directories(dir);
        processPicture(picsPak, 15, dir + "/15.png");
        processPicture(picsPak, 16, dir + "/16.png");
        processPicture(picsPak, 17, dir + "/17.png");
        processPicture(picsPak, 18, dir + "/18.png");
        processPicture(picsPak, 19, dir + "/19.png");
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
        //roomMatrix = MatrixMultiply(MatrixRotateX(PI), MatrixRotateY(PI));
        roomMatrix = MatrixRotateX(PI);

        //dumpInstructions("instr.txt");

        processTexts();

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
        //processTestScript();
        processScripts();
        processTracks();
        processSounds();
        processPictures();


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