#pragma once
#include <cassert>
#include <cstdlib>
#include <filesystem>

#include "pak/pak.h"
#include "structs/floor.h"
#include "structs/model.h"

floorStruct loadFloorPak(string filename);
void saveFloorGLTF(floorStruct& floor2, char* filename);
void extractBackground(u8 * img, char* outPng);
void extractV1Mask(const std::vector<cameraOverlayPolygon>* srcPolys, const char* outPng);
PakModel loadModel(char* data, int size);
void saveModelGLTF(const PakModel& model, const string dirname);
void extractObjects(string from, string josnTo);
void extractVars(string dir, string josnTo);
void extractLife(string fname, string outDir);

inline void extractAllData() {
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

    //PakFile pak;
    //pak.open("original/LISTBODY.PAK");
    ////int i = 0;
    //for ( int i = 0; i < pak.headers.size(); i++ )
    //{
    //    auto h = pak.headers[i];
    //    auto& testBody = pak.readBlock(i);
    //    auto& model = loadModel((char*)testBody.data(), h.uncompressedSize);
    //    sprintf(str, "data/models/LISTBODY_%d", i);
    //    std::filesystem::create_directories(str);
    //    saveModelGLTF(model, string(str));
    //}

    /*char* srcFN2 = "original/LISTBOD2.PAK";
    int filesNum2 = PAK_getNumFiles(srcFN2);
    for (int i = 0; i < filesNum2; i++)
    {
        int size = getPakSize(srcFN2, i);
        char* testBody = loadPak(srcFN2, i);
        loadModel(testBody, size);
        delete testBody;
    }*/

    //extractObjects("original/OBJETS.ITD", "data/objects.json");
    //extractVars("original", "data/vars.json");

    extractLife("original/LISTLIFE.PAK", "data/life_scripts.txt");
}