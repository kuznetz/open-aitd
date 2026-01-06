#pragma once
#include <string>
#include "structs/model.h"
#include "structs/animation.h"
#include <raymath.h>
#include "../utils/TriangulatePolygon.h"
#include "../loaders/loaders.h"
#include "../utils/my_gltf.h"
#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

inline Matrix roomMatMdl;

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
    //string matName = string("mat_")+ to_string(colorIdx) + "_" + to_string(subType);
    string matName = string("mat_") + to_string(colorIdx);
    for (int i = 0; i < m.materials.size(); i++) {
        if (m.materials[i].name == matName) {
            return i;
        }
    }
    tinygltf::Material newMat;
    newMat.name = matName;
    newMat.doubleSided = false;

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
    auto qX = QuaternionFromAxisAngle({ 1,0,0 }, -(float)bone.delta[0] * 2 * PI / 1024);
    auto qY = QuaternionFromAxisAngle({ 0,1,0 }, -(float)bone.delta[1] * 2 * PI / 1024);
    auto qZ = QuaternionFromAxisAngle({ 0,0,1 }, -(float)bone.delta[2] * 2 * PI / 1024);
    auto& q = QuaternionMultiply(QuaternionMultiply(qZ, qX), qY);
    //X  mirror
    q.x = -q.x;
    q.w = -q.w;

    //q = QuaternionTransform(q, roomMatMdl);
    return q;

    //Matrix mx = MatrixRotateX(bone.delta[0] * -2.f * PI / 1024);
    //Matrix my = MatrixRotateY(bone.delta[1] * -2.f * PI / 1024);
    //Matrix mz = MatrixRotateZ(bone.delta[2] * -2.f * PI / 1024);
    //Matrix matRotation = MatrixMultiply(MatrixMultiply(my, mx), mz);
    //matRotation = MatrixTranspose(matRotation);
    //return QuaternionTransform(QuaternionFromMatrix(matRotation), roomMatMdl);
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

