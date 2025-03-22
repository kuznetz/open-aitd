#pragma once
#include <string>
#include "structs/model.h"
#include "structs/animation.h"
#include <raymath.h>
#include "TriangulatePolygon.h"
#include "../loaders/loaders.h"
#include "../utils/my_gltf.h"

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

Quaternion GetAniRotation(AniBone& bone)
{
    //zxy
    auto q1 = QuaternionFromAxisAngle({ 0,0,1 }, -(float)bone.delta[2] * 2 * PI / 1024);
    auto q2 = QuaternionFromAxisAngle({ 1,0,0 }, -(float)bone.delta[0] * 2 * PI / 1024);
    auto q3 = QuaternionFromAxisAngle({ 0,1,0 }, -(float)bone.delta[1] * 2 * PI / 1024);
    return QuaternionMultiply(QuaternionMultiply(q1, q2), q3);
}

struct boneAnimExp {
    int type;
    vector<Vector4> rotates;
    vector<Vector3> translates;
    vector<Vector3> scales;
};

void addAnimation(tinygltf::Model& m, Animation &anim) {
    vector<float> timeline;
    vector<boneAnimExp> expBones;

    expBones.resize(anim.frames[0].bones.size());
    for (int j = 0; j < anim.frames[0].bones.size(); j++) {
        expBones[j].type = anim.frames[0].bones[j].type;
        switch (expBones[j].type) {
        case 0:
            expBones[j].rotates.resize(anim.frames.size());
            break;
        case 1:
            expBones[j].translates.resize(anim.frames.size());
            break;
        case 2:
            expBones[j].scales.resize(anim.frames.size());
            break;
        }
    }

    for (int i = 0; i < anim.frames.size(); i++) {
        auto& f = anim.frames[i];
        timeline.push_back((float)f.timestamp / 50);
        for (int j = 0; j < f.bones.size(); j++) {
            auto& b = f.bones[j];
            auto& eb = expBones[j];
            if (b.type != eb.type) throw new exception("Type changed");
            switch (eb.type) {
            case 0:
                eb.rotates[i] = GetAniRotation(b);
                break;
            case 1:
                //expBones[j].translates.resize(anim.frames.size());
                break;
            case 2:
                //expBones[j].scales.resize(anim.frames.size());
                break;
            }
        }
    }

    int vwTime = createBufferAndView(m, timeline.data(), timeline.size() * sizeof(float), 0);
    tinygltf::Accessor accTime;
    accTime.bufferView = vwTime;
    accTime.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    accTime.count = timeline.size();
    accTime.type = TINYGLTF_TYPE_SCALAR;
    m.accessors.push_back(accTime);
    int accTimeIdx = m.accessors.size() - 1;

    tinygltf::Animation outAni;
    outAni.samplers.resize(expBones.size());
    outAni.channels.resize(expBones.size());

    for (int i = 0; i < expBones.size(); i++) {
        int vwData = 0;
        int accDataType = TINYGLTF_TYPE_VEC3;
        string aniTargetPath = "rotation";
        switch (expBones[i].type) {
        case 0:
            vwData = createBufferAndView(m, expBones[i].rotates.data(), timeline.size() * 4 * sizeof(float), 0);
            accDataType = TINYGLTF_TYPE_VEC4;
            break;
        case 1:
            break;
        case 2:
            break;
        }

        tinygltf::Accessor accData;
        accData.bufferView = vwData;
        accData.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
        accData.count = timeline.size();
        accData.type = accDataType;
        m.accessors.push_back(accData);
        int accDataIdx = m.accessors.size() - 1;

        outAni.samplers[i].input = accTimeIdx;
        outAni.samplers[i].output = accDataIdx;
        outAni.channels[i].sampler = i;
        outAni.channels[i].target_node = i + 1;
        outAni.channels[i].target_path = "rotation";
    }

    m.animations.push_back(outAni);
}

void saveModelGLTF(const PakModel& model, vector<Animation*> animations, const string dirname)
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

    if (model.bones.size()) {
        vector<u8> vecBoneAffect(model.vertices.size(), 0);
        tinygltf::Skin skin;

        for (int bIdx = 0; bIdx < model.bones.size(); bIdx++) {
            skin.joints.push_back(bIdx + 1);
            auto& bone = model.bones[bIdx];
            tinygltf::Node boneN;
            auto rIdx = bone.rootVertexIdx / 6;
            boneN.translation = {
                (float)model.vertices[rIdx * 3 + 0] / 1000,
                -(float)model.vertices[rIdx * 3 + 1] / 1000,
                (float)model.vertices[rIdx * 3 + 2] / 1000
            };
            for (int j = 0; j < model.bones.size(); j++) {
                if (bIdx == j) continue;
                if (model.bones[j].parentBoneIdx == model.bones[bIdx].boneIdx) {
                    boneN.children.push_back(j + 1);
                }
            }
            m.nodes.push_back(boneN);
            int vfrom = (bone.fromVertexIdx / 6);
            for (int j = vfrom; j < vfrom+bone.vertexCount; j++) {
                vecBoneAffect[j] = bIdx;
            }
        }

        m.skins.push_back(skin);
        m.nodes[0].skin = 0;
        addVertexSkin(m, vecBoneAffect);
    }

    for (int i = 0; i < animations.size(); i++) {
        addAnimation(m, *animations[i]);
    }

    tinygltf::TinyGLTF gltf;
    gltf.WriteGltfSceneToFile(&m, dirname + "/model.gltf",
       false, // embedImages
       true, // embedBuffers
       true, // pretty print
       false
    );
}