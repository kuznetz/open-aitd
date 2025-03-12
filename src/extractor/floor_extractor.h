#pragma once
#include "../../libs/json.hpp"
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_IMPLEMENTATION
#include "../../libs/tiny_gltf.h"
#include <vector>

using json = nlohmann::json;
using namespace std;
#include "structs/floor.h"

uint8_t cubeIndices[] = {
    //Top
    2, 6, 7, 2, 3, 7,
    //Bottom
    0, 4, 5, 0, 1, 5,
    //Left
    0, 2, 6, 0, 4, 6,
    //Right
    1, 3, 7, 1, 5, 7,
    //Front
    0, 2, 3, 0, 1, 3,
    //Back
    4, 6, 7, 4, 5, 7
}; //36

float cubeVertices[] = {
    0, 0, 1, //0
    1, 0, 1, //1
    0, 1, 1, //2
    1, 1, 1, //3
    0, 0, 0, //4
    1, 0, 0, //5
    0, 1, 0, //6
    1, 1, 0  //7
}; //24
const int vertSize = 24 * sizeof(float);

void addCube(tinygltf::Model& m) {
    //Store Cube
    tinygltf::Buffer cubeBuffer;
    cubeBuffer.data.resize(vertSize + 36);
    memcpy(cubeBuffer.data.data(), cubeVertices, vertSize);
    memcpy(cubeBuffer.data.data() + vertSize, cubeIndices, 36);
    m.buffers.push_back(cubeBuffer);

    tinygltf::BufferView cubeVertVw;
    cubeVertVw.buffer = 0;
    cubeVertVw.byteOffset = 0;
    cubeVertVw.byteLength = vertSize;
    cubeVertVw.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    m.bufferViews.push_back(cubeVertVw);

    tinygltf::Accessor cubeVertAcc;
    cubeVertAcc.bufferView = 0;
    cubeVertAcc.byteOffset = 0;
    cubeVertAcc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    cubeVertAcc.count = 8;
    cubeVertAcc.type = TINYGLTF_TYPE_VEC3;
    //lineAcc.maxValues = { 1.0, 1.0, 0.0 };
    //lineAcc.minValues = { 0.0, 0.0, 0.0 };
    m.accessors.push_back(cubeVertAcc);

    tinygltf::BufferView cubeIdxVw;
    cubeIdxVw.buffer = 0;
    cubeIdxVw.byteOffset = vertSize;
    cubeIdxVw.byteLength = 36;
    cubeIdxVw.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
    m.bufferViews.push_back(cubeIdxVw);

    tinygltf::Accessor cubeIdxAcc;
    cubeIdxAcc.bufferView = 1;
    cubeIdxAcc.byteOffset = 0;
    cubeIdxAcc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
    cubeIdxAcc.count = 36;
    cubeIdxAcc.type = TINYGLTF_TYPE_SCALAR;
    //cubeIdxAcc.maxValues.push_back(2);
    //cubeIdxAcc.minValues.push_back(0);
    m.accessors.push_back(cubeIdxAcc);

    tinygltf::Primitive primitive;
    primitive.indices = 1;                 // The index of the accessor for the vertex indices
    primitive.attributes["POSITION"] = 0;  // The index of the accessor for positions
    primitive.mode = TINYGLTF_MODE_TRIANGLES;

    tinygltf::Mesh mesh;
    mesh.primitives.push_back(primitive);
    m.meshes.push_back(mesh);
}

int addLine(tinygltf::Model& m, const vector<float>& line) {
    int lineBytes = line.size() * sizeof(float);
    tinygltf::Buffer buffer;
    buffer.data.resize(lineBytes);
    memcpy(buffer.data.data(), line.data(), lineBytes);
    m.buffers.push_back(buffer);
    int bufIdx = m.buffers.size() - 1;

    tinygltf::BufferView lineVw;
    lineVw.buffer = bufIdx;
    lineVw.byteOffset = 0;
    lineVw.byteLength = lineBytes;
    lineVw.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
    m.bufferViews.push_back(lineVw);
    int vwIdx = m.bufferViews.size() - 1;

    tinygltf::Accessor lineAcc;
    lineAcc.bufferView = vwIdx;
    lineAcc.byteOffset = 0;
    lineAcc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    lineAcc.count = line.size() / 3;
    lineAcc.type = TINYGLTF_TYPE_VEC3;
    m.accessors.push_back(lineAcc);
    int accIdx = m.accessors.size() - 1;

    tinygltf::Primitive primitive;
    //primitive.indices = 1;                   // The index of the accessor for the vertex indices
    primitive.attributes["POSITION"] = accIdx; // The index of the accessor for positions
    primitive.mode = TINYGLTF_MODE_LINE_LOOP;
    tinygltf::Mesh mesh;
    mesh.primitives.push_back(primitive);
    m.meshes.push_back(mesh);
    return m.meshes.size() - 1;
}

void addColliders(tinygltf::Model& m, int roomIdx, roomStruct &room) {
    tinygltf::Node rootColl;
    rootColl.name = string("colliders_") + to_string(roomIdx);
    for (int collIdx = 0; collIdx < room.hardColTable.size(); collIdx++) {
        auto& coll = room.hardColTable[collIdx];
        tinygltf::Node collN;
        collN.name = string("coll_") + to_string(roomIdx) + "_" + to_string(collIdx);
        collN.mesh = 0;
        collN.translation = {
            -(double)coll.zv.ZVX1 / 1000,
            -(double)coll.zv.ZVY1 / 1000,
            (double)coll.zv.ZVZ1 / 1000
        };
        collN.scale = {
            -(double)coll.zv.ZVX2 / 1000 - collN.translation[0],
            -(double)coll.zv.ZVY2 / 1000 - collN.translation[1],
            (double)coll.zv.ZVZ2 / 1000 - collN.translation[2],
        };

        /*tinygltf::Value::Object extras;
        extras["collType"] = (tinygltf::Value)(int)zone.type;
        extras["collParameter"] = (tinygltf::Value)(int)zone.parameter;
        zoneN.extras = (tinygltf::Value)extras;*/

        /*zoneN.extras["collType"] = zone.parameter;
        zoneN.extras["collParameter"] = zone.parameter;*/
        m.nodes.push_back(collN);
        int collNIdx = m.nodes.size() - 1;
        rootColl.children.push_back(collNIdx);
    }
    m.nodes.push_back(rootColl);
}

void addZones(tinygltf::Model& m, int roomIdx, roomStruct& room) {
    tinygltf::Node rootZone;
    rootZone.name = string("zones_") + to_string(roomIdx);
    for (int zoneIdx = 0; zoneIdx < room.sceZoneTable.size(); zoneIdx++) {
        auto& zone = room.sceZoneTable[zoneIdx];
        tinygltf::Node zoneN;
        zoneN.name = string("zone_") + to_string(roomIdx) + "_" + to_string(zoneIdx);
        zoneN.mesh = 0;
        zoneN.translation = {
            -(double)zone.zv.ZVX1 / 1000,
            -(double)zone.zv.ZVY1 / 1000,
            (double)zone.zv.ZVZ1 / 1000
        };
        zoneN.scale = {
            -(double)zone.zv.ZVX2 / 1000 - zoneN.translation[0],
            -(double)zone.zv.ZVY2 / 1000 - zoneN.translation[1],
            (double)zone.zv.ZVZ2 / 1000 - zoneN.translation[2],
        };

        /*zoneN.extras["collType"] = zone.parameter;
        zoneN.extras["collParameter"] = zone.parameter;*/
        m.nodes.push_back(zoneN);
        int zoneNIdx = m.nodes.size() - 1;
        rootZone.children.push_back(zoneNIdx);
    }
    m.nodes.push_back(rootZone);
}

void saveFloorGLTF(floorStruct* floor, char* filename)
{
    //json flJson;
  // Create a model with a single mesh and save it as a gltf file
    tinygltf::Model m;

    tinygltf::Asset asset;
    m.asset.version = "2.0";
    m.asset.generator = "open-AITD";
    
    addCube(m);
    
    vector<tinygltf::Node> roomNodes(floor->rooms.size());
    for (int roomIdx = 0; roomIdx < floor->rooms.size(); roomIdx++) {
        auto& room = floor->rooms[roomIdx];
        auto& roomN = roomNodes[roomIdx];
        roomN.name = string("room_") + to_string(roomIdx);
        roomN.translation = {
            -(float)room.worldX / 100,
            -(float)room.worldY / 100,
            -(float)room.worldZ / 100
        };
        roomN.scale = { 1, 1, 1 };

        addColliders(m, roomIdx, room);
        int rootCollIdx = m.nodes.size() - 1;
        roomN.children.push_back(rootCollIdx);

        addZones(m, roomIdx, room);
        int rootZoneIdx = m.nodes.size() - 1;
        roomN.children.push_back(rootZoneIdx);

        //int roomNIdx = m.nodes.size() - 1;
    }

    for (int camIdx = 0; camIdx < floor->cameras.size(); camIdx++) {
        auto& cam = floor->cameras[camIdx];
        //tinygltf::Node camN;
        //camN.name = string("camera_") + to_string(camIdx);
        //add camera
        //m.nodes.push_back(camN);

        for (int viewIdx = 0; viewIdx < cam.viewedRoomTable.size(); viewIdx++) {
            auto& vw = cam.viewedRoomTable[viewIdx];            
            auto roomIdx = vw.viewedRoomIdx;
            auto& roomN = roomNodes[vw.viewedRoomIdx];
            
            for (int zoneIdx = 0; zoneIdx < vw.coverZones.size(); zoneIdx++) {
                auto& camZone = vw.coverZones[zoneIdx];
                vector<float> flzone(camZone.pointTable.size() * 3);
                for (int pIdx = 0; pIdx < camZone.pointTable.size(); pIdx++) {
                    auto& p = camZone.pointTable[pIdx];
                    flzone[pIdx * 3 + 0] = -(float)p.x / 100;// +roomN.translation[0];
                    flzone[pIdx * 3 + 1] = 0;
                    flzone[pIdx * 3 + 2] = (float)p.y / 100;// +roomN.translation[2];
                }
                auto lineMeshIdx = addLine(m, flzone);

                tinygltf::Node camZoneN;
                camZoneN.name = string("cam_zone_") + to_string(camIdx) + "_" + to_string(roomIdx) + "_" + to_string(zoneIdx);
                camZoneN.mesh = lineMeshIdx;
                m.nodes.push_back(camZoneN);
                int camZoneNIdx = m.nodes.size() - 1;
                roomN.children.push_back(camZoneNIdx);
            }
        }

    }

    for (int roomIdx = 0; roomIdx < floor->rooms.size(); roomIdx++) {
        m.nodes.push_back( roomNodes[roomIdx] );
    }


    //tinygltf::Node node;
    //node.mesh = 0;
    //node.scale = { 10, 5, 1 };
    //node.translation = { -5, 0, 0 };
    //m.nodes.push_back(node);

    //tinygltf::Scene scene; 
    //m.scenes.push_back(scene);
    //scene.nodes.push_back(0); // Default scene    

    // Create a simple material
    //tinygltf::Material mat;
    //mat.pbrMetallicRoughness.baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
    //mat.doubleSided = true;
    //m.materials.push_back(mat);

    // Save it to a file
    tinygltf::TinyGLTF gltf;
    gltf.WriteGltfSceneToFile(&m, filename,
        false, // embedImages
        true, // embedBuffers
        true, // pretty print
        false
    );

}