void addAnimation(tinygltf::Model& m, Animation &anim, vector<Vector3>& bonePos, int maxBones) {
    const int bonesOffset = 0;
    vector<float> timeline;
    float curTime = 0;
    timeline.push_back(0);
    for (int i = 0; i < anim.frames.size(); i++) {
        auto& f = anim.frames[i];
        curTime += f.timestamp / 60.f;
        timeline.push_back(curTime);
    }

    if (maxBones > anim.frames[0].bones.size()) {
        maxBones = anim.frames[0].bones.size();
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
            
            //auto& v = m.nodes[j + bonesOffset].translation;
            eb.rotates[i+1] = QuaternionIdentity();
            eb.translates[i+1] = bonePos[j];
            //eb.translates[i] = { 0,0,0 };
            eb.scales[i+1] = { 1,1,1 };

            Vector3 v;
            switch (b.type) {
            case 0:
                eb.rotates[i+1] = GetAniRotation(b);
                break;
            case 1:
                //eb.translates[i] = { b.delta[0] / 1000.0f, b.delta[1] / 1000.0f, b.delta[2] / 1000.0f };
                eb.translates[i+1] = Vector3Add(bonePos[j], Vector3Transform(
                    { b.delta[0] / 1000.0f, b.delta[1] / 1000.0f, b.delta[2] / 1000.0f },
                    roomMatMdl));
                break;
            case 2:
                eb.scales[i+1] = { b.delta[0] / 256.0f + 1.0f, b.delta[1] / 256.0f + 1.0f, b.delta[2] / 256.0f + 1.0f };
                break;
            }
        }
    }

    for (int j = 0; j < expBones.size(); j++) {
        auto& eb = expBones[j];
        //int lastI = timeline.size() - 1;
        eb.rotates[0] = eb.rotates[1];
        eb.translates[0] = eb.translates[1];
        eb.scales[0] = eb.scales[1];
    }

    auto& t = expBones[0].translates;
    Vector3 pos = { 0,0,0 };
    for (int i = 0; i < anim.frames.size(); i++) {
        auto& f = anim.frames[i];
        pos = Vector3Add(pos,Vector3Transform(
            { f.offset[0] / 1000.0f, f.offset[1] / 1000.0f, f.offset[2] / 1000.0f },
            roomMatMdl
        ));
        t[i + 1] = pos;
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


tinygltf::Primitive createPrimitivePoly(tinygltf::Model& m, const PakModelPrimitive& prim, vector<Vector3>& modelVerts, int vertAccIdx) {
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
        //Strange order to wrap normals
        modelIdxs.emplace_back(idxMap[triangles[i].p0]);
        modelIdxs.emplace_back(idxMap[triangles[i].p2]);
        modelIdxs.emplace_back(idxMap[triangles[i].p1]);
    }

    if (modelIdxs.size() < 3) {
        throw new exception("modelIdxs.size() < 3");
    }

    return createPolyPrimitive(m, modelIdxs, vertAccIdx, matIdx);
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

Matrix mColToRow(const Matrix& m)
{
    Matrix result = {
        m.m0, m.m1, m.m2, m.m3,
        m.m4, m.m5, m.m6, m.m7,
        m.m8, m.m9, m.m10, m.m11,
        m.m12, m.m13, m.m14, m.m15,
    };
    return result;
}

int addBoneMatrices(tinygltf::Model& m, vector<Vector3>& position, int boneCount)
{
    vector<Matrix> boneMatrices(boneCount);
    for (int bIdx = 0; bIdx < boneCount; bIdx++) {
        auto& v = position[bIdx];
        boneMatrices[bIdx] = mColToRow(MatrixTranslate(-v.x, -v.y, -v.z));
        //boneMatrices[bIdx] = MatrixIdentity();
        //boneMatrices[bIdx] = MatrixTranslate(0, -v.y, 0);
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
    roomMatMdl = MatrixMultiply(MatrixRotateX(PI), MatrixRotateY(PI)); //MatrixIdentity();
    tinygltf::Model m;
    m.asset.version = "2.0";
    m.asset.generator = "open-AITD";
    
    vector<Vector3> modelVerts;
    modelVerts.resize(model.vertices.size() / 3);
    for (int i = 0; i < modelVerts.size(); i++) {
        modelVerts[i] = Vector3Transform({
            model.vertices[i * 3 + 0] / 1000.f,
            model.vertices[i * 3 + 1] / 1000.f,
            model.vertices[i * 3 + 2] / 1000.f
            }, roomMatMdl);
    }

    tinygltf::Skin skin;
    vector<Vector3> bonePos(model.bones.size());
    vector<u8> vecBoneAffect(modelVerts.size(), 0);
    if (model.bones.size()) {

        for (int bIdx = 0; bIdx < model.bones.size(); bIdx++) {
            skin.joints.push_back(bIdx);
            auto& bone = model.bones[bIdx];

            tinygltf::Node boneN;
            boneN.name = string("bone_") + to_string(bIdx);
            auto rIdx = bone.rootVertexIdx / 6;
            bonePos[bIdx] = modelVerts[rIdx];
            boneN.translation = { bonePos[bIdx].x, bonePos[bIdx].y, bonePos[bIdx].z };
            
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

        vector<Vector3> boneWorldPosition(model.bones.size());
        for (int bIdx = 0; bIdx < model.bones.size(); bIdx++) {
            int curBoneIdx = bIdx;
            Vector3 curVector = bonePos[curBoneIdx];
            while (true) {
                curBoneIdx = getParent(m, curBoneIdx);
                if (curBoneIdx == -1) break;
                curVector = Vector3Add(curVector, bonePos[curBoneIdx]);
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
            //!!! Not worked
            //tinygltf::Mesh mesh;
            //auto& prim = model.primitives[pIdx];
            //auto& prim = createPrimitivePoly(m, prim, modelVerts, vertAccIdx);

            //if (!mesh.primitives.size()) continue;
            //m.meshes.push_back(mesh);
            //auto meshIdx = m.meshes.size() - 1;

            //tinygltf::Node zoneN;
            //zoneN.name = string("model_" + to_string(pIdx));
            //zoneN.mesh = meshIdx;
            //m.nodes.push_back(zoneN);
        }
    }
    else 
    {
        //=== single mesh ===
        tinygltf::Mesh mesh;
        VertexSkin vSkin = { -1,-1 };
        if (model.bones.size()) {
            vSkin = addVertexSkin(m, vecBoneAffect);
        }

        for (int pIdx = 0; pIdx < model.primitives.size(); pIdx++)
        {
            auto& prim = model.primitives[pIdx];
            if (prim.type != 1) continue;
            auto& prim2 = createPrimitivePoly(m, prim, modelVerts, vertAccIdx);
            if (model.bones.size()) {
                prim2.attributes["JOINTS_0"] = vSkin.jointsAccIdx;
                prim2.attributes["WEIGHTS_0"] = vSkin.weightsAccIdx;
            }
            mesh.primitives.push_back(prim2);
        }

        for (int pIdx = 0; pIdx < model.primitives.size(); pIdx++)
        {
            auto& prim = model.primitives[pIdx];
            if (prim.type != 3) continue;
            const int vertCount = 5;
            auto matIdx = getMaterialIdx(m, prim.colorIndex, prim.subType);
            auto& pos = modelVerts[prim.vertexIdxs[0] / 6];
            float size = prim.size / 1000.0f;
            auto& prim2 = createSpherePrim(m, size, vertCount, pos, matIdx);
            if (model.bones.size()) {
                int VertCountFull = vertCount * vertCount * 6;
                vector<u8> vecBoneAffect2(VertCountFull, vecBoneAffect[prim.vertexIdxs[0] / 6]);
                auto vSkin2 = addVertexSkin(m, vecBoneAffect2);
                prim2.attributes["JOINTS_0"] = vSkin2.jointsAccIdx;
                prim2.attributes["WEIGHTS_0"] = vSkin2.weightsAccIdx;
            }
            mesh.primitives.push_back(prim2);
        }

        //Lines
        for (int pIdx = 0; pIdx < model.primitives.size(); pIdx++)
        {
            auto& prim = model.primitives[pIdx];
            if (prim.type != 0) continue;
            if (prim.vertexIdxs.size() != 2) {
                throw new exception("Line indexes not 2");
            }
            auto matIdx = getMaterialIdx(m, prim.colorIndex, 0);
            Vector3 points[2] = {
                modelVerts[prim.vertexIdxs[0] / 6],
                modelVerts[prim.vertexIdxs[1] / 6]
            };
            auto& prim2 = createPipePrim(m, points, 0.005f, 4, matIdx);
            if (model.bones.size()) {
                vector<u8> vecBoneAffect2(8);
                for (int i = 0; i < 4; i++) {
                    vecBoneAffect2[i] = vecBoneAffect[prim.vertexIdxs[0] / 6];
                }
                for (int i = 4; i < 8; i++) {
                    vecBoneAffect2[i] = vecBoneAffect[prim.vertexIdxs[1] / 6];
                }
                auto vSkin2 = addVertexSkin(m, vecBoneAffect2);
                prim2.attributes["JOINTS_0"] = vSkin2.jointsAccIdx;
                prim2.attributes["WEIGHTS_0"] = vSkin2.weightsAccIdx;
            }
            mesh.primitives.push_back(prim2);
        }

        //1x1 pixel, 2x2 square, NxN square, size depends projected z-value
        for (int pIdx = 0; pIdx < model.primitives.size(); pIdx++)
        {
            auto& prim = model.primitives[pIdx];
            float size = 0.01f;
            if (prim.type == 2) {
            } else if (prim.type == 6) {
                size = 0.02f;
            } else if (prim.type == 7) {
                size = 0.1f;
            } else continue;

            auto matIdx = getMaterialIdx(m, prim.colorIndex, 0);
            auto& prim2 = createCubePrim(m, modelVerts[prim.vertexIdxs[0] / 6], { size,size,size }, matIdx);
            if (model.bones.size()) {
                vector<u8> vecBoneAffect2(8);
                for (int i = 0; i < 8; i++) {
                    vecBoneAffect2[i] = vecBoneAffect[prim.vertexIdxs[0] / 6];
                }
                auto vSkin2 = addVertexSkin(m, vecBoneAffect2);
                prim2.attributes["JOINTS_0"] = vSkin2.jointsAccIdx;
                prim2.attributes["WEIGHTS_0"] = vSkin2.weightsAccIdx;
            }
            mesh.primitives.push_back(prim2);
        }

        m.meshes.push_back(mesh);
        auto meshIdx = m.meshes.size()-1;

        tinygltf::Node zoneN;
        zoneN.name = string("model");
        zoneN.mesh = meshIdx;
        m.nodes.push_back(zoneN);
        int nodeSkinIdx = m.nodes.size() - 1;

        if (model.bones.size()) {
            m.nodes[nodeSkinIdx].skin = 0;
            skin.skeleton = 0;
            m.skins.push_back(skin);

            for (int i = 0; i < animations.size(); i++) {
                //if (animations[i]->id != 262) continue;
                addAnimation(m, *animations[i], bonePos, model.bones.size());
            }
        }
    }

    tinygltf::TinyGLTF gltf;
    std::filesystem::create_directories(dirname);
    gltf.WriteGltfSceneToFile(&m, dirname + "/model.gltf",
       false, // embedImages
       false, // embedBuffers
       false, // pretty print
       false  // binary (glb)
    );

    Vector3 v1 = Vector3Transform({
        model.bounds.ZVX1 / 1000.f ,
        model.bounds.ZVY1 / 1000.f ,
        model.bounds.ZVZ1 / 1000.f
        }, roomMatMdl);
    Vector3 v2 = Vector3Transform({
        model.bounds.ZVX2 / 1000.f ,
        model.bounds.ZVY2 / 1000.f ,
        model.bounds.ZVZ2 / 1000.f
        }, roomMatMdl);

    json dataJson;
    dataJson["bounds"] = json::array();
    dataJson["bounds"].push_back(v1.x);
    dataJson["bounds"].push_back(v1.y);
    dataJson["bounds"].push_back(v1.z);
    dataJson["bounds"].push_back(v2.x);
    dataJson["bounds"].push_back(v2.y);
    dataJson["bounds"].push_back(v2.z);
    std::ofstream o(dirname + "/data.json");
    o << std::setw(2) << dataJson;
}