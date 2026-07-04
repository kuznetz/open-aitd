#pragma once

#include "pak/pak.h"
#include "loaders/loaders.h"
#include "../../names-decode/name_decoders.hpp"

#include "./extractor_path.h"
#include "./extractors/background_extractor.h"
#include "./extractors/object_extractor.h"
//#include "extractors/floor_extractor.h"
#include "./extractors/floor_extractor_2.h"
#include "./extractors/mask_extractor.h"
#include "./extractors/sound_extractor.h"
#include "./extractors/model_extractor.h"
//#include "extractors/skeleton_extractor.h"
#include "./extractors/track_extractor.h"
#include "./extractors/vars_extractor.hpp"

#include "./life/life_writer.hpp"
#include "./life/life_optimizer.h"

#include "./music/music_extractor.h"

#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <string>
#include <sstream>

#include "../include/extractor.h"


namespace AITDExtractor {

    class AITDExtractor : public IAITDExtractor {
    public:
        AITDExtractor();
        void processStages();
        void processModels();
        void processScripts(bool floppy);
        void processTracks();
        void processSounds();
        void processAdlibMusic();
        void processTexts();
        void processPictures();
        void extractAllData(bool floppy);

    private:
        vector<gameObjectStruct> gameObjs;
        openAITD::NameDecoders nameDecoders;
        Matrix roomMatrix;
        vector <floorStruct> stages;
        ResourceLoader resLoader;
        Pallete pallete;
        int modelCount = 0;
        int varCount = 0;

        void processPicture(PakFile& picsPak, int id, string outPath);
        string intToStrWidth(int number, int width);

    };    

    AITDExtractor::AITDExtractor() : resLoader("./original") {
        roomMatrix = MatrixRotateX(PI);        
        nameDecoders.load();

        for (int fl = 0; fl < 8; fl++) {
            this->stages.push_back(resLoader.loadFloor(fl));
        }

        pallete = resLoader.loadPallete();
    }

    void AITDExtractor::processStages() {
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
                    extractBackground(data.data(), str.c_str(), pallete);
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

    void AITDExtractor::processModels() {
        std::ifstream ifs("newdata/animation_links.json");
        json animLinks = json::parse(ifs);

        PakFile animPak("original/LISTANIM.PAK");
        PakFile anim2Pak("original/LISTANI2.PAK");
        vector<Animation> anims;
        vector<Animation> anims2;
        for (int i = 0; i < animPak.headers.size(); i++) {
            auto& block = animPak.readBlock(i);
            auto& anim = resLoader.loadAnimation(block);
            anim.id = i;
            anims.push_back(anim);
            auto& block2 = anim2Pak.readBlock(i);
            auto& anim2 = resLoader.loadAnimation(block2);
            anim2.id = i;
            anims2.push_back(anim2);
        }

        PakFile bodyPak("original/LISTBODY.PAK");
        PakFile body2Pak("original/LISTBOD2.PAK");
        //int bodyId = 2;
        this->modelCount = bodyPak.headers.size();
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

            string outDir = string("data/models/") + nameDecoders.model.getName(i);
            if (!std::filesystem::exists(outDir)) {
                auto& model = resLoader.loadModel(testBody);
                vector<Animation*> animations;
                for (int j = 0; j < curAnims.size(); j++) {
                    animations.push_back(&anims[curAnims[j]]);
                }
                saveModelGLTF(model, animations, pallete, string(outDir));
            }

            if (altBody) {
                outDir = string("data/models/") + nameDecoders.model.getName(i) + "_alt";
                if (!std::filesystem::exists(outDir)) {
                    auto& model2 = resLoader.loadModel(testBody2);
                    vector<Animation*> animations;
                    for (int j = 0; j < curAnims.size(); j++) {
                        animations.push_back(&anims2[curAnims[j]]);
                    }
                    saveModelGLTF(model2, animations, pallete, string(outDir));
                }
            }
        }
    }

    void AITDExtractor::processScripts(bool floppy) {
        PakFile lifePak("original/LISTLIFE.PAK");
        vector<LifeInstructions> allLifes;
        //int i2 = 514;
        for (int i = 0; i < lifePak.headers.size(); i++)
        {
            auto& data = lifePak.readBlock(i);
            auto& life = resLoader.loadLife(data, floppy);
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

        std::filesystem::create_directories("data/scripts");

        const char* filename_c = "data/scripts/constants.lua";
        if (!std::filesystem::exists(filename_c)) {
            ofstream out(filename_c, ios::trunc | ios::out);
            LifeLUAWriter writer(out, nameDecoders);
            int objectCount = this->gameObjs.size();            
            writer.writeConsts(lifesNodes.size(), objectCount, modelCount, this->varCount);		
        }

        const char* filename = "data/scripts/main.lua";
        if (!std::filesystem::exists(filename)) {
            ofstream out(filename, ios::trunc | ios::out);
            LifeLUAWriter writer(out, nameDecoders);

            out << "dofile(\"constants.lua\")\n\n";

            for (int j = 0; j < lifesNodes.size(); j++)
            {
                out << "-- " << nameDecoders.life.getName(j, true) << "\n";
                out << "function life_" << j << "(obj)\n";
                writer.writeLifeNodes(1, lifesNodes[j]);
                out << "end\n\n";
            }
            out.flush();
            out.close();
        }

    }

    void AITDExtractor::processTracks() {
        PakFile trackPak("original/LISTTRAK.PAK");
        auto dir = string("data/tracks");
        std::filesystem::create_directories(dir);
        for (int i = 0; i < trackPak.headers.size(); i++)
        {
            auto& data = trackPak.readBlock(i);
            auto& track = loadTrack(data.data(), trackPak.headers[i].uncompressedSize);
            auto s = dir + "/" + to_string(i) + ".json";
            if (std::filesystem::exists(s)) continue;
            extractTrack(track, s, this->roomMatrix);
            //allTracks.push_back(track);
        }
    }

    void AITDExtractor::processSounds() {
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

    void AITDExtractor::processAdlibMusic() {
        //PakFile musicPak("original/LISTMUS.PAK");
        PakFile musicPak;
        musicPak.open("original/LISTMUS.PAK");
        auto dir = string("data/music");
        std::filesystem::create_directories(dir);
        for (int i2 = 0; i2 < musicPak.headers.size(); i2++) {
            auto& data = musicPak.readBlock(i2);
            auto s = dir + "/" + to_string(i2) + ".ogg";
            if (std::filesystem::exists(s)) continue;
            renderMusic(data.data(), s, true, 0.8f);            
            // auto s = dir + "/" + to_string(i2) + ".adl";
            // ofstream f(s, ios::binary);
            // f.write((char*)data.data(), data.size());
            // f.close();            
        }
    }

    void AITDExtractor::processTexts() {
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

    void AITDExtractor::processPicture(PakFile& picsPak, int id, string outPath) {
        if (!std::filesystem::exists(outPath)) {
            auto& data = picsPak.readBlock(id);
            extractBackground(data.data(), outPath.c_str(), pallete);
        }
    }

    void AITDExtractor::processPictures() {
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

    void AITDExtractor::extractAllData(bool floppy) {
        //dumpInstructions("instr.txt");
        this->processTexts();

        this->gameObjs = resLoader.loadGameObjects();
        if (!std::filesystem::exists("data/objects.json")) {
            extractGameObjects(this->gameObjs, "data/objects.json", nameDecoders);
        }

        VarsExtractor varsExtractor("original");
        varsExtractor.loadVars();
        varsExtractor.loadDefines();
        this->varCount = varsExtractor.getVars().size();
        if (!std::filesystem::exists("data/vars.json")) {
            varsExtractor.saveToJson("data/vars.json");
        }

        this->processStages();
        this->processModels();
        //processTestScript();
        this->processTracks();
        this->processSounds();
        this->processPictures();

        if (floppy) {
            this->processAdlibMusic();
        } else {
            //TODO: CD Audio Ripper
        }

        this->processScripts(floppy);

        //PakFile animPak("original/LISTANIM.PAK");
        //vector<Animation> anims;
        //for (int j = 0; j < animPak.headers.size(); j++)
        //{
        //    auto& data = animPak.readBlock(j);
        //    auto& anim = loadAnimation(data.data());
        //    anims.push_back(anim);
        //}
    }

    string AITDExtractor::intToStrWidth(int number, int width) {
        std::stringstream ss;
        ss << std::setw(width) << std::setfill('0') << number;
        return ss.str();
    }

    std::unique_ptr<IAITDExtractor> createAITDExtractor() {
        return std::make_unique<AITDExtractor>();
    }

}