// Old deprecated code
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
    };

    // Exports a whole character (all body variants + all of its animations)
    // into a single glTF file: data/char_models/{char_name}/character.gltf.
    //
    // Different body variants may have a different number of bones; they are all
    // re-mapped onto one shared skeleton (the variant with the most bones).
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
            return def;
        }

        // ------------------------------------------------------------------
        // Skeleton unification (different bone counts)
        // ------------------------------------------------------------------

        // Maps position of a bone inside `model.bones` onto the rig bone position
        // by matching the semantic bone id (ModelBone::boneIdx).
        vector<int> buildBoneMap(const PakModel& model, const PakModel& rig) const
        {
            vector<int> map(model.bones.size(), 0);
            for (int p = 0; p < (int)model.bones.size(); p++) {
                int mapped = -1;
                for (int q = 0; q < (int)rig.bones.size(); q++) {
                    if (rig.bones[q].boneIdx == model.bones[p].boneIdx) {
                        mapped = q;
                        break;
                    }
                }
                if (mapped < 0) {
                    mapped = (p < (int)rig.bones.size()) ? p : 0;
                }
                map[p] = mapped;
            }
            return map;
        }

        // Builds the shared skeleton (nodes + one skin) from the rig source.
        void buildSharedSkeleton(tinygltf::Model& m, const PakModel& rig,
                                 vector<Vector3>& outBonePos,
                                 vector<Vector3>& outBoneWorldPos)
        {
            const int n = (int)rig.bones.size();
            outBonePos.resize(n);

            for (int bIdx = 0; bIdx < n; bIdx++) {
                auto& bone = rig.bones[bIdx];
                int rIdx = bone.rootVertexIdx / 6;
                outBonePos[bIdx] = Vector3Transform({
                    rig.vertices[rIdx * 3 + 0] / 1000.f,
                    rig.vertices[rIdx * 3 + 1] / 1000.f,
                    rig.vertices[rIdx * 3 + 2] / 1000.f
                }, modelMatrix);
            }

            // parent by array position (parentBoneIdx refers to boneIdx)
            vector<int> parentPos(n, -1);
            for (int b = 0; b < n; b++) {
                int pBoneIdx = rig.bones[b].parentBoneIdx;
                for (int j = 0; j < n; j++) {
                    if (j == b) continue;
                    if (rig.bones[j].boneIdx == pBoneIdx) {
                        parentPos[b] = j;
                        break;
                    }
                }
            }

            tinygltf::Skin skin;
            for (int bIdx = 0; bIdx < n; bIdx++) {
                skin.joints.push_back(bIdx);

                tinygltf::Node boneN;
                boneN.name = string("bone_") + to_string(bIdx);
                auto& p = outBonePos[bIdx];
                boneN.translation = { p.x, p.y, p.z };

                for (int j = 0; j < n; j++) {
                    if (j == bIdx) continue;
                    if (rig.bones[j].parentBoneIdx == rig.bones[bIdx].boneIdx) {
                        boneN.children.push_back(j);
                    }
                }

                m.nodes.push_back(boneN);
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

            skin.inverseBindMatrices = addBoneMatrices(m, outBoneWorldPos, n);
            skin.skeleton = 0;
            m.skins.push_back(skin);
        }

        // ------------------------------------------------------------------
        // Primitive deduplication (relative to the base body)
        // ------------------------------------------------------------------

        // Returns true if two primitives describe the same geometry.
        //
        // Vertex indices are resolved to actual coordinates, so the comparison
        // is geometry based (not index based). For polygons the vertex list is
        // matched as a set: order, winding, an explicit closing vertex and
        // consecutive duplicate points are all ignored. Other primitive types
        // compare their (short) vertex sequence up to cyclic rotation / reversal.
        bool samePrimitive(const PakModel& a, const PakModelPrimitive& pa,
                           const PakModel& b, const PakModelPrimitive& pb) const
        {
            if (pa.type != pb.type) return false;

            // Only compare attributes that actually affect the rendered result
            // for the given primitive type (see addModelMesh):
            //   Polygon(1)          -> subType (shading)
            //   Line(0)             -> geometry only (subType forced to 0, size unused)
            //   Sphere(3)           -> subType + size
            //   Pixel/Square(2,6,7) -> geometry only (subType forced to 0, size unused)
            // Color shade (colorIndex) is ignored for every type.
            const bool checkSubType = (pa.type == Sphere);
            const bool checkSize    = (pa.type == Sphere);

            if (checkSubType && pa.subType != pb.subType) return false;
            if (checkSize && pa.size != pb.size) return false;

            // Resolve vertex indices to their coordinate triplets.
            auto resolve = [](const PakModel& mdl, const PakModelPrimitive& prim,
                              vector<const s16*>& out) -> bool {
                out.resize(prim.vertexIdxs.size());
                for (int i = 0; i < (int)prim.vertexIdxs.size(); i++) {
                    int idx = prim.vertexIdxs[i] / 6;
                    if (idx < 0 || (idx * 3 + 2) >= (int)mdl.vertices.size()) return false;
                    out[i] = &mdl.vertices[idx * 3];
                }
                return true;
            };

            vector<const s16*> va, vb;
            if (!resolve(a, pa, va)) return false;
            if (!resolve(b, pb, vb)) return false;

            auto samePoint = [](const s16* p, const s16* q) {
                return p[0] == q[0] && p[1] == q[1] && p[2] == q[2];
            };

            if (pa.type == Polygon) {
                // Drop consecutive duplicates and a closing vertex equal to the
                // first one so that e.g. [A,B,C,D] and [A,B,C,D,A] match.
                auto uniqueRing = [&](const vector<const s16*>& v) {
                    vector<const s16*> r;
                    for (auto p : v) {
                        if (r.empty() || !samePoint(r.back(), p)) r.push_back(p);
                    }
                    if (r.size() > 1 && samePoint(r.front(), r.back())) r.pop_back();
                    return r;
                };
                auto ua = uniqueRing(va);
                auto ub = uniqueRing(vb);
                if (ua.size() != ub.size()) return false;

                // Order-independent bijection between the two vertex sets.
                vector<bool> used(ub.size(), false);
                for (size_t i = 0; i < ua.size(); i++) {
                    bool found = false;
                    for (size_t j = 0; j < ub.size(); j++) {
                        if (!used[j] && samePoint(ua[i], ub[j])) {
                            used[j] = true;
                            found = true;
                            break;
                        }
                    }
                    if (!found) return false;
                }
                return true;
            }

            // Line(2 vertices) / Sphere / Pixel (single vertex).
            const int n = (int)va.size();
            if (n != (int)vb.size()) return false;
            if (n == 0) return true;

            for (int rev = 0; rev < 2; rev++) {
                for (int off = 0; off < n; off++) {
                    bool ok = true;
                    for (int i = 0; i < n; i++) {
                        int j = (rev == 0)
                            ? (off + i) % n
                            : ((off - i) % n + n) % n;
                        if (!samePoint(va[i], vb[j])) { ok = false; break; }
                    }
                    if (ok) return true;
                }
            }
            return false;
        }

        // Returns the model's primitives with every primitive that is already
        // present in `baseModel` removed (dedup relative to the base body).
        vector<PakModelPrimitive> dedupPrimitives(const PakModel& model, const PakModel& baseModel) const
        {
            vector<PakModelPrimitive> out;
            out.reserve(model.primitives.size());

            for (const auto& prim : model.primitives) {
                bool duplicated = false;
                for (const auto& basePrim : baseModel.primitives) {
                    if (samePrimitive(model, prim, baseModel, basePrim)) {
                        duplicated = true;
                        break;
                    }
                }
                if (!duplicated) {
                    out.push_back(prim);
                }
            }
            return out;
        }

        // ------------------------------------------------------------------
        // Mesh building (one variant -> one skinned mesh node)
        // ------------------------------------------------------------------

        void addModelMesh(tinygltf::Model& m, const PakModel& model, const string& meshName,
                          const vector<int>& boneMap,
                          const vector<Vector3>& sharedBoneWorldPos,
                          const Pallete& pallete, const string& dirname,
                          int noiseTexIdx, bool hasNoise)
        {
            vector<Vector3> modelVerts(model.vertices.size() / 3);
            for (int i = 0; i < (int)modelVerts.size(); i++) {
                modelVerts[i] = Vector3Transform({
                    model.vertices[i * 3 + 0] / 1000.f,
                    model.vertices[i * 3 + 1] / 1000.f,
                    model.vertices[i * 3 + 2] / 1000.f
                }, modelMatrix);
            }

            const bool hasBones = !model.bones.empty();
            vector<u8> vecBoneAffect(modelVerts.size(), 0);

            if (hasBones) {
                for (int bIdx = 0; bIdx < (int)model.bones.size(); bIdx++) {
                    int vfrom = model.bones[bIdx].fromVertexIdx / 6;
                    for (int j = vfrom; j < vfrom + model.bones[bIdx].vertexCount; j++) {
                        if (j >= 0 && j < (int)vecBoneAffect.size()) {
                            vecBoneAffect[j] = (u8)bIdx;
                        }
                    }
                }

                // Re-map onto the shared skeleton and place vertices in its bind pose.
                for (int i = 0; i < (int)modelVerts.size(); i++) {
                    int sb = (vecBoneAffect[i] < boneMap.size()) ? boneMap[vecBoneAffect[i]] : 0;
                    if (sb < 0 || sb >= (int)sharedBoneWorldPos.size()) sb = 0;
                    modelVerts[i] = Vector3Add(modelVerts[i], sharedBoneWorldPos[sb]);
                    vecBoneAffect[i] = (u8)sb;
                }
            }

            int vertAccIdx = createVertexes(m, modelVerts);

            float lineSize = computeLineSize(modelVerts);
            float noisesize = hasNoise ? computeNoiseSize(modelVerts) : 0.0f;

            VertexSkin vSkin = { 0, 0 };
            if (hasBones) {
                vSkin = addVertexSkin(m, vecBoneAffect);
            }

            tinygltf::Mesh mesh;

            // --- polygons ---
            for (int pIdx = 0; pIdx < (int)model.primitives.size(); pIdx++) {
                auto& prim = model.primitives[pIdx];
                if (prim.type != 1) continue;
                auto prim2 = createPrimitivePoly(m, prim, modelVerts, vertAccIdx, pallete,
                                                 vecBoneAffect, vSkin, hasBones, noiseTexIdx, noisesize);
                mesh.primitives.push_back(prim2);
            }

            // --- spheres ---
            for (int pIdx = 0; pIdx < (int)model.primitives.size(); pIdx++) {
                auto& prim = model.primitives[pIdx];
                if (prim.type != 3) continue;

                const int vertCount = 5;
                auto matIdx = getMaterialIdx(m, prim.colorIndex, pallete, prim.subType, noiseTexIdx);
                auto& pos = modelVerts[prim.vertexIdxs[0] / 6];
                float size = prim.size / 1000.0f;
                auto prim2 = createSpherePrim(m, size, vertCount, pos, matIdx);

                if (hasBones) {
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
                if (prim.vertexIdxs.size() != 2) {
                    throw new exception("Line indexes not 2");
                }

                auto matIdx = getMaterialIdx(m, prim.colorIndex, pallete, 0, noiseTexIdx);
                Vector3 points[2] = {
                    modelVerts[prim.vertexIdxs[0] / 6],
                    modelVerts[prim.vertexIdxs[1] / 6]
                };
                auto prim2 = createPipePrim(m, points, lineSize, 4, matIdx);

                if (hasBones) {
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

                if (hasBones) {
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
            if (hasBones) {
                meshNode.skin = 0;
            }
            m.nodes.push_back(meshNode);
        }

        // ------------------------------------------------------------------
        // Animations (baked once for the whole character)
        // ------------------------------------------------------------------

        void addCharacterAnimations(tinygltf::Model& m, const vector<Animation*>& list,
                                    vector<Vector3>& sharedBonePos, int sharedBoneCount)
        {
            for (size_t i = 0; i < list.size(); i++) {
                Animation* a = list[i];
                if (!a || a->frames.empty()) continue;
                addAnimation(m, *a, sharedBonePos, sharedBoneCount);
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

            // rig source = variant with the most bones (base wins on tie)
            int rigIdx = 0;
            size_t maxBones = 0;
            for (int k = 0; k < (int)models.size(); k++) {
                if (models[k]->bones.size() > maxBones) {
                    maxBones = models[k]->bones.size();
                    rigIdx = k;
                }
            }
            if (maxBones == 0) return;

            const string dirname = outRoot + "/" + charDef.name + (alt ? "_alt" : "");
            if (std::filesystem::exists(dirname)) return;

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

            vector<Vector3> sharedBonePos;
            vector<Vector3> sharedBoneWorldPos;
            buildSharedSkeleton(m, *models[rigIdx], sharedBonePos, sharedBoneWorldPos);
            const int sharedBoneCount = (int)sharedBonePos.size();

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

            // Reference for dedup = the JSON baseModel, if it is rendered in the
            // current export mode. If the base body is not part of the current
            // mode (e.g. it has no alt variant while a variant does), variants
            // must keep their full geometry - so no dedup.
            const PakModel* baseModel = nullptr;
            {
                int baseId = findModelId(charDef.baseModel);
                if (baseId >= 0) {
                    if (alt) {
                        if (bodyHasAlt[baseId]) baseModel = &body2Models[baseId];
                    } else {
                        baseModel = &bodyModels[baseId];
                    }
                }
            }

            // one skinned mesh node per variant
            for (int k = 0; k < (int)models.size(); k++) {
                auto& model = *models[k];

                // Variants repeat the whole base body; keep only the geometry
                // that is not already present in the base model.
                PakModel deduped;
                const PakModel* meshModel = &model;
                if (baseModel && &model != baseModel) {
                    deduped = model;
                    deduped.primitives = dedupPrimitives(model, *baseModel);
                    meshModel = &deduped;
                }

                auto boneMap = buildBoneMap(model, *models[rigIdx]);

                bool hasNoise = false;
                for (auto& prim : meshModel->primitives) {
                    if (prim.type == 1 && prim.subType == 1) { hasNoise = true; break; }
                }

                addModelMesh(m, *meshModel, meshNames[k], boneMap, sharedBoneWorldPos,
                             pallete, dirname, noiseTexIdx, hasNoise);
            }

            addCharacterAnimations(m, charAnims, sharedBonePos, sharedBoneCount);

            writeCharacter(m, charDef, dirname, sharedBoneCount, meshNames, usedAnimIds);
        }

        void writeCharacter(tinygltf::Model& m, const CharacterDef& charDef, const string& dirname,
                            int boneCount, const vector<string>& meshNames, const vector<int>& animIds)
        {
            std::filesystem::create_directories(dirname);

            tinygltf::TinyGLTF gltf;
            gltf.WriteGltfSceneToFile(&m, dirname + "/character.gltf",
                false, // embedImages
                false, // embedBuffers
                false, // pretty print
                false  // binary (glb)
            );

            json dataJson;
            dataJson["name"] = charDef.name;
            dataJson["bones"] = boneCount;
            dataJson["baseModel"] = charDef.baseModel;
            dataJson["meshes"] = meshNames;
            dataJson["animations"] = animIds;

            std::ofstream o(dirname + "/data.json");
            o << std::setw(2) << dataJson;
        }
    };

}
