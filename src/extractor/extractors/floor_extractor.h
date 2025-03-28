#pragma once

//#include <json_fwd.hpp>
#include <fstream>
#include <vector>
#include <raymath.h>
#include <iomanip>

#include "../utils/my_gltf.h"
#include "../structs/floor.h"
using json = nlohmann::json;
using namespace std;

int createBoxNode(tinygltf::Model& m, string name, hardColStruct& coll) {
    tinygltf::Node collN;
    collN.name = name;
    collN.mesh = 0;

    double SizeX = (double)(coll.zv.ZVX2 - coll.zv.ZVX1) / 1000;
    double SizeY = (double)(coll.zv.ZVY2 - coll.zv.ZVY1) / 1000;
    double SizeZ = (double)(coll.zv.ZVZ2 - coll.zv.ZVZ1) / 1000;

    collN.translation = {
        (double)coll.zv.ZVX1 / 1000,
        -((double)coll.zv.ZVY1 / 1000) - SizeY,
        -((double)coll.zv.ZVZ1 / 1000) - SizeZ
    };
    collN.scale = {
        SizeX,
        SizeY,
        SizeZ,
    };

    m.nodes.push_back(collN);
    return m.nodes.size() - 1;
}

void addCamera(tinygltf::Model& m, int camIdx, cameraStruct& cam) {
    auto tx = (float)cam.alpha * 360 / 1024;
    auto ty = (float)cam.beta * 360 / 1024;
    auto tz = (float)cam.gamma * 360 / 1024;
    /* 20 335.4 0 */
    //Matrix my = MatrixInvert(MatrixRotateY(((float)cam.beta * 2 * PI / 1024)));
    Matrix my = MatrixRotateY((float)cam.beta * 2 * PI / 1024);
    Matrix mx = MatrixRotateX((float)cam.alpha * 2 * PI / 1024);
    Matrix mz = MatrixRotateZ((float)cam.gamma * 2 * PI / 1024);
    auto myxz = MatrixTranspose(MatrixMultiply(MatrixMultiply(my, mx), mz));
    auto q = QuaternionFromMatrix(myxz);

    //MatrixLookAt

    float legcFocalX = (float)cam.focalX / 320;
    float legcFocalY = (float)cam.focalY / 200;
    float aspect = legcFocalY / legcFocalX;
    float fovY = 2.0 * atan(1.0 / (legcFocalY * 2)); //in radians
    float nearDist = (float)cam.nearDistance / 1000;
    Vector3 cameraForw = Vector3Normalize({ myxz.m8, myxz.m9, myxz.m10 });
    Vector3 camPosition = { (float)cam.x / 100, (float)cam.y / 100, (float)cam.z / 100 };
    camPosition = Vector3Add(camPosition, Vector3Scale(cameraForw, nearDist));

    tinygltf::Camera camera;
    camera.name = string("camera_") + to_string(camIdx);
    camera.type = "perspective";
    camera.perspective.aspectRatio = aspect;
    camera.perspective.yfov = fovY;
    camera.perspective.znear = nearDist;
    camera.perspective.zfar = 100000;
    m.cameras.push_back(camera);
    int cam2Idx = m.cameras.size() - 1;

    tinygltf::Node camN;
    camN.name = string("camera_") + to_string(camIdx);
    camN.camera = cam2Idx;
    camN.translation = {
        camPosition.x,
        camPosition.y,
        camPosition.z
    };
    camN.rotation = { q.x, q.y, q.z, q.w };
    m.nodes.push_back(camN);
}

void saveFloorGLTF(floorStruct& floor2, char* filename)
{
    floorStruct* floor = &floor2;
    nlohmann::json floorJson;
    tinygltf::Model m;
    m.asset.version = "2.0";
    m.asset.generator = "open-AITD";
    
    createCubeMesh(m);
    
    vector<tinygltf::Node> roomNodes(floor->rooms.size());

    floorJson["rooms"] = nlohmann::json::array();
    for (int roomId = 0; roomId < floor->rooms.size(); roomId++) {
        auto& room = floor->rooms[roomId];
        
        nlohmann::json roomJson;
        roomJson["colliders"] = nlohmann::json::array();
        roomJson["zones"] = nlohmann::json::array();

        auto& roomN = roomNodes[roomId];
        roomN.name = string("room_") + to_string(roomId);
        roomN.translation = {
            (float)room.worldX / 100,
            -(float)room.worldY / 100,
            (float)room.worldZ / 100
        };

        tinygltf::Node rootColl;
        rootColl.name = string("colliders_") + to_string(roomId);
        for (int collIdx = 0; collIdx < room.hardColTable.size(); collIdx++) {
            auto& coll = room.hardColTable[collIdx];

            json collJson;
            collJson["parameter"] = coll.parameter;
            collJson["type"] = coll.type;
            roomJson["colliders"].push_back(collJson);

            createBoxNode(m, string("coll_") + to_string(roomId) + "_" + to_string(collIdx), coll);
            int collNIdx = m.nodes.size() - 1;
            rootColl.children.push_back(collNIdx);
        }
        m.nodes.push_back(rootColl);
        int rootCollIdx = m.nodes.size() - 1;
        roomN.children.push_back(rootCollIdx);


        tinygltf::Node rootZone;
        rootZone.name = string("zones_") + to_string(roomId);
        for (int zoneIdx = 0; zoneIdx < room.sceZoneTable.size(); zoneIdx++) {
            auto& zone = room.sceZoneTable[zoneIdx];

            json collJson;
            collJson["parameter"] = zone.parameter;
            collJson["type"] = zone.type;
            roomJson["zones"].push_back(collJson);

            createBoxNode(m, string("zone_") + to_string(roomId) + "_" + to_string(zoneIdx), zone);
            int zoneNIdx = m.nodes.size() - 1;
            rootZone.children.push_back(zoneNIdx);
        }
        m.nodes.push_back(rootZone);
        int rootZoneIdx = m.nodes.size() - 1;
        roomN.children.push_back(rootZoneIdx);

        //int roomNIdx = m.nodes.size() - 1;
        floorJson["rooms"].push_back(roomJson);
    }

    floorJson["cameras"] = nlohmann::json::array();
    for (int camIdx = 0; camIdx < floor->cameras.size(); camIdx++) {
        auto& cam = floor->cameras[camIdx];
        
        json cameraJson;
        //cameraJson["roomViews"] = nlohmann::json::array();

        addCamera(m, camIdx, cam);

        for (int viewIdx = 0; viewIdx < cam.viewedRoomTable.size(); viewIdx++) {
            auto& vw = cam.viewedRoomTable[viewIdx];
            auto roomId = vw.viewedRoomIdx;
            cameraJson["rooms"].push_back(roomId);

            //json roomViewsJson;
            //roomViewsJson["roomId"] = roomId;

            auto& roomN = roomNodes[vw.viewedRoomIdx];
            tinygltf::Node camRoomN;
            camRoomN.name = string("camera_room_") + to_string(camIdx) + "_" + to_string(roomId);

            //roomViewsJson["overlays"] = json::array();
            for (int ovlIdx = 0; ovlIdx < vw.overlays_V1.size(); ovlIdx++) {
                auto& ovl = vw.overlays_V1[ovlIdx];
                //json overlay = json::object();
                //overlay["zoneCount"] = ovl.zones.size();
                for (int ovlZIdx = 0; ovlZIdx < ovl.zones.size(); ovlZIdx++) {
                    auto& ovlZ = ovl.zones[ovlZIdx];
                    tinygltf::Node ovlZN;
                    ovlZN.name = string("overlay_zone_") + to_string(camIdx) + "_" + to_string(roomId) + "_" + to_string(ovlIdx) + "_" + to_string(ovlZIdx);
                    ovlZN.mesh = 0;
                    float z = (float)(ovlZ.zoneZ2 - ovlZ.zoneZ1) / 100;
                    ovlZN.translation = {
                        (float)ovlZ.zoneX1 / 100,
                        0,
                        -((float)ovlZ.zoneZ1 / 100) - z
                    };
                    ovlZN.scale = {
                        (float)(ovlZ.zoneX2 - ovlZ.zoneX1) / 100,
                        -0.1,
                        z,
                    };
                    m.nodes.push_back(ovlZN);
                    int ovlZNIdx = m.nodes.size() - 1;
                    camRoomN.children.push_back(ovlZNIdx);
                }
                //roomViewsJson["overlays"].push_back(overlay);
            }

            //roomViewsJson["coverZones"] = vw.coverZones.size();
            for (int zoneIdx = 0; zoneIdx < vw.coverZones.size(); zoneIdx++) {
                auto& camZone = vw.coverZones[zoneIdx];
                vector<float> flzone(camZone.pointTable.size() * 3);
                for (int pIdx = 0; pIdx < camZone.pointTable.size(); pIdx++) {
                    auto& p = camZone.pointTable[pIdx];
                    flzone[pIdx * 3 + 0] = (float)p.x / 100;// +camN.translation[0];
                    flzone[pIdx * 3 + 1] = 0;
                    flzone[pIdx * 3 + 2] = -(float)p.y / 100;// +camN.translation[2];
                }
                auto lineMeshIdx = createLineMesh(m, flzone);

                tinygltf::Node camZoneN;
                camZoneN.name = string("cam_zone_") + to_string(camIdx) + "_" + to_string(roomId) + "_" + to_string(zoneIdx);
                camZoneN.mesh = lineMeshIdx;

                m.nodes.push_back(camZoneN);
                int camZoneNIdx = m.nodes.size() - 1;
                camRoomN.children.push_back(camZoneNIdx);
            }

            m.nodes.push_back(camRoomN);
            int idx = m.nodes.size() - 1;
            roomN.children.push_back(idx);

            //cameraJson["roomViews"].push_back(roomViewsJson);
        }

        floorJson["cameras"].push_back(cameraJson);
    }

    for (int roomId = 0; roomId < floor->rooms.size(); roomId++) {
        m.nodes.push_back( roomNodes[roomId] );
    }

    // Save it to a file
    std::ofstream o(string(filename) + ".json");
    o << std::setw(2) << floorJson;

    tinygltf::TinyGLTF gltf;
    gltf.WriteGltfSceneToFile(&m, string(filename)+".gltf",
        false, // embedImages
        true, // embedBuffers
        true, // pretty print
        false
    );

}