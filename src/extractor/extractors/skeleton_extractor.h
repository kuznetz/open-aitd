#pragma once
#include <string>
#include "../structs/model.h"
#include "../structs/animation.h"
#include <raymath.h>
#include "../loaders/loaders.h"
#include "../utils/my_gltf.h"
#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1
#include <nlohmann/json.hpp>

using namespace std;
using nlohmann::json;

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

int getParent(tinygltf::Model& m, int childIdx) {
    for (int i = 0; i < m.nodes.size(); i++) {
        auto& chld = m.nodes[i].children;
        if (std::find(chld.begin(), chld.end(), childIdx) != chld.end()) {
            return i;
        }
    }
    return -1;
}

int addBoneMatrices(tinygltf::Model& m, vector<Vector3>& bonePos, vector<Vector4>& boneRot, int boneCount)
{
    vector<Matrix> boneMatrices(boneCount);
    for (int bIdx = 0; bIdx < boneCount; bIdx++) {
        auto& v = bonePos[bIdx];
        boneMatrices[bIdx] = MatrixIdentity();
        //boneMatrices[bIdx] = QuaternionToMatrix(boneRot[bIdx]);
        //boneMatrices[bIdx] = MatrixTranslate(-v.x, -v.y, -v.z);
        //MatrixTranslate(v.x, v.y, v.z);
        //boneMatrices[bIdx] = MatrixScale(0.5 * bIdx, 0.5 * bIdx, 0.5 * bIdx);
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
    if (!model.bones.size()) return;

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

    vector<Vector3> bonePos(model.bones.size());
    for (int bIdx = 0; bIdx < model.bones.size(); bIdx++) {
        skin.joints.push_back(bIdx);
        auto& bone = model.bones[bIdx];

        tinygltf::Node boneN;
        auto rIdx = bone.rootVertexIdx / 6;
        bonePos[bIdx] = {
            (float)model.vertices[rIdx * 3 + 0] / 1000,
            -(float)model.vertices[rIdx * 3 + 1] / 1000,
            (float)model.vertices[rIdx * 3 + 2] / 1000
        };
        boneN.translation = { bonePos[bIdx].x, bonePos[bIdx].y, bonePos[bIdx].z };
        //boneN.rotation = { 0, 0, 0, 1 };
            
        //Add children
        for (int j = 0; j < model.bones.size(); j++) {
            if (bIdx == j) continue;
            if (model.bones[j].parentBoneIdx == model.bones[bIdx].boneIdx) {
                boneN.children.push_back(j);
            }
        }

        m.nodes.push_back(boneN);
    }

    map<int, Vector3> boneWorldPos;
    for (int bIdx = 0; bIdx < model.bones.size(); bIdx++) {
        int curBoneIdx = bIdx;
        auto& boneT = bonePos[curBoneIdx];
        Vector3 curVector = { (float)boneT.x, (float)boneT.y, (float)boneT.z };
        while (true) {
            curBoneIdx = getParent(m, curBoneIdx);
            if (curBoneIdx == -1) break;
            auto& T = bonePos[curBoneIdx];
            Vector3 v = { (float)T.x, (float)T.y, (float)T.z };
            curVector = Vector3Add(curVector, v);
        }
        boneWorldPos[bIdx] = curVector;
    }

    vector<Vector4> boneRot(model.bones.size());
    for (int bIdx = 0; bIdx < model.bones.size(); bIdx++) {
        auto& boneN = m.nodes[bIdx];
        if (boneN.children.size() != 1) {
            boneRot[bIdx] = QuaternionIdentity();
            continue;
        }
        auto chldIdx = boneN.children[0];
        auto vec = Vector3Subtract(boneWorldPos[chldIdx], boneWorldPos[bIdx]);
        auto& q = QuaternionFromVector3ToVector3(Vector3UnitY,vec);
        boneRot[bIdx] = q;
        boneN.rotation = { q.x, q.y, q.z, q.w };
        auto dist = Vector3Length(vec);
        m.nodes[chldIdx].translation = { 0, dist, 0 };
    }

    //skin.inverseBindMatrices = addBoneMatrices(m, bonePos, boneRot, model.bones.size());
    skin.skeleton = 0;
    m.skins.push_back(skin);

    /*for (int i = 0; i < animations.size(); i++) {
        addAnimation(m, *animations[i]);
    }*/

    std::filesystem::create_directories(dirname);
    tinygltf::TinyGLTF gltf;
    gltf.WriteGltfSceneToFile(&m, dirname + "/skeleton.gltf",
       false, // embedImages
       false, // embedBuffers
       false, // pretty print
       false
    );
}