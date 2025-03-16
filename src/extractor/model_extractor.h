#pragma once
#include <string>
#include "structs/model.h"
#include <raymath.h>
#include "TriangulatePolygon.h"
#include "palette.h"
#include "my_gltf.h"

using namespace std;

void ComputeUV(vector<Vector3>& allVertices, vector<int>& polyVertices, Vector3& forward, Vector3& left)
{
    int lastPoly = polyVertices.size() - 1;
    Vector3 up;
    do
    {
        Vector3 a = allVertices[polyVertices[0]];
        Vector3 b = allVertices[polyVertices[1]];
        Vector3 c = allVertices[polyVertices[lastPoly]];
        left = Vector3Normalize(Vector3Subtract(b, a));
        forward = Vector3Normalize(Vector3Subtract(c, a));
        
        up = Vector3Normalize(Vector3CrossProduct(left, forward));
        left = Vector3Normalize(Vector3CrossProduct(up, forward));
        lastPoly--;
    } while (lastPoly > 1);
}

int getMaterialIdx(tinygltf::Model& m, u8 colorIdx, u8 subType = 0)
{
    string matName = string("mat_")+ to_string(colorIdx) + "_" + to_string(subType);
    for (int i = 0; i < m.materials.size(); i++) {
        if (m.materials[i].name == matName) return i;
    }
    tinygltf::Material newMat;
    newMat.name = matName;
    newMat.doubleSided = true;

    auto col = getPalColor(colorIdx);
    newMat.pbrMetallicRoughness.baseColorFactor = { 
        (float)col[0] / 255,
        (float)col[1] / 255,
        (float)col[2] / 255,
        1 };
    newMat.pbrMetallicRoughness.metallicFactor = 0;
    newMat.pbrMetallicRoughness.roughnessFactor = 1;
    //newMat.alphaMode
    m.materials.push_back(newMat);
    return m.materials.size() - 1;
}

void saveModelGLTF(const PakModel& model, const string dirname)
{
    tinygltf::Model m;
    m.asset.version = "2.0";
    m.asset.generator = "open-AITD";  

    vector<Vector3> modelVerts;
    modelVerts.resize(model.vertices.size() / 3);
    for (int i = 0; i < modelVerts.size(); i++) {
        modelVerts[i] = {
            (float)model.vertices[i * 3 + 0] / 1000,
            -(float)model.vertices[i * 3 + 1] / 1000,
            (float)model.vertices[i * 3 + 2] / 1000
        };
    }
    int vertsIdx = createVertexes(m, modelVerts);

    tinygltf::Mesh mesh;
    for (int pIdx = 0; pIdx < model.primitives.size(); pIdx++) {
        auto& prim = model.primitives[pIdx];

        //prim.subType
        //prim.colorIndex

        if (prim.type == 1) {
            auto matIdx = getMaterialIdx(m, prim.colorIndex, prim.subType);

            /*
            UV:
                ComputeUV(polyVertices, out forward, out left);
                foreach (int pointIndex in polyVertices)
                {
                    Vector3 poly = allVertices[pointIndex];
                    uv.Add(new Vector2(
                        Vector3.Dot(poly, left) * noisesize,
                        Vector3.Dot(poly, forward) * noisesize
                    ));
                }
            */

            std::vector<int> idxMap(prim.vertexIdxs.size());
            std::vector<triangulate::Point> polygon;
            for (int i = 0; i < prim.vertexIdxs.size(); i++) {
                int vIdx = prim.vertexIdxs[i] / 6;
                idxMap[i] = vIdx;
                auto qwe = (float)model.vertices[vIdx * 3 + 0];
                Vector3& vec = modelVerts[vIdx];
                polygon.emplace_back(vec.x, vec.y, vec.z);
            }
            const auto triangles = triangulate::triangulate(polygon);
            vector<unsigned int> modelIdxs;
            for (int i = 0; i < triangles.size(); i++) {
                modelIdxs.emplace_back(idxMap[triangles[i].p0]);
                modelIdxs.emplace_back(idxMap[triangles[i].p1]);
                modelIdxs.emplace_back(idxMap[triangles[i].p2]);
            }
            auto prim = createPolyPrimitive(m, modelIdxs, vertsIdx, matIdx);
            mesh.primitives.push_back(prim);
        }

    }
    m.meshes.push_back(mesh);
    auto meshIdx = m.meshes.size()-1;

    tinygltf::Node zoneN;
    zoneN.name = string("model");
    zoneN.mesh = meshIdx;
    m.nodes.push_back(zoneN);

    tinygltf::TinyGLTF gltf;
    gltf.WriteGltfSceneToFile(&m, dirname + "/model.gltf",
       false, // embedImages
       true, // embedBuffers
       true, // pretty print
       false
    );
}