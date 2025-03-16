#pragma once
#include <string>
#include "structs/model.h"
#include <raymath.h>
#include "TriangulatePolygon.h"
#include "my_gltf.h"

using namespace std;

void saveModelGLTF(const PakModel& model, const string dirname)
{
    tinygltf::Model m;
    m.asset.version = "2.0";
    m.asset.generator = "open-AITD";  

    for (int pIdx = 0; pIdx < model.primitives.size(); pIdx++) {

        vector<Vector3> modelVerts;
        vector<unsigned int> modelIdxs;

        auto& prim = model.primitives[pIdx];
        if (prim.type == 1) {

            std::vector<triangulate::Point> polygon;
            for (int i = 0; i < prim.vertexIdxs.size(); i++) {
                int vIdx = prim.vertexIdxs[i] / 6;
                auto qwe = (float)model.vertices[vIdx * 3 + 0];
                Vector3 vec = {
                    (float)model.vertices[vIdx * 3 + 0] / 1000,
                    -(float)model.vertices[vIdx * 3 + 1] / 1000,
                    (float)model.vertices[vIdx * 3 + 2] / 1000
                };
                modelVerts.push_back({ vec.x, vec.y, vec.z });
                polygon.emplace_back(vec.x, vec.y, vec.z);
            }
            const auto triangles = triangulate::triangulate(polygon);
            for (int i = 0; i < triangles.size(); i++) {
                modelIdxs.emplace_back(triangles[i].p0);
                modelIdxs.emplace_back(triangles[i].p1);
                modelIdxs.emplace_back(triangles[i].p2);                
            }
            auto meshIdx = createPolyMesh( m, modelVerts, modelIdxs );

            tinygltf::Node zoneN;
            zoneN.name = string("prim_") + to_string(pIdx);
            zoneN.mesh = meshIdx;
            m.nodes.push_back(zoneN);
        }
    }

    tinygltf::TinyGLTF gltf;
    gltf.WriteGltfSceneToFile(&m, dirname + "/model.gltf",
       false, // embedImages
       true, // embedBuffers
       true, // pretty print
       false
    );
}