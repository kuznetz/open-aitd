#pragma once
#include "model_extractor.h"
#include "../structs/model.h"
#include "../structs/animation.h"
#include "../../../common/name_decoders.hpp"

#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <filesystem>
#include <algorithm>

namespace AITDExtractor {

    using namespace std;
    using nlohmann::json;

    // One concrete body variant of a character.
    struct CharacterModelDef {
        string name;                 // decoded model name
        int    modelId = -1;         // index inside LISTBODY.PAK
        bool   hasAlt = false;       // LISTBOD2 differs from LISTBODY
    };

    // Character definition parsed from newdata/characters.json.
    struct CharacterDef {
        string         name;         // e.g. "PLAYER"
        string         baseModel;    // e.g. "PLAYER_NORMAL" (defines the rig)
        vector<string> models;       // variants
        vector<int>    animations;   // animation ids (LISTANIM / LISTANI2)
        int            attachBone = -1; // bone (ModelBone::boneIdx) holding the in-hand item
    };

    // Exports the base character body (its animations) into
    // data/char_models/{char_name}/BASE.gltf and the in-hand item of the
    // base body plus every variant into its own attach file.
    //
    // No skeleton is shared between the baseModel and the variants: the body
    // skeleton of BASE.gltf is built from the baseModel, while the skeleton
    // of every attach file is built from the model of the variant that owns it.
    class CharactersExtractor {
    public:
        CharactersExtractor(ResourceLoader& resLoader,
                            const Pallete& pallete,
                            const openAITD::NameDecoders& names,
                            bool exportAlt = false)
            : resLoader(resLoader), pallete(pallete), names(names), exportAlt(exportAlt) {}

        void extractAll(const string& jsonPath = "newdata/characters.json",
                        const string& outRoot = "data/char_models")
        {
            this->outRoot = outRoot;
            loadResources();

            std::ifstream ifs(jsonPath);
            if (!ifs.is_open()) return;

            json arr = json::parse(ifs);
            for (auto& defJson : arr) {
                CharacterDef def = resolveDefinition(defJson);
                extractCharacter(def, false);
                if (exportAlt) {
                    extractCharacter(def, true);
                }
            }
        }

    private:
        ResourceLoader&            resLoader;
        const Pallete&             pallete;
        const openAITD::NameDecoders& names;
        bool                       exportAlt;
        string              outRoot;

        // --- cached resources ---
        bool              resourcesLoaded = false;
        PakFile           bodyPak;
        PakFile           body2Pak;
        vector<PakModel>  bodyModels;
        vector<PakModel>  body2Models;
        vector<bool>      bodyHasAlt;
        vector<Animation> anims;   // LISTANIM
        vector<Animation> anims2;  // LISTANI2

        // ------------------------------------------------------------------
        // Resource loading / json parsing
        // ------------------------------------------------------------------

        void loadResources()
        {
            if (resourcesLoaded) return;
            resourcesLoaded = true;

            bodyPak.open("original/LISTBODY.PAK");
            body2Pak.open("original/LISTBOD2.PAK");

            const int count = (int)bodyPak.headers.size();
            bodyModels.resize(count);
            body2Models.resize(count);
            bodyHasAlt.assign(count, false);

            for (int i = 0; i < count; i++) {
                auto body1 = bodyPak.readBlock(i);
                auto body2 = body2Pak.readBlock(i);

                bool alt = body2Pak.headers[i].uncompressedSize != bodyPak.headers[i].uncompressedSize;
                if (!alt) {
                    alt = !!memcmp(body1.data(), body2.data(), body1.size());
                }
                bodyHasAlt[i] = alt;

                bodyModels[i] = resLoader.loadModel(body1);
                if (alt) {
                    body2Models[i] = resLoader.loadModel(body2);
                }
            }

            PakFile animPak("original/LISTANIM.PAK");
            PakFile anim2Pak("original/LISTANI2.PAK");

            anims.resize(animPak.headers.size());
            for (int i = 0; i < (int)animPak.headers.size(); i++) {
                auto block = animPak.readBlock(i);
                anims[i] = resLoader.loadAnimation(block);
                anims[i].id = i;
            }

            anims2.resize(anim2Pak.headers.size());
            for (int i = 0; i < (int)anim2Pak.headers.size(); i++) {
                auto block = anim2Pak.readBlock(i);
                anims2[i] = resLoader.loadAnimation(block);
                anims2[i].id = i;
            }
        }

        int findModelId(const string& name) const
        {
            for (int i = 0; i < (int)bodyModels.size(); i++) {
                if (names.model.getName(i) == name) {
                    return i;
                }
            }
            return -1;
        }

        CharacterDef resolveDefinition(const json& defJson) const
        {
            CharacterDef def;
            def.name = defJson.value("name", string());
            def.baseModel = defJson.value("baseModel", string());
            if (defJson.contains("models")) {
                def.models = defJson["models"].get<vector<string>>();
            }
            if (defJson.contains("animations")) {
                def.animations = defJson["animations"].get<vector<int>>();
            }
            def.attachBone = defJson.value("attachBone", -1);
            return def;
        }

        // ------------------------------------------------------------------
        // Skeleton building (one skeleton per model, nothing is shared)
        // ------------------------------------------------------------------

        // Computes local bind positions and accumulated world positions of every
        // bone of a single model (both indexed by the bone array position).
        void computeBonePositions(const PakModel& model,
                                  vector<Vector3>& outBonePos,
                                  vector<Vector3>& outBoneWorldPos) const
        {
            const int n = (int)model.bones.size();
            outBonePos.resize(n);

            for (int bIdx = 0; bIdx < n; bIdx++) {
                auto& bone = model.bones[bIdx];
                int rIdx = bone.rootVertexIdx / 6;
                outBonePos[bIdx] = Vector3Transform({
                    model.vertices[rIdx * 3 + 0] / 1000.f,
                    model.vertices[rIdx * 3 + 1] / 1000.f,
                    model.vertices[rIdx * 3 + 2] / 1000.f
                }, modelMatrix);
            }

            // parent by array position (parentBoneIdx refers to boneIdx)
            vector<int> parentPos(n, -1);
            for (int b = 0; b < n; b++) {
                int pBoneIdx = model.bones[b].parentBoneIdx;
                for (int j = 0; j < n; j++) {
                    if (j == b) continue;
                    if (model.bones[j].boneIdx == pBoneIdx) {
                        parentPos[b] = j;
                        break;
                    }
                }
            }

            outBoneWorldPos.resize(n);
            for (int b = 0; b < n; b++) {
                Vector3 acc = { 0, 0, 0 };
                int c = b;
                while (c != -1) {
                    acc = Vector3Add(acc, outBonePos[c]);
                    c = parentPos[c];
                }
                outBoneWorldPos[b] = acc;
            }
        }

        // Builds one skeleton (bone nodes + one skin) from a single model. Bone
        // node indices equal the bone array positions, so the baked animations
        // (indexed the same way) always match this specific model.
        void buildSkeleton(tinygltf::Model& m, const PakModel& model,
                           vector<Vector3>& outBonePos,
                           vector<Vector3>& outBoneWorldPos)
        {
            computeBonePositions(model, outBonePos, outBoneWorldPos);

            const int n = (int)model.bones.size();

            tinygltf::Skin skin;
            for (int bIdx = 0; bIdx < n; bIdx++) {
                skin.joints.push_back(bIdx);

                tinygltf::Node boneN;
                boneN.name = string("bone_") + to_string(bIdx);
                auto& p = outBonePos[bIdx];
                boneN.translation = { p.x, p.y, p.z };

                for (int j = 0; j < n; j++) {
                    if (j == bIdx) continue;
                    if (model.bones[j].parentBoneIdx == model.bones[bIdx].boneIdx) {
                        boneN.children.push_back(j);
                    }
                }

                m.nodes.push_back(boneN);
            }

            skin.inverseBindMatrices = addBoneMatrices(m, outBoneWorldPos, n);
            skin.skeleton = 0;
            m.skins.push_back(skin);
        }

        // ------------------------------------------------------------------
        // In-hand item separation (attach bone subtree)
        // ------------------------------------------------------------------

        // Position of the bone whose semantic id (ModelBone::boneIdx) equals
        // `boneId` inside `model.bones`. Falls back to treating `boneId` as a
        // plain array position when no semantic match exists.
        int findBonePos(const PakModel& model, int boneId) const
        {
            if (boneId < 0) return -1;
            for (int b = 0; b < (int)model.bones.size(); b++) {
                if (model.bones[b].boneIdx == boneId) return b;
            }
            if (boneId < (int)model.bones.size()) return boneId;
            return -1;
        }

        // Marks the attach bone plus every bone that (transitively) depends on
        // it. `parentBoneIdx` refers to the semantic id (ModelBone::boneIdx).
        vector<bool> collectAttachBoneSet(const PakModel& model, int attachBone) const
        {
            vector<bool> inSet(model.bones.size(), false);
            const int root = findBonePos(model, attachBone);
            if (root < 0) return inSet;

            inSet[root] = true;

            bool changed = true;
            while (changed) {
                changed = false;
                for (int b = 0; b < (int)model.bones.size(); b++) {
                    if (inSet[b]) continue;
                    int parentIdx = model.bones[b].parentBoneIdx;
                    for (int j = 0; j < (int)model.bones.size(); j++) {
                        if (inSet[j] && model.bones[j].boneIdx == parentIdx) {
                            inSet[b] = true;
                            changed = true;
                            break;
                        }
                    }
                }
            }
            return inSet;
        }

        // Per-primitive flag: true when the primitive is bound to the attach
        // bone subtree (the in-hand item), false for the rest of the body.
        vector<bool> classifyAttachPrimitives(const PakModel& model,
                                              const vector<bool>& attachBones) const
        {
            vector<bool> isAttach(model.primitives.size(), false);

            // vertex -> bone array position (same rule as addModelMesh)
            vector<int> vertBone(model.vertices.size() / 3, 0);
            for (int b = 0; b < (int)model.bones.size(); b++) {
                int vfrom = model.bones[b].fromVertexIdx / 6;
                for (int j = vfrom; j < vfrom + model.bones[b].vertexCount; j++) {
                    if (j >= 0 && j < (int)vertBone.size()) {
                        vertBone[j] = b;
                    }
                }
            }

            for (int p = 0; p < (int)model.primitives.size(); p++) {
                auto& prim = model.primitives[p];
                if (prim.vertexIdxs.empty()) continue;
                const int vIdx = prim.vertexIdxs[0] / 6;
                if (vIdx < 0 || vIdx >= (int)vertBone.size()) continue;
                const int bonePos = vertBone[vIdx];
                if (bonePos >= 0 && bonePos < (int)attachBones.size()) {
                    isAttach[p] = attachBones[bonePos];
                }
            }
            return isAttach;
        }

        // ------------------------------------------------------------------
        // Dedicated attach skeletons (in-hand items)
        // ------------------------------------------------------------------

        // One dedicated skeleton for an attach (in-hand item) mesh. The skeleton
        // is built from the bones of the variant model that owns the attach.
        struct AttachSkeleton {
            vector<int> boneArrayPos;   // subtree bone positions inside model.bones
            vector<int> nodeIdx;        // glTF node index per subtree bone (0 = root)
            int skinIdx = -1;           // glTF skin index (-1 => empty subtree)
            int rootBonePos = -1;       // model bone array position of the subtree root
        };

        // Builds a dedicated skeleton + skin for the attach subtree of `model`
        // (subtree bones only, without their ancestors). `boneWorldPos` are the
        // attach model's own bone world positions. Returns an invalid skeleton
        // (skinIdx < 0) only when the subtree is empty.
        AttachSkeleton buildAttachSkeleton(tinygltf::Model& m, const PakModel& model,
                                           const vector<bool>& inSet,
                                           const vector<Vector3>& boneWorldPos)
        {
            AttachSkeleton sk;

            vector<int> positions;
            for (int b = 0; b < (int)model.bones.size(); b++) {
                if (inSet[b]) positions.push_back(b);
            }
            if (positions.empty()) return sk;

            // Root = the subtree bone whose parent is not part of the subtree.
            int rootPos = positions[0];
            for (int i = 0; i < (int)positions.size(); i++) {
                int pBoneIdx = model.bones[positions[i]].parentBoneIdx;
                bool parentInSet = false;
                for (int j = 0; j < (int)positions.size(); j++) {
                    if (model.bones[positions[j]].boneIdx == pBoneIdx) { parentInSet = true; break; }
                }
                if (!parentInSet) { rootPos = positions[i]; break; }
            }
            auto it = std::find(positions.begin(), positions.end(), rootPos);
            if (it != positions.end() && it != positions.begin()) {
                int tmp = positions[0];
                positions[0] = *it;
                *it = tmp;
            }

            sk.boneArrayPos = positions;
            sk.nodeIdx.resize(positions.size());
            sk.rootBonePos = positions[0];

            // Nodes: the root sits at the attach origin, the rest are relative to
            // their (in-subtree) parent so the accumulated world positions match
            // this model's own bind pose.
            for (int i = 0; i < (int)positions.size(); i++) {
                tinygltf::Node n;
                n.name = string("bone_") + to_string(positions[i]);

                const Vector3 world = boneWorldPos[positions[i]];
                if (i == 0) {
                    // Attach origin: the engine anchors the item to a body bone.
                    n.translation = { 0, 0, 0 };
                } else {
                    int parentPos = -1;
                    int pBoneIdx = model.bones[positions[i]].parentBoneIdx;
                    for (int j = 0; j < (int)positions.size(); j++) {
                        if (i == j) continue;
                        if (model.bones[positions[j]].boneIdx == pBoneIdx) { parentPos = j; break; }
                    }
                    if (parentPos < 0) {
                        n.translation = { world.x, world.y, world.z };
                    } else {
                        Vector3 d = Vector3Subtract(world, boneWorldPos[positions[parentPos]]);
                        n.translation = { d.x, d.y, d.z };
                    }
                }

                sk.nodeIdx[i] = (int)m.nodes.size();
                m.nodes.push_back(n);
            }

            // Parent -> children links within the subtree.
            for (int i = 1; i < (int)positions.size(); i++) {
                int pBoneIdx = model.bones[positions[i]].parentBoneIdx;
                for (int j = 0; j < (int)positions.size(); j++) {
                    if (i == j) continue;
                    if (model.bones[positions[j]].boneIdx == pBoneIdx) {
                        m.nodes[sk.nodeIdx[j]].children.push_back(sk.nodeIdx[i]);
                        break;
                    }
                }
            }

            // Bind pose relative to the attach origin (root bone at 0,0,0).
            const Vector3 rootWorld = boneWorldPos[sk.rootBonePos];
            vector<Vector3> jointWorld(positions.size());
            for (int i = 0; i < (int)positions.size(); i++) {
                jointWorld[i] = Vector3Subtract(boneWorldPos[positions[i]], rootWorld);
            }

            tinygltf::Skin skin;
            for (int i = 0; i < (int)positions.size(); i++) {
                skin.joints.push_back(sk.nodeIdx[i]);
            }
            skin.inverseBindMatrices = addBoneMatrices(m, jointWorld, (int)positions.size());
            skin.skeleton = sk.nodeIdx[0];
            sk.skinIdx = (int)m.skins.size();
            m.skins.push_back(skin);

            return sk;
        }

        // ------------------------------------------------------------------
        // Mesh building (one variant -> one skinned mesh node)
        // ------------------------------------------------------------------

        void addModelMesh(tinygltf::Model& m, const PakModel& model, const string& meshName,
                          const vector<Vector3>& boneWorldPos,
                          const Pallete& pallete, const string& dirname,
                          int noiseTexIdx, bool hasNoise,
                          const vector<bool>* primFilter = nullptr,
                          int skinIdx = 0,
                          const vector<int>* jointRemap = nullptr,
                          Vector3 vertexOffset = { 0,0,0 },
                          bool skinned = true)
        {
            // Optional per-primitive mask used to split a variant into the body
            // mesh and its in-hand item mesh (nullptr keeps every primitive).
            auto keepPrim = [&](int pIdx) {
                return !primFilter ||
                       (pIdx < (int)primFilter->size() && (*primFilter)[pIdx]);
            };

            vector<Vector3> modelVerts(model.vertices.size() / 3);
            for (int i = 0; i < (int)modelVerts.size(); i++) {
                modelVerts[i] = Vector3Transform({
                    model.vertices[i * 3 + 0] / 1000.f,
                    model.vertices[i * 3 + 1] / 1000.f,
                    model.vertices[i * 3 + 2] / 1000.f
                }, modelMatrix);
            }

            const bool hasModelBones = !model.bones.empty();
            vector<u8> vecBoneAffect(modelVerts.size(), 0);

            if (hasModelBones) {
                for (int bIdx = 0; bIdx < (int)model.bones.size(); bIdx++) {
                    int vfrom = model.bones[bIdx].fromVertexIdx / 6;
                    for (int j = vfrom; j < vfrom + model.bones[bIdx].vertexCount; j++) {
                        if (j >= 0 && j < (int)vecBoneAffect.size()) {
                            vecBoneAffect[j] = (u8)bIdx;
                        }
                    }
                }

                // Place the vertices in this model's own skeleton bind pose.
                // `vertexOffset` moves an attach (in-hand item) mesh into its own
                // local space (the engine re-attaches it at runtime); `jointRemap`
                // (optional) then translates the model bone position into a
                // dedicated attach skin joint index.
                for (int i = 0; i < (int)modelVerts.size(); i++) {
                    int sb = vecBoneAffect[i];
                    if (sb < 0 || sb >= (int)boneWorldPos.size()) sb = 0;
                    modelVerts[i] = Vector3Add(modelVerts[i],
                                               Vector3Subtract(boneWorldPos[sb], vertexOffset));
                    int joint = jointRemap ? (*jointRemap)[sb] : sb;
                    vecBoneAffect[i] = (u8)joint;
                }
            }

            // Only true for meshes that must carry skinning data (the body and
            // multi-bone attaches); single-bone attaches stay engine-anchored.
            const bool skinnedMesh = hasModelBones && skinned;

            int vertAccIdx = createVertexes(m, modelVerts);

            float lineSize = computeLineSize(modelVerts);
            float noisesize = hasNoise ? computeNoiseSize(modelVerts) : 0.0f;

            VertexSkin vSkin = { 0, 0 };
            if (skinnedMesh) {
                vSkin = addVertexSkin(m, vecBoneAffect);
            }

            tinygltf::Mesh mesh;

            // --- polygons ---
            for (int pIdx = 0; pIdx < (int)model.primitives.size(); pIdx++) {
                auto& prim = model.primitives[pIdx];
                if (prim.type != 1) continue;
                if (!keepPrim(pIdx)) continue;
                auto prim2 = createPrimitivePoly(m, prim, modelVerts, vertAccIdx, pallete,
                                                 vecBoneAffect, vSkin, skinnedMesh, noiseTexIdx, noisesize);
                mesh.primitives.push_back(prim2);
            }

            // --- spheres ---
            for (int pIdx = 0; pIdx < (int)model.primitives.size(); pIdx++) {
                auto& prim = model.primitives[pIdx];
                if (prim.type != 3) continue;
                if (!keepPrim(pIdx)) continue;

                const int vertCount = 5;
                auto matIdx = getMaterialIdx(m, prim.colorIndex, pallete, prim.subType, noiseTexIdx);
                auto& pos = modelVerts[prim.vertexIdxs[0] / 6];
                float size = prim.size / 1000.0f;
                auto prim2 = createSpherePrim(m, size, vertCount, pos, matIdx);

                if (skinnedMesh) {
                    int VertCountFull = vertCount * vertCount * 6;
                    vector<u8> vecBoneAffect2(VertCountFull, vecBoneAffect[prim.vertexIdxs[0] / 6]);
                    auto vSkin2 = addVertexSkin(m, vecBoneAffect2);
                    prim2.attributes["JOINTS_0"] = vSkin2.jointsAccIdx;
                    prim2.attributes["WEIGHTS_0"] = vSkin2.weightsAccIdx;
                }
                mesh.primitives.push_back(prim2);
            }

            // --- lines ---
            for (int pIdx = 0; pIdx < (int)model.primitives.size(); pIdx++) {
                auto& prim = model.primitives[pIdx];
                if (prim.type != 0) continue;
                if (!keepPrim(pIdx)) continue;
                if (prim.vertexIdxs.size() != 2) {
                    throw new exception("Line indexes not 2");
                }

                auto matIdx = getMaterialIdx(m, prim.colorIndex, pallete, 0, noiseTexIdx);
                Vector3 points[2] = {
                    modelVerts[prim.vertexIdxs[0] / 6],
                    modelVerts[prim.vertexIdxs[1] / 6]
                };
                auto prim2 = createPipePrim(m, points, lineSize, 4, matIdx);

                if (skinnedMesh) {
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

            // --- pixel / squares ---
            for (int pIdx = 0; pIdx < (int)model.primitives.size(); pIdx++) {
                auto& prim = model.primitives[pIdx];
                if (!keepPrim(pIdx)) continue;

                float size;
                if (prim.type == 2) {
                    size = lineSize;
                } else if (prim.type == 6) {
                    size = lineSize * 2.5f;
                } else if (prim.type == 7) {
                    size = lineSize * 5.0f;
                } else {
                    continue;
                }

                auto matIdx = getMaterialIdx(m, prim.colorIndex, pallete, 0, noiseTexIdx);
                auto prim2 = createCubePrim(m, modelVerts[prim.vertexIdxs[0] / 6], { size, size, size }, matIdx);

                if (skinnedMesh) {
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
            int meshIdx = m.meshes.size() - 1;

            tinygltf::Node meshNode;
            meshNode.name = meshName;
            meshNode.mesh = meshIdx;
            if (skinnedMesh) {
                meshNode.skin = skinIdx;
            }
            m.nodes.push_back(meshNode);
        }

        // ------------------------------------------------------------------
        // Animations (baked once for the whole character)
        // ------------------------------------------------------------------

        // Adds animation channels for the descendant bones of one attach
        // skeleton. The attach root stays at the origin (0,0,0) because the
        // engine anchors the item to a body bone itself; descendant bones keep
        // the same local transforms as the model skeleton.
        //
        // The attachBone itself is intentionally NOT animated here: it is
        // animated by the base model (BASE.gltf) as part of the body
        // skeleton, and the engine re-attaches the item to it at runtime.
        void addAttachAnimation(tinygltf::Model& m, tinygltf::Animation& outAni,
                                Animation& anim, const vector<Vector3>& bonePos,
                                int maxBones, const AttachSkeleton& sk, int accTimeIdx)
        {
            const int frameCount = (int)anim.frames.size();
            const int keys = frameCount + 1;

            for (int i = 0; i < (int)sk.boneArrayPos.size(); i++) {
                int sb = sk.boneArrayPos[i];
                if (sb == sk.rootBonePos) continue; // attachBone animation stays in the base model
                if (sb < 0 || sb >= maxBones) continue;

                vector<Vector4> rot(keys);
                vector<Vector3> tr(keys);
                vector<Vector3> sc(keys);
                for (int fi = 1; fi <= frameCount; fi++) {
                    auto& b = anim.frames[fi - 1].bones[sb];
                    rot[fi] = QuaternionIdentity();
                    tr[fi] = bonePos[sb];
                    sc[fi] = { 1,1,1 };
                    switch (b.type) {
                    case 0: rot[fi] = GetAniRotation(b); break;
                    case 1: tr[fi] = Vector3Add(bonePos[sb], Vector3Transform({
                                b.delta[0] / 1000.f, b.delta[1] / 1000.f, b.delta[2] / 1000.f },
                                modelMatrix)); break;
                    case 2: sc[fi] = { b.delta[0] / 256.f + 1.f,
                                       b.delta[1] / 256.f + 1.f,
                                       b.delta[2] / 256.f + 1.f }; break;
                    }
                }
                rot[0] = rot[1]; tr[0] = tr[1]; sc[0] = sc[1];

                addAniRotation(m, outAni, rot, accTimeIdx, sk.nodeIdx[i]);
                addAniTranslation(m, outAni, tr, accTimeIdx, sk.nodeIdx[i]);
                addAniScale(m, outAni, sc, accTimeIdx, sk.nodeIdx[i]);
            }
        }

        void addCharacterAnimations(tinygltf::Model& m, const vector<Animation*>& list,
                                    vector<Vector3>& bonePos, int boneCount)
        {
            for (size_t i = 0; i < list.size(); i++) {
                Animation* a = list[i];
                if (!a || a->frames.empty()) continue;
                addAnimation(m, *a, bonePos, boneCount);
            }
        }

        // Creates the animation key-time accessor (same timeline as addAnimation).
        int addTimeAccessor(tinygltf::Model& m, const Animation& anim)
        {
            vector<float> timeline;
            timeline.push_back(0);
            float curTime = 0;
            for (int i = 0; i < (int)anim.frames.size(); i++) {
                curTime += anim.frames[i].timestamp / 60.f;
                timeline.push_back(curTime);
            }

            int vwTime = createBufferAndView(m, timeline.data(), timeline.size() * sizeof(float), 0);
            tinygltf::Accessor accTime;
            accTime.bufferView = vwTime;
            accTime.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
            accTime.count = timeline.size();
            accTime.type = TINYGLTF_TYPE_SCALAR;
            m.accessors.push_back(accTime);
            return (int)m.accessors.size() - 1;
        }

        // True when the animation actually changes at least one descendant bone
        // of the attach skeleton (i.e. it is not empty for this attach). The
        // attachBone itself (subtree root) is ignored - its animation lives in
        // the base model, never in an attach file.
        bool animationAffectsAttach(const Animation& anim, const AttachSkeleton& sk,
                                    int maxBones) const
        {
            for (int i = 0; i < (int)sk.boneArrayPos.size(); i++) {
                int sb = sk.boneArrayPos[i];
                if (sb == sk.rootBonePos) continue;
                if (sb < 0 || sb >= maxBones) continue;
                for (int f = 0; f < (int)anim.frames.size(); f++) {
                    const AniBone& b = anim.frames[f].bones[sb];
                    if (b.delta[0] || b.delta[1] || b.delta[2]) return true;
                }
            }
            return false;
        }

        // Bakes only the non-empty character animations into a standalone attach
        // model (each attach file owns its animation objects).
        void addAttachAnimations(tinygltf::Model& mv, const vector<Animation*>& list,
                                 const vector<Vector3>& bonePos, int boneCount,
                                 const AttachSkeleton& sk)
        {
            if (sk.skinIdx < 0) return;

            for (size_t i = 0; i < list.size(); i++) {
                Animation* a = list[i];
                if (!a || a->frames.empty()) continue;

                const int maxBones = (int)std::min((size_t)boneCount, a->frames[0].bones.size());
                if (maxBones <= 0) continue;

                if (!animationAffectsAttach(*a, sk, maxBones)) continue;

                const int accTimeIdx = addTimeAccessor(mv, *a);

                tinygltf::Animation outAni;
                outAni.name = string("a_") + to_string(a->id);
                addAttachAnimation(mv, outAni, *a, bonePos, maxBones, sk, accTimeIdx);

                if (outAni.channels.empty()) continue;
                mv.animations.push_back(outAni);
            }
        }

        // ------------------------------------------------------------------
        // Character extraction
        // ------------------------------------------------------------------

        void extractCharacter(const CharacterDef& charDef, bool alt)
        {
            // resolve model ids (base first, then variants, unique)
            vector<int> ids;
            vector<string> idNames;
            auto addId = [&](const string& nm) {
                if (nm.empty()) return;
                int id = findModelId(nm);
                if (id < 0) return;
                if (std::find(ids.begin(), ids.end(), id) != ids.end()) return;
                ids.push_back(id);
                idNames.push_back(nm);
            };
            addId(charDef.baseModel);
            for (auto& nm : charDef.models) addId(nm);

            // gather bodies for the requested mode
            vector<PakModel*> models;
            vector<string>    meshNames;
            for (int k = 0; k < (int)ids.size(); k++) {
                int id = ids[k];
                if (alt) {
                    if (!bodyHasAlt[id]) continue;
                    models.push_back(&body2Models[id]);
                } else {
                    models.push_back(&bodyModels[id]);
                }
                meshNames.push_back(idNames[k]);
            }
            if (models.empty()) return;

            const string dirname = outRoot + "/" + charDef.name + (alt ? "_alt" : "");
            if (std::filesystem::exists(dirname)) return;

            const int baseId = findModelId(charDef.baseModel);

            // The base body is the source of the BASE.gltf skeleton. It is
            // part of the current export mode only when its body is present.
            const PakModel* baseModel = nullptr;
            if (baseId >= 0) {
                if (alt) {
                    if (bodyHasAlt[baseId]) baseModel = &body2Models[baseId];
                } else {
                    baseModel = &bodyModels[baseId];
                }
            }
            const PakModel& skelModel = baseModel ? *baseModel : *models[0];
            if (skelModel.bones.empty()) return;

            auto& animSrc = alt ? anims2 : anims;
            vector<Animation*> charAnims;
            vector<int>        usedAnimIds;
            for (int aid : charDef.animations) {
                if (aid < 0 || aid >= (int)animSrc.size()) continue;
                charAnims.push_back(&animSrc[aid]);
                usedAnimIds.push_back(aid);
            }

            tinygltf::Model m;
            m.asset.version = "2.0";
            m.asset.generator = "open-AITD";

            // BASE.gltf skeleton = the base model's own skeleton.
            vector<Vector3> bonePos;
            vector<Vector3> boneWorldPos;
            buildSkeleton(m, skelModel, bonePos, boneWorldPos);
            const int boneCount = (int)bonePos.size();

            // noise texture (shared by all variants)
            bool anyNoise = false;
            for (auto* pm : models) {
                for (auto& prim : pm->primitives) {
                    if (prim.type == 1 && prim.subType == 1) { anyNoise = true; break; }
                }
                if (anyNoise) break;
            }

            int noiseTexIdx = -1;
            if (anyNoise) {
                std::filesystem::create_directories(dirname);
                auto noiseRgba = generateNoiseRgba(NOISE_TEXTURE_SIZE);
                savePng((dirname + "/" + NOISE_TEXTURE_FILE).c_str(),
                        NOISE_TEXTURE_SIZE, NOISE_TEXTURE_SIZE,
                        noiseRgba.data(), PNG_COLOR_TYPE_RGBA);
                noiseTexIdx = addImageTexture(m, NOISE_TEXTURE_FILE);
            }

            // --- base character (BASE.gltf) ---
            // The base model's skeleton plus its body mesh "BASE". The in-hand
            // item of the base body goes to its own file, like every variant.
            const string BASE_MESH_NAME = "BASE";

            vector<string> baseMeshNames;

            if (baseModel) {
                auto& model = *baseModel;

                bool hasNoise = false;
                for (auto& prim : model.primitives) {
                    if (prim.type == 1 && prim.subType == 1) { hasNoise = true; break; }
                }

                // Peel the attachBone subtree off the rest of the body.
                bool split = false;
                vector<bool> isBody;
                if (charDef.attachBone >= 0) {
                    auto attachBones = collectAttachBoneSet(model, charDef.attachBone);
                    auto isAttach = classifyAttachPrimitives(model, attachBones);

                    bool anyAttach = false;
                    for (bool v : isAttach) { if (v) { anyAttach = true; break; } }

                    if (anyAttach) {
                        isBody.assign(isAttach.size(), false);
                        for (size_t p = 0; p < isAttach.size(); p++) {
                            isBody[p] = !isAttach[p];
                        }
                        split = true;
                    }
                }

                if (split) {
                    addModelMesh(m, model, BASE_MESH_NAME, boneWorldPos,
                                 pallete, dirname, noiseTexIdx, hasNoise, &isBody);
                } else {
                    addModelMesh(m, model, BASE_MESH_NAME, boneWorldPos,
                                 pallete, dirname, noiseTexIdx, hasNoise);
                }
                baseMeshNames.push_back(BASE_MESH_NAME);
            }

            addCharacterAnimations(m, charAnims, bonePos, boneCount);

            std::filesystem::create_directories(dirname);
            writeGltf(m, dirname + "/BASE.gltf");

            // --- attach files (base item + every variant item) ---
            // Each attach is written to its own file in its own local space (the
            // attach bone at the origin); the engine attaches it to the body. The
            // skeleton of an attach file is built from the model of the variant
            // it belongs to (never from the base model or a shared rig).
            json attachmentsJson = json::array();

            for (int k = 0; charDef.attachBone >= 0 && k < (int)models.size(); k++) {
                auto& model = *models[k];
                const bool isBase = (ids[k] == baseId);

                bool hasNoise = false;
                for (auto& prim : model.primitives) {
                    if (prim.type == 1 && prim.subType == 1) { hasNoise = true; break; }
                }

                auto attachBones = collectAttachBoneSet(model, charDef.attachBone);
                auto isAttach = classifyAttachPrimitives(model, attachBones);

                bool anyAttach = false;
                for (bool v : isAttach) { if (v) { anyAttach = true; break; } }
                if (!anyAttach) continue;

                // The attach skeleton comes from this variant's own model.
                vector<Vector3> varBonePos;
                vector<Vector3> varBoneWorldPos;
                computeBonePositions(model, varBonePos, varBoneWorldPos);

                tinygltf::Model mv;
                mv.asset.version = "2.0";
                mv.asset.generator = "open-AITD";

                int vNoiseTexIdx = -1;
                if (hasNoise) {
                    std::filesystem::create_directories(dirname);
                    vNoiseTexIdx = addImageTexture(mv, NOISE_TEXTURE_FILE);
                }

                // Every attach gets its own skeleton, even a single-bone one.
                Vector3 attOffset = { 0,0,0 };
                int rootPos = findBonePos(model, charDef.attachBone);
                if (rootPos >= 0 && rootPos < (int)varBoneWorldPos.size()) {
                    attOffset = varBoneWorldPos[rootPos];
                }

                AttachSkeleton sk = buildAttachSkeleton(mv, model, attachBones,
                                                        varBoneWorldPos);
                vector<int> attRemap(model.bones.size(), 0);
                for (int i = 0; i < (int)sk.boneArrayPos.size(); i++) {
                    attRemap[sk.boneArrayPos[i]] = i;
                }

                addModelMesh(mv, model, meshNames[k], varBoneWorldPos,
                             pallete, dirname, vNoiseTexIdx, hasNoise, &isAttach,
                             sk.skinIdx, &attRemap, attOffset, true);

                addAttachAnimations(mv, charAnims, varBonePos, (int)model.bones.size(), sk);

                std::filesystem::create_directories(dirname);
                const string attachFile = meshNames[k] + ".gltf";
                writeGltf(mv, dirname + "/" + attachFile);

                json a;
                a["name"] = meshNames[k];
                a["file"] = attachFile;
                a["base"] = isBase;
                attachmentsJson.push_back(a);
            }

            // --- data.json ---
            std::filesystem::create_directories(dirname);
            json dataJson;
            dataJson["name"] = charDef.name;
            dataJson["bones"] = boneCount;
            dataJson["baseModel"] = charDef.baseModel;
            dataJson["base"] = BASE_MESH_NAME;
            dataJson["baseFile"] = "BASE.gltf";
            dataJson["meshes"] = baseMeshNames;
            dataJson["animations"] = usedAnimIds;
            dataJson["attachBone"] = charDef.attachBone;
            dataJson["attachments"] = attachmentsJson;

            std::ofstream o(dirname + "/data.json");
            o << std::setw(2) << dataJson;
        }

        void writeGltf(tinygltf::Model& m, const string& filePath)
        {
            tinygltf::TinyGLTF gltf;
            gltf.WriteGltfSceneToFile(&m, filePath,
                false, // embedImages
                false, // embedBuffers
                false, // pretty print
                false  // binary (glb)
            );
        }
    };

}
