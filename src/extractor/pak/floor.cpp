#include "common.h"
#include "floor.h"
#include "pak.h"
#include <stdexcept>

struct debugBlock {
    char* name;
    int index;
    int start;
    int end;
};
debugBlock debugBlocks[1000];
int countBlock = 0;
void addDebugBlock(debugBlock db) {
    /*
    for (int i = 0; i < countBlock - 1; i++) {
        if ((db.start >= debugBlocks[i].start) && (db.start < debugBlocks[i].end)) {
            throw new std::exception("Block collision (start)");
        }
        if ((db.end > debugBlocks[i].start) && (db.end <= debugBlocks[i].end)) {
            throw new std::exception("Block collision (end)");
        }
    }
    debugBlocks[countBlock] = db;
    countBlock++;
    */
}

void loadRooms(floorStruct* result, char* filename) {
    int i;
    int roomsRawDataSize = getPakSize(filename, 0);
    char* roomsRawData = loadPak(filename, 0);

    int numMax = (((READ_LE_U32(roomsRawData)) / 4));

    result->roomCount = 0;
    for (i = 0; i < numMax; i++)
    {
        if (roomsRawDataSize >= READ_LE_U32(roomsRawData + i * 4))
        {
            result->roomCount++;
        }
        else break;
    }
    result->rooms = new roomStruct[result->roomCount];

    for (i = 0; i < result->roomCount; i++)
    {
        u32 j;

        roomStruct* curRoom = &result->rooms[i];
        char* curRoomRawData = roomsRawData + READ_LE_U32(roomsRawData + i * 4);

        //Copy room header to result struct
        curRoom->worldX = READ_LE_S16(curRoomRawData + 4);
        curRoom->worldY = READ_LE_S16(curRoomRawData + 6);
        curRoom->worldZ = READ_LE_S16(curRoomRawData + 8);

        curRoom->numCameraInRoom = READ_LE_U16(curRoomRawData + 0xA);
        curRoom->cameraIdxTable = new u16[curRoom->numCameraInRoom];
        for (j = 0; j < curRoom->numCameraInRoom; j++)
        {
            curRoom->cameraIdxTable[j] = READ_LE_U16(curRoomRawData + 0xC + 2 * j);
        }

        // hard col read
        char* hardColRawData = curRoomRawData + READ_LE_U16(curRoomRawData);
        curRoom->numHardCol = READ_LE_U16(hardColRawData);
        hardColRawData += 2;
        if (curRoom->numHardCol)
        {
            curRoom->hardColTable = new hardColStruct[curRoom->numHardCol];
            for (j = 0; j < curRoom->numHardCol; j++)
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
        }

        // sce zone read
        char* sceZoneRawData = curRoomRawData + READ_LE_U16(curRoomRawData + 2);
        curRoom->numSceZone = READ_LE_U16(sceZoneRawData);
        sceZoneRawData += 2;
        if (curRoom->numSceZone)
        {
            curRoom->sceZoneTable = new sceZoneStruct[curRoom->numSceZone];
            for (j = 0; j < curRoom->numSceZone; j++)
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
}

void loadCameraCover(cameraViewedRoomStruct* curCameraViewedRoom, u8* camerasRawData, u32 offset) {
    int j;
    u8* rawData = camerasRawData + offset;
    //u16 unknown = READ_LE_U16(rawData);
    curCameraViewedRoom->numCoverZones = READ_LE_U16(rawData);
    rawData += 2;
    curCameraViewedRoom->coverZones = new cameraZoneEntryStruct[curCameraViewedRoom->numCoverZones];

    for (j = 0; j < curCameraViewedRoom->numCoverZones; j++)
    {
        int pointIdx;
        int numPoints;
        curCameraViewedRoom->coverZones[j].numPoints = numPoints = READ_LE_U16(rawData);
        rawData += 2;
        curCameraViewedRoom->coverZones[j].pointTable = new cameraZonePointStruct[numPoints + 1];
        for (pointIdx = 0; pointIdx < curCameraViewedRoom->coverZones[j].numPoints; pointIdx++)
        {
            curCameraViewedRoom->coverZones[j].pointTable[pointIdx].x = READ_LE_U16(rawData);
            rawData += 2;
            curCameraViewedRoom->coverZones[j].pointTable[pointIdx].y = READ_LE_U16(rawData);
            rawData += 2;
        }
        // copy first point to last position
        curCameraViewedRoom->coverZones[j].pointTable[numPoints].x = curCameraViewedRoom->coverZones[j].pointTable[0].x; 
        curCameraViewedRoom->coverZones[j].pointTable[numPoints].y = curCameraViewedRoom->coverZones[j].pointTable[0].y;
    }

    struct debugBlock db = { "cameraCovers", 0, offset, (rawData - camerasRawData) };
    addDebugBlock(db);
}

void loadCameraMaskV1(cameraViewedRoomStruct* curCameraViewedRoom, u8* camerasRawData, u32 offset) {
    int j;
    u8* maskRawData = camerasRawData + offset;
    curCameraViewedRoom->numV1Mask = READ_LE_U16(maskRawData);
    maskRawData += 2;

    curCameraViewedRoom->V1masks = new cameraMaskV1Struct[curCameraViewedRoom->numV1Mask];
    for (int maskIdx = 0; maskIdx < curCameraViewedRoom->numV1Mask; maskIdx++)    {
        
        cameraMaskV1Struct* curMask = &curCameraViewedRoom->V1masks[maskIdx];
        
        curMask->numZone = READ_LE_U16(maskRawData);
        maskRawData += 2;
        u16 polyOffset = READ_LE_U16(maskRawData);
        maskRawData += 2;

        curMask->zones = new zoneStruct[curMask->numZone];
        for (int i = 0; i < curMask->numZone; i++) {
            s16* zoneTest = (s16*)maskRawData;
            zoneStruct* z = &curMask->zones[i];
            z->zoneX1 = READ_LE_S16(maskRawData + 0);
            z->zoneZ1 = READ_LE_S16(maskRawData + 2);
            z->zoneX2 = READ_LE_S16(maskRawData + 4);
            z->zoneZ2 = READ_LE_S16(maskRawData + 6);
            maskRawData += 8;
        }

        u8* rawVerts = camerasRawData + offset + polyOffset + 2;
        curMask->numPolys = READ_LE_U16(rawVerts);
        rawVerts += 2;
        curMask->polys = new cameraMaskV1PolyStruct[curMask->numPolys];
        for (int polyIdx = 0; polyIdx < curMask->numPolys; polyIdx++)
        {
            cameraMaskV1PolyStruct* poly = &curMask->polys[polyIdx];
            poly->pointsCount = READ_LE_S16(rawVerts);
            rawVerts += 2;
            if ( poly->pointsCount > 0)
            {
                poly->points = new s16[poly->pointsCount * 2];
                for (int verticeId = 0; verticeId < poly->pointsCount; verticeId++)
                {
                    poly->points[verticeId * 2 + 0] = READ_LE_S16(rawVerts);
                    poly->points[verticeId * 2 + 1] = READ_LE_S16(rawVerts + 2);
                    rawVerts += 4;
                }
            }
            else 
            {
                int y = 0;
                //invalid poly
            }
        }
        int offsetTest = rawVerts - (camerasRawData + offset);
     }
    
    struct debugBlock db = { "cameraMaskV1", 0, offset, (maskRawData - camerasRawData) };
    addDebugBlock(db);
}

void loadCameras(floorStruct* result, char* filename) {
    int i;
    int camerasRawDataSize = getPakSize(filename, 1);
    u8* camerasRawData = (u8*)loadPak(filename, 1);

    int maxExpectedNumberOfCamera = ((READ_LE_U32(camerasRawData)) / 4);
    result->cameraCount = 0;
    int minOffset = 0;
    for (int i = 0; i < maxExpectedNumberOfCamera; i++)
    {
        int offset = READ_LE_U32(camerasRawData + i * 4);
        if ((offset > minOffset) && (offset < camerasRawDataSize))
        {
            minOffset = offset;
            result->cameraCount++;
        }
        else
        {
            break;
        }
    }

    result->cameras = new cameraStruct[result->cameraCount];
    for (i = 0; i < result->cameraCount; i++)
    {
        int k;
        unsigned int offset = READ_LE_U32(camerasRawData + (i * 4));
        cameraStruct* curCamera = &result->cameras[i];

        u8* cameraRawData = (u8*)(camerasRawData + offset);
        //curCamera->offset = cameraRawData - camerasRawData;
        u8* backupDataPtr = cameraRawData;

        curCamera->alpha = READ_LE_U16(cameraRawData + 0x00);
        curCamera->beta = READ_LE_U16(cameraRawData + 0x02);
        curCamera->gamma = READ_LE_U16(cameraRawData + 0x04);

        curCamera->x = READ_LE_U16(cameraRawData + 0x06);
        curCamera->y = READ_LE_U16(cameraRawData + 0x08);
        curCamera->z = READ_LE_U16(cameraRawData + 0x0A);

        curCamera->focal1 = READ_LE_U16(cameraRawData + 0x0C);
        curCamera->focal2 = READ_LE_U16(cameraRawData + 0x0E);
        curCamera->focal3 = READ_LE_U16(cameraRawData + 0x10);

        curCamera->numViewedRooms = READ_LE_U16(cameraRawData + 0x12);

        cameraRawData += 0x14;
        
        struct debugBlock db = { "cam", i, offset, (cameraRawData - camerasRawData) };
        addDebugBlock(db);

        curCamera->viewedRoomTable = new cameraViewedRoomStruct[curCamera->numViewedRooms];
        for (k = 0; k < curCamera->numViewedRooms; k++)
        {
            cameraViewedRoomStruct* curCameraViewedRoom = &curCamera->viewedRoomTable[k];
            int viewOffset = cameraRawData - camerasRawData;
            curCameraViewedRoom->viewedRoomIdx = READ_LE_U16(cameraRawData + 0x00);
            
            u16 offsetToMask = READ_LE_U16(cameraRawData + 0x02);
            u16 offsetToCover = READ_LE_U16(cameraRawData + 0x04);
            //curCameraViewedRoom->offsetToHybrids = 0;
            //curCameraViewedRoom->offsetCamOptims = 0;

            curCameraViewedRoom->lightX = READ_LE_U16(cameraRawData + 0x06);
            curCameraViewedRoom->lightY = READ_LE_U16(cameraRawData + 0x08);
            curCameraViewedRoom->lightZ = READ_LE_U16(cameraRawData + 0x0A);

            cameraRawData += 0x0C;

            struct debugBlock db = { "camViewRoom", k, viewOffset, (cameraRawData - camerasRawData) };
            addDebugBlock(db);

            if (offsetToMask) {
                loadCameraMaskV1(curCameraViewedRoom, camerasRawData, offset + offsetToMask);
            }
            if (offsetToCover) {
                loadCameraCover(curCameraViewedRoom, camerasRawData, offset + offsetToCover);
            }


        }

    }

}

floorStruct* loadFloor(char* filename) {
    floorStruct* result = new floorStruct;
    //loadRooms(result, filename);
    loadCameras(result, filename);
    return result;
}


