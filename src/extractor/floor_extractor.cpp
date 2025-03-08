#include "../structures/floor.h"
#include "pak/pak.h"
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <list>

u8* debugStartPtr;
int debugSize;
struct debugBlock {
    char* name;
    int index;
    int start;
    int end;
};
std::list<debugBlock> debugBlocks;
void addDebugBlock(const char* name, int idx, u8* start, u8* end) {
    debugBlock dbNew = { (char*)name, idx, debugStartPtr - start, debugStartPtr - end };
    for (auto db = debugBlocks.begin(); db != debugBlocks.end(); ++db) {
        if ((dbNew.start >= db->start) && (dbNew.start < db->end)) {
            throw new std::exception("Block collision (start)");
        }
        if ((dbNew.end > db->start) && (dbNew.end <= db->end)) {
            throw new std::exception("Block collision (end)");
        }
    }
    debugBlocks.push_back(dbNew);
}

void loadRooms(floorStruct* result, char* filename) {
    int i;
    int roomsRawDataSize = getPakSize(filename, 0);
    char* roomsRawData = loadPak(filename, 0);

    int numMax = (((READ_LE_U32(roomsRawData)) / 4));

    int roomCount = 0;
    for (i = 0; i < numMax; i++)
    {
        if (roomsRawDataSize >= READ_LE_U32(roomsRawData + i * 4))
        {
            roomCount++;
        }
        else break;
    }

    result->rooms.resize(roomCount);
    for (i = 0; i < result->rooms.size(); i++)
    {
        u32 j;

        roomStruct* curRoom = &result->rooms[i];
        char* curRoomRawData = roomsRawData + READ_LE_U32(roomsRawData + i * 4);

        //Copy room header to result struct
        curRoom->worldX = READ_LE_S16(curRoomRawData + 4);
        curRoom->worldY = READ_LE_S16(curRoomRawData + 6);
        curRoom->worldZ = READ_LE_S16(curRoomRawData + 8);

        int numCameraInRoom = READ_LE_U16(curRoomRawData + 0xA);
        curRoom->cameraIdxTable.resize(numCameraInRoom);
        for (j = 0; j < numCameraInRoom; j++)
        {
            curRoom->cameraIdxTable[j] = READ_LE_U16(curRoomRawData + 0xC + 2 * j);
        }

        // hard col read
        char* hardColRawData = curRoomRawData + READ_LE_U16(curRoomRawData);
        int numHardCol = READ_LE_U16(hardColRawData);
        hardColRawData += 2;
        curRoom->hardColTable.resize(numHardCol);
        for (j = 0; j < numHardCol; j++)
        {
            ZVStruct* zvData = &curRoom->hardColTable[j].zv;
            zvData->ZVX1 = READ_LE_S16(hardColRawData + 0x00);
            zvData->ZVX2 = READ_LE_S16(hardColRawData + 0x02);
            zvData->ZVY1 = READ_LE_S16(hardColRawData + 0x04);
            zvData->ZVY2 = READ_LE_S16(hardColRawData + 0x06);
            zvData->ZVZ1 = READ_LE_S16(hardColRawData + 0x08);
            zvData->ZVZ2 = READ_LE_S16(hardColRawData + 0x0A);

            curRoom->hardColTable[j].parameter = READ_LE_U16(hardColRawData + 0x0C);
            curRoom->hardColTable[j].type = READ_LE_U16(hardColRawData + 0x0E);

            hardColRawData += 0x10;
        }


        // sce zone read
        char* sceZoneRawData = curRoomRawData + READ_LE_U16(curRoomRawData + 2);
        int numSceZone = READ_LE_U16(sceZoneRawData);
        sceZoneRawData += 2;
        curRoom->sceZoneTable.resize(numSceZone);
        for (j = 0; j < numSceZone; j++)
        {
            ZVStruct* zvData = &curRoom->sceZoneTable[j].zv;

            zvData->ZVX1 = READ_LE_S16(sceZoneRawData + 0x00);
            zvData->ZVX2 = READ_LE_S16(sceZoneRawData + 0x02);
            zvData->ZVY1 = READ_LE_S16(sceZoneRawData + 0x04);
            zvData->ZVY2 = READ_LE_S16(sceZoneRawData + 0x06);
            zvData->ZVZ1 = READ_LE_S16(sceZoneRawData + 0x08);
            zvData->ZVZ2 = READ_LE_S16(sceZoneRawData + 0x0A);

            curRoom->sceZoneTable[j].parameter = READ_LE_U16(sceZoneRawData + 0x0C);
            curRoom->sceZoneTable[j].type = READ_LE_U16(sceZoneRawData + 0x0E);

            sceZoneRawData += 0x10;
        }
    }
}

void loadCameraCovers(cameraViewedRoomStruct* curCameraViewedRoom, u8* coversRawData, u8* cameraStartRawData) {
    int j;
    u8* rawData = coversRawData;
    //u16 unknown = READ_LE_U16(rawData);
    int numCoverZones = READ_LE_U16(rawData);
    rawData += 2;
    curCameraViewedRoom->coverZones.resize(numCoverZones);
    for (j = 0; j < numCoverZones; j++)
    {
        int pointIdx;
        int numPoints = READ_LE_U16(rawData);
        rawData += 2;
        curCameraViewedRoom->coverZones[j].pointTable.resize(numPoints);
        for (pointIdx = 0; pointIdx < numPoints; pointIdx++)
        {
            curCameraViewedRoom->coverZones[j].pointTable[pointIdx].x = READ_LE_U16(rawData);
            rawData += 2;
            curCameraViewedRoom->coverZones[j].pointTable[pointIdx].y = READ_LE_U16(rawData);
            rawData += 2;
        }
    }
    addDebugBlock("CameraCovers", 0, coversRawData, rawData);
}

void loadCameraOverlaysV1(cameraViewedRoomStruct* curCameraViewedRoom, u8* startOverlayRawData, u8* cameraStartRawData) {
    u8* maskRawData = startOverlayRawData;
    int numV1Mask = READ_LE_U16(maskRawData);
    maskRawData += 2;
    curCameraViewedRoom->overlays_V1.resize(numV1Mask);
    for (int maskIdx = 0; maskIdx < numV1Mask; maskIdx++)    {
        cameraOverlayV1Struct* curMask = &curCameraViewedRoom->overlays_V1[maskIdx];
        
        int numZone = READ_LE_U16(maskRawData);
        maskRawData += 2;
        u16 polyOffset = READ_LE_U16(maskRawData);
        maskRawData += 2;

        curMask->zones.resize(numZone);
        for (int i = 0; i < numZone; i++) {
            s16* zoneTest = (s16*)maskRawData;
            zoneStruct* z = &curMask->zones[i];
            z->zoneX1 = READ_LE_S16(maskRawData + 0);
            z->zoneZ1 = READ_LE_S16(maskRawData + 2);
            z->zoneX2 = READ_LE_S16(maskRawData + 4);
            z->zoneZ2 = READ_LE_S16(maskRawData + 6);
            maskRawData += 8;
        }
        addDebugBlock("CameraOverlaysV1", 0, startOverlayRawData, maskRawData);

        int offsetTest = maskRawData - cameraStartRawData;

        u8* startRawVerts = startOverlayRawData + polyOffset;
        u8* rawVerts = startRawVerts;
        int numPolys = READ_LE_U16(rawVerts);
        rawVerts += 2;
        curMask->polygons.resize(numPolys);
        for (int polyIdx = 0; polyIdx < numPolys; polyIdx++)
        {
            cameraOverlayPolygon* poly = &curMask->polygons[polyIdx];
            int pointsCount = READ_LE_S16(rawVerts);
            rawVerts += 2;
            if ( pointsCount > 0)
            {
                poly->resize(pointsCount);
                for (int verticeId = 0; verticeId < pointsCount; verticeId++)
                {
                    (*poly)[verticeId].x = READ_LE_S16(rawVerts);
                    (*poly)[verticeId].y = READ_LE_S16(rawVerts + 2);
                    rawVerts += 4;
                }
            }
        }
        addDebugBlock("CameraOverlayVerts", 0, startRawVerts, rawVerts);
     }   
}

void loadCameraRoom(cameraViewedRoomStruct* curCameraRoom, u8* cameraRoomRawData, u8* cameraStartRawData) {
    curCameraRoom->viewedRoomIdx = READ_LE_U16(cameraRoomRawData + 0x00);
    u16 offsetToOverlays = READ_LE_U16(cameraRoomRawData + 0x02);
    u16 offsetToCovers = READ_LE_U16(cameraRoomRawData + 0x04);
    //curCameraRoom->offsetToHybrids = 0;
    //curCameraRoom->offsetCamOptims = 0;

    curCameraRoom->lightX = READ_LE_U16(cameraRoomRawData + 0x06);
    curCameraRoom->lightY = READ_LE_U16(cameraRoomRawData + 0x08);
    curCameraRoom->lightZ = READ_LE_U16(cameraRoomRawData + 0x0A);

    addDebugBlock("camViewRoom", 0, cameraRoomRawData, cameraRoomRawData + 0x0C);
    
    //int offsetTest = cameraStartRawData - debugStartPtr; //24 858 1300 2240
    if (offsetToOverlays) {
        loadCameraOverlaysV1(curCameraRoom, cameraStartRawData + offsetToOverlays, cameraStartRawData);
    }
    if (offsetToCovers) {
        loadCameraCovers(curCameraRoom, cameraStartRawData + offsetToCovers, cameraStartRawData);
    }
}

void loadCameras(floorStruct* result, char* filename) {
    int i;
    int camerasRawDataSize = getPakSize(filename, 1);
    u8* camerasRawData = (u8*)loadPak(filename, 1);

    debugBlocks.clear();
    debugStartPtr = camerasRawData;
    debugSize = camerasRawDataSize;

    int maxExpectedNumberOfCamera = ((READ_LE_U32(camerasRawData)) / 4);
    int cameraCount = 0;
    int minOffset = 0;
    for (int i = 0; i < maxExpectedNumberOfCamera; i++)
    {
        int offset = READ_LE_U32(camerasRawData + i * 4);
        if ((offset > minOffset) && (offset < camerasRawDataSize))
        {
            minOffset = offset;
            cameraCount++;
        }
        else
        {
            break;
        }
    }

    result->cameras.resize(cameraCount);
    for (i = 0; i < result->cameras.size(); i++)
    {
        int k;
        unsigned int offset = READ_LE_U32(camerasRawData + (i * 4));
        cameraStruct* curCamera = &result->cameras[i];

        u8* cameraRawData = (u8*)(camerasRawData + offset);
        //curCamera->offset = cameraRawData - camerasRawData;

        curCamera->alpha = READ_LE_U16(cameraRawData + 0x00);
        curCamera->beta = READ_LE_U16(cameraRawData + 0x02);
        curCamera->gamma = READ_LE_U16(cameraRawData + 0x04);

        curCamera->x = READ_LE_U16(cameraRawData + 0x06);
        curCamera->y = READ_LE_U16(cameraRawData + 0x08);
        curCamera->z = READ_LE_U16(cameraRawData + 0x0A);

        curCamera->focal1 = READ_LE_U16(cameraRawData + 0x0C);
        curCamera->focal2 = READ_LE_U16(cameraRawData + 0x0E);
        curCamera->focal3 = READ_LE_U16(cameraRawData + 0x10);

        int numViewedRooms = READ_LE_U16(cameraRawData + 0x12);
        cameraRawData += 0x14;
        u8* cameraStartRawData = (u8*)(camerasRawData + offset);

        addDebugBlock( "CameraHead", i, camerasRawData, cameraRawData);

        curCamera->viewedRoomTable.resize(numViewedRooms);
        for (k = 0; k < numViewedRooms; k++)
        {
            loadCameraRoom(&curCamera->viewedRoomTable[k], cameraRawData, cameraStartRawData);
            cameraRawData += 0x0C;
        }
    }
}

floorStruct* loadFloorPak(char* filename) {
    floorStruct* result = new floorStruct;
    loadRooms(result, filename);
    loadCameras(result, filename);
    std::cout << "Load Floor END...\n";
    return result;
}

void saveFloorTxt(char* filename, floorStruct* fs) {
    char fname[50];
    sprintf(fname, "%s.txt", filename);
    std::ofstream myfile;
    myfile.open(fname);

    myfile << "CAMERAS: " << fs->cameras.size() << "\n";
    for (int i = 0; i < fs->cameras.size(); i++) {
        cameraStruct* cam = &fs->cameras[i];
        myfile << "CAMERA:\n";
        myfile << " XYZ: " << cam->x << " " << cam->y << " " << cam->z << "\n";
        myfile << " VIEWS: " << cam->viewedRoomTable.size() << "\n";
        for (int i2 = 0; i2 < cam->viewedRoomTable.size(); i2++) {
            cameraViewedRoomStruct* vw = &cam->viewedRoomTable[i2];
            myfile << " VIEW:\n";
            myfile << "  MASKS_V1: " << vw->overlays_V1.size() << "\n";
            for (int i3 = 0; i3 < vw->overlays_V1.size(); i3++) {
                cameraOverlayV1Struct* mask = &vw->overlays_V1[i3];
                myfile << "  MASK_V1:\n";
                myfile << "  ZONES: " << mask->zones.size() << "\n";
                for (int i4 = 0; i4 < mask->zones.size(); i4++) {
                    myfile << "   ZONE: " << mask->zones[i4].zoneX1 << " " << mask->zones[i4].zoneZ1 << " " << mask->zones[i4].zoneX2 << " " << mask->zones[i4].zoneZ2 << "\n";
                }
            }
        }
    }
    myfile.close();
}