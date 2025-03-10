#include "tests/tests.h"
#include <cassert>
#include <cstdlib>
#include <filesystem>

#include "extractor/floor_extractor.h"
#include "extractor/background_extractor.h"
#include "extractor/mask_extractor.h"

void extractAllData() {
    char str[100];
    char str2[100];
    char str3[100];
    int tmp = 3;
    for (int fl = 0; fl < 8; fl++) {
        //std::filesystem::create_directories("original");
        //std::filesystem::create_directories("backgrounds");
        sprintf(str, "original/ETAGE%02d", fl);
        auto curFloor = loadFloorPak(str);
        for (int cam = 0; cam < curFloor->cameras.size(); cam++) {
            //background
            sprintf(str2, "data/floor_%02d/camera_%02d", fl, cam);
            std::filesystem::create_directories(str2);
            sprintf(str, "original/CAMERA%02d", fl );
            sprintf(str3, "%s/background.png", str2);
            if (!std::filesystem::exists(str3)) {
                extractBackground(str, cam, str3);
            }

            for (int vw = 0; vw < curFloor->cameras[cam].viewedRoomTable.size(); vw++) {
                auto curVw = &curFloor->cameras[cam].viewedRoomTable[vw];
                for (int ovl = 0; ovl < curVw->overlays_V1.size(); ovl++) {
                    auto polys = &curVw->overlays_V1[ovl].polygons;
                    sprintf(str2, "data/floor_%02d/camera_%02d/mask_%02d_%02d.png", fl, cam, vw, ovl);
                    if (!std::filesystem::exists(str2)) {
                        extractV1Mask(polys, str2);
                    }
                }
            }

            //mask
            //if (tmp > 0) { //tmp--;
            //}
        }
        delete curFloor;
    }    
}

int main(void)
{
    extractAllData();

    //CameraTest::runTest();
    //MaskTest::runTest();
    //ModelTest::runTest();
    PerspectiveTest::runTest();

    return 0;
}
