#pragma once
#include <string>
#include "structs/model.h"
#include "structs/animation.h"
#include <raymath.h>
#include "TriangulatePolygon.h"
#include "../loaders/loaders.h"
#include "../utils/my_gltf.h"
#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

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
    vector<Vector4> rotates;
    vector<Vector3> translates;
    vector<Vector3> scales;
};

void addAniRotation(tinygltf::Model& m, tinygltf::Animation& outAni, vector<Vector4>& rotates, int accTimeIdx, int boneIdx) {
    tinygltf::Accessor accData;
    accData.bufferView = createBufferAndView(m, rotates.data(), rotates.size() * 4 * sizeof(float), 0);
    accData.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    accData.count = rotates.size();
    accData.type = TINYGLTF_TYPE_VEC4;
    m.accessors.push_back(accData);
    int accDataIdx = m.accessors.size() - 1;

    tinygltf::AnimationSampler smp;
    smp.input = accTimeIdx;
    smp.output = accDataIdx;
    outAni.samplers.push_back(smp);

    tinygltf::AnimationChannel chn;
    chn.sampler = outAni.samplers.size() -1;
    chn.target_node = boneIdx;
    chn.target_path = "rotation";
    outAni.channels.push_back(chn);
}

void addAniTranslation(tinygltf::Model& m, tinygltf::Animation& outAni, vector<Vector3>& translates, int accTimeIdx, int boneIdx) {
    tinygltf::Accessor accData;
    accData.bufferView = createBufferAndView(m, translates.data(), translates.size() * 4 * sizeof(float), 0);
    accData.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    accData.count = translates.size();
    accData.type = TINYGLTF_TYPE_VEC3;
    m.accessors.push_back(accData);
    int accDataIdx = m.accessors.size() - 1;

    tinygltf::AnimationSampler smp;
    smp.input = accTimeIdx;
    smp.output = accDataIdx;
    outAni.samplers.push_back(smp);

    tinygltf::AnimationChannel chn;
    chn.sampler = outAni.samplers.size() - 1;
    chn.target_node = boneIdx;
    chn.target_path = "translation";
    outAni.channels.push_back(chn);
}

void addAniScale(tinygltf::Model& m, tinygltf::Animation& outAni, vector<Vector3>& scales, int accTimeIdx, int boneIdx) {
    tinygltf::Accessor accData;
    accData.bufferView = createBufferAndView(m, scales.data(), scales.size() * 4 * sizeof(float), 0);
    accData.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    accData.count = scales.size();
    accData.type = TINYGLTF_TYPE_VEC3;
    m.accessors.push_back(accData);
    int accDataIdx = m.accessors.size() - 1;

    tinygltf::AnimationSampler smp;
    smp.input = accTimeIdx;
    smp.output = accDataIdx;
    outAni.samplers.push_back(smp);

    tinygltf::AnimationChannel chn;
    chn.sampler = outAni.samplers.size() - 1;
    chn.target_node = boneIdx;
    chn.target_path = "scale";
    outAni.channels.push_back(chn);
}

void addAnimation(tinygltf::Model& m, Animation &anim) {
    const int bonesOffset = 0;
    vector<float> timeline;
    float curTime = 0;
    timeline.push_back(0);
    for (int i = 0; i < anim.frames.size(); i++) {
        auto& f = anim.frames[i];
        curTime += f.timestamp / 60.f;
        timeline.push_back(curTime);
    }

    int maxBones = anim.frames[0].bones.size();
    if (maxBones > m.nodes.size() - 1) {
        maxBones = m.nodes.size() - 1;
    }

    vector<boneAnimExp> expBones;
    expBones.resize(maxBones);
    for (int j = 0; j < maxBones; j++) {
        expBones[j].rotates.resize(timeline.size());
        expBones[j].translates.resize(timeline.size());
        expBones[j].scales.resize(timeline.size());
    }

    for (int i = 0; i < anim.frames.size(); i++) {
        auto& f = anim.frames[i];
        for (int j = 0; j < maxBones; j++) {
            auto& b = f.bones[j];
            auto& eb = expBones[j];
            
            eb.rotates[i] = QuaternionIdentity();
            auto& v = m.nodes[j + bonesOffset].translation;
            eb.translates[i] = { (float)v[0], (float)v[1], (float)v[2] };
            //eb.translates[i] = { 0,0,0 };
            eb.scales[i] = { 1,1,1 };

            switch (b.type) {
            case 0:
                eb.rotates[i] = GetAniRotation(b);
                break;
            case 1:
                eb.translates[i] = { b.delta[0] / 1000.0f, -b.delta[1] / 1000.0f, b.delta[2] / 1000.0f};
                break;
            case 2:
                eb.scales[i] = { b.delta[0] / 256.0f + 1.0f, b.delta[1] / 256.0f + 1.0f, b.delta[2] / 256.0f + 1.0f };
                break;
            }
        }
    }

    for (int j = 0; j < expBones.size(); j++) {
        auto& eb = expBones[j];
        int lastI = timeline.size() - 1;
        eb.rotates[lastI] = eb.rotates[0];
        eb.translates[lastI] = eb.translates[0];
        eb.scales[lastI] = eb.scales[0];
    }

    //First to last

    int vwTime = createBufferAndView(m, timeline.data(), timeline.size() * sizeof(float), 0);
    tinygltf::Accessor accTime;
    accTime.bufferView = vwTime;
    accTime.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    accTime.count = timeline.size();
    accTime.type = TINYGLTF_TYPE_SCALAR;
    m.accessors.push_back(accTime);
    int accTimeIdx = m.accessors.size() - 1;

    tinygltf::Animation outAni;
    outAni.name = string("a_" + to_string(anim.id));

    for (int i = 0; i < expBones.size(); i++) {
        addAniRotation(m, outAni, expBones[i].rotates, accTimeIdx, i + bonesOffset);
        addAniTranslation(m, outAni, expBones[i].translates, accTimeIdx, i + bonesOffset);
        addAniScale(m, outAni, expBones[i].scales, accTimeIdx, i + bonesOffset);
    }

    m.animations.push_back(outAni);
}

vector<triangulate::TriangleIdx> triangulate1(int numPoints) {
    vector<triangulate::TriangleIdx> result;

    int v0 = 0;
    int v1 = 1;
    int v2 = numPoints - 1;
    bool swap = true;

    while (v1 < v2)
    {
        result.push_back({ v0,v1,v2 });
        if (swap)
        {
            v0 = v1;
            v1++;
        }
        else
        {
            v0 = v2;
            v2--;
        }

        swap = !swap;
    }

    return result;
}

/*
vector triangulate2(int numPoints) {
    std::vector<triangulate::Point> polygon;
    for (int i = 0; i < prim.vertexIdxs.size(); i++) {
        int vIdx = prim.vertexIdxs[i] / 6;
        idxMap[i] = vIdx;
        //auto qwe = (float)model.vertices[vIdx * 3 + 0];
        Vector3& vec = modelVerts[vIdx];
        polygon.emplace_back(vec.x, vec.y, vec.z);
    }
    const auto triangles = triangulate::triangulate(polygon);
}
*/


void addPrimitive(tinygltf::Model& m, tinygltf::Mesh& mesh, const PakModelPrimitive& prim, vector<Vector3>& modelVerts, int vertAccIdx) {
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
        for (int i = 0; i < prim.vertexIdxs.size(); i++) {
            int vIdx = prim.vertexIdxs[i] / 6;
            if (vIdx >= modelVerts.size()) {
                throw new exception("vIdx >= modelVerts.size()");
            }
            idxMap[i] = vIdx;
        }

        const auto& triangles = triangulate1(prim.vertexIdxs.size());

        vector<unsigned int> modelIdxs;
        for (int i = 0; i < triangles.size(); i++) {
            modelIdxs.emplace_back(idxMap[triangles[i].p0]);
            modelIdxs.emplace_back(idxMap[triangles[i].p1]);
            modelIdxs.emplace_back(idxMap[triangles[i].p2]);
        }

        if (modelIdxs.size() < 3) {
            throw new exception("modelIdxs.size() < 3");
        }

        tinygltf::Primitive prim2 = createPolyPrimitive(m, modelIdxs, vertAccIdx, matIdx);
        mesh.primitives.push_back(prim2);
    }
}

int getParent(tinygltf::Model& m, int childIdx) {
    for (int i = 0; i < m.nodes.size(); i++) {
        auto& chld = m.nodes[i].children;
        if (std::find(chld.begin(), chld.end(), childIdx) != chld.end()) {
            return i;
        }
    }
    return -1;
}

int addBoneMatrices(tinygltf::Model& m, map<int, Vector3>& boneWorldPosition, int boneCount)
{
    vector<Matrix> boneMatrices(boneCount);
    for (int bIdx = 0; bIdx < boneCount; bIdx++) {
        auto& v = boneWorldPosition[bIdx];
        boneMatrices[bIdx] = MatrixTranslate(-v.x*10, -v.y*20 + 10, -v.z*10);
    }

    int boneMatBytes = boneCount * sizeof(Matrix);
    int boneMatOffs = addDataToBuffer(m, boneMatrices.data(), boneMatBytes);

    tinygltf::BufferView boneMatVw;
    boneMatVw.buffer = 0;
    boneMatVw.byteOffset = boneMatOffs;
    boneMatVw.byteLength = boneMatBytes;
    m.bufferViews.push_back(boneMatVw);

    tinygltf::Accessor boneMatAcc;
    boneMatAcc.bufferView = m.bufferViews.size() - 1;
    boneMatAcc.byteOffset = 0;
    boneMatAcc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    boneMatAcc.count = boneCount;
    boneMatAcc.type = TINYGLTF_TYPE_MAT4;
    m.accessors.push_back(boneMatAcc);

    return m.accessors.size() - 1;
}

void saveModelGLTF(const PakModel& model, vector<Animation*> animations, const string dirname)
{
    const bool splitPrimitives = false;
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

    tinygltf::Skin skin;
    vector<u8> vecBoneAffect(modelVerts.size(), 0);

    if (model.bones.size()) {

        for (int bIdx = 0; bIdx < model.bones.size(); bIdx++) {
            skin.joints.push_back(bIdx);
            auto& bone = model.bones[bIdx];

            tinygltf::Node boneN;
            auto rIdx = bone.rootVertexIdx / 6;
            boneN.translation = {
                (float)model.vertices[rIdx * 3 + 0] / 1000,
                -(float)model.vertices[rIdx * 3 + 1] / 1000,
                (float)model.vertices[rIdx * 3 + 2] / 1000
            };
            
            //Add children
            for (int j = 0; j < model.bones.size(); j++) {
                if (bIdx == j) continue;
                if (model.bones[j].parentBoneIdx == model.bones[bIdx].boneIdx) {
                    boneN.children.push_back(j);
                }
            }

            m.nodes.push_back(boneN);
            int vfrom = (bone.fromVertexIdx / 6);
            for (int j = vfrom; j < vfrom+bone.vertexCount; j++) {
                vecBoneAffect[j] = bIdx;
            }
        }

        map<int, Vector3> boneWorldPosition;
        for (int bIdx = 0; bIdx < model.bones.size(); bIdx++) {
            int curBoneIdx = bIdx;
            auto& boneT = m.nodes[curBoneIdx].translation;
            Vector3 curVector = { (float)boneT[0], (float)boneT[1], (float)boneT[2] };
            while (true) {
                curBoneIdx = getParent(m, curBoneIdx);
                if (curBoneIdx == -1) break;
                auto& T = m.nodes[curBoneIdx].translation;
                Vector3 v = { (float)T[0], (float)T[1], (float)T[2] };
                curVector = Vector3Add(curVector, v);
            }
            boneWorldPosition[bIdx] = curVector;
        }

        for (int i = 0; i < vecBoneAffect.size(); i++ ) {
            auto& v = boneWorldPosition[vecBoneAffect[i]];
            modelVerts[i] = Vector3Add(modelVerts[i], v);
        }

        skin.inverseBindMatrices = addBoneMatrices(m, boneWorldPosition, model.bones.size());
    }

    int vertAccIdx = createVertexes(m, modelVerts);

    if (splitPrimitives)
    {
        //=== multi mesh (for test) ===
        for (int pIdx = 0; pIdx < model.primitives.size(); pIdx++)
        {
            tinygltf::Mesh mesh;
            auto& prim = model.primitives[pIdx];
            //auto& prim = model.primitives[model.primitives.size()-1];
            addPrimitive(m, mesh, prim, modelVerts, vertAccIdx);

            if (!mesh.primitives.size()) continue;
            m.meshes.push_back(mesh);
            auto meshIdx = m.meshes.size() - 1;

            tinygltf::Node zoneN;
            zoneN.name = string("model_" + to_string(pIdx));
            zoneN.mesh = meshIdx;
            m.nodes.push_back(zoneN);
        }
    }
    else 
    {
        //=== single mesh ===
        tinygltf::Mesh mesh;
        for (int pIdx = 0; pIdx < model.primitives.size(); pIdx++)
        {
            auto& prim = model.primitives[pIdx];
            //auto& prim = model.primitives[model.primitives.size()-1];
            addPrimitive(m, mesh, prim, modelVerts, vertAccIdx);
        }
        m.meshes.push_back(mesh);
        auto meshIdx = m.meshes.size()-1;

        tinygltf::Node zoneN;
        zoneN.name = string("model");
        zoneN.mesh = meshIdx;
        m.nodes.push_back(zoneN);
        int nodeSkinIdx = m.nodes.size() - 1;

        if (model.bones.size()) {
            //addVertexSkin(m, vecBoneAffect);
            //m.nodes[nodeSkinIdx].skin = 0;
            //skin.skeleton = 0;
            //m.skins.push_back(skin);

            /*for (int i = 0; i < animations.size(); i++) {
                addAnimation(m, *animations[i]);
            }*/
        }
    }

    tinygltf::TinyGLTF gltf;
    gltf.WriteGltfSceneToFile(&m, dirname + "/model.gltf",
       false, // embedImages
       false, // embedBuffers
       false, // pretty print
       false
    );
}