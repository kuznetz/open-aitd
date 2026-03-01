#pragma once
#include <vector>
#include <string>
#include <cstring>
#include <cmath>
#include "../raylib-cpp/raylib-cpp.h"
#include "./bounds.hpp"
#include "./model_math.hpp"

namespace openAITD {

    using namespace ::std;
    using namespace ::raylib;

    struct BoneChannels {
        cgltf_animation_channel* translate;
        cgltf_animation_channel* rotate;
        cgltf_animation_channel* scale;
        cgltf_interpolation_type interpolationType;
    };

    struct Animation {
        cgltf_animation* anim;
        vector<BoneChannels> boneChannels;
        vector <float> keyFrames;
        vector <Transform> rootMotion;
        vector <vector<Transform>> bakedPoses;
        vector <Bounds> bakedBounds;
        float transition;
        float duration;
    };

    struct Bone {
        int parent;
    };

    class Model {
    public:
        raylib::Model model;
        cgltf_data* data = nullptr;
        cgltf_skin* skin = nullptr;
        vector<Bone> bones;
        vector<Animation> animations;
        Bounds bounds;
        
        ~Model() {
            if (!data) return;
            UnloadModel(model);
            cgltf_free(data);
        }

        void Load(const char* fileName) {
            model = raylib::LoadModel(fileName);
            data = LoadData(fileName);
            if (!data) {
                return;
            }

            CalcBounds();

            if (data->skins_count > 1)
            {
                TRACELOG(LOG_WARNING, "MODEL: expected exactly one skin to load animation data from, but found %i", data->skins_count);
            }
            if (!data->skins_count) return;
            
            skin = &data->skins[0];
            makeBones();
            makeAnimations();
        }

        void CalcPoseByTime(Transform* pose, int animationId, float time) const {
            if (animationId < 0 || animationId >= animations.size()) return;
            auto boneChannels = animations[animationId].boneChannels.data();
            
            for (int k = 0; k < skin->joints_count; k++)
            {
                Vector3 translation = { skin->joints[k]->translation[0], skin->joints[k]->translation[1], skin->joints[k]->translation[2] };
                Quaternion rotation = { skin->joints[k]->rotation[0], skin->joints[k]->rotation[1], skin->joints[k]->rotation[2], skin->joints[k]->rotation[3] };
                Vector3 scale = { skin->joints[k]->scale[0], skin->joints[k]->scale[1], skin->joints[k]->scale[2] };

                if (boneChannels[k].translate)
                {
                    if (!GetPoseAtTimeGLTF(boneChannels[k].interpolationType, boneChannels[k].translate->sampler->input, boneChannels[k].translate->sampler->output, time, &translation))
                    {
                        TRACELOG(LOG_INFO, "MODEL: Failed to load translate pose data for bone %d", k);
                    }
                }

                if (boneChannels[k].rotate)
                {
                    if (!GetPoseAtTimeGLTF(boneChannels[k].interpolationType, boneChannels[k].rotate->sampler->input, boneChannels[k].rotate->sampler->output, time, &rotation))
                    {
                        TRACELOG(LOG_INFO, "MODEL: Failed to load rotate pose data for bone %d", k);
                    }
                }

                if (boneChannels[k].scale)
                {
                    if (!GetPoseAtTimeGLTF(boneChannels[k].interpolationType, boneChannels[k].scale->sampler->input, boneChannels[k].scale->sampler->output, time, &scale))
                    {
                        TRACELOG(LOG_INFO, "MODEL: Failed to load scale pose data for bone %d", k);
                    }
                }

                pose[k] = { translation, rotation, scale };
            }
        }

        void PoseLerp(Transform* result, const Transform* poseFrom, const Transform* poseTo, const float n) const {
            if (!skin) return;
            for (int i = 0; i < skin->joints_count; i++) {
                result[i].translation = Vector3Lerp(poseFrom[i].translation, poseTo[i].translation, n);
                result[i].rotation = QuaternionSlerp(poseFrom[i].rotation, poseTo[i].rotation, n);
                result[i].scale = Vector3Lerp(poseFrom[i].scale, poseTo[i].scale, n);
            }
        }

        void BakePoses(int fps) {
            for (size_t i = 0; i < animations.size(); i++) {
                auto& anim = animations[i];
                int frameCount = ceil(anim.duration * fps);
                if (frameCount <= 0) frameCount = 1;
                
                anim.bakedPoses.resize(frameCount);
                anim.rootMotion.resize(frameCount);
                
                for (int j = 0; j < frameCount; j++) {
                    float t = (float)j / fps;
                    anim.bakedPoses[j].resize(bones.size());
                    CalcPoseByTime(anim.bakedPoses[j].data(), (int)i, t); 
                    anim.rootMotion[j] = anim.bakedPoses[j][0];
                    anim.bakedPoses[j][0].translation = { 0,0,0 };
                }
            }
        }

        int GetKeyFrame(const Animation& anim, float animTime) const {
            int i = 1;
            for (i = 1; i < (int)anim.keyFrames.size(); i++) {
                if (animTime < anim.keyFrames[i]) {
                    return i - 1;
                }
            }
            return i - 1;
        }

        void RenderStatic() const {
            if (!data) return;
            for (int i = 0; i < model.meshCount; i++)
            {
                DrawMesh(model.meshes[i], model.materials[model.meshMaterial[i]], model.transform);
            }
        }

    private:
        static cgltf_data* LoadData(const char* fileName) {
            int dataSize = 0;
            unsigned char* fileData = LoadFileData(fileName, &dataSize);

            cgltf_options options = { cgltf_file_type_gltf };
            options.file.read = LoadFileGLTFCallback;
            options.file.release = ReleaseFileGLTFCallback;
            cgltf_data* data = NULL;
            cgltf_result result = cgltf_parse(&options, fileData, dataSize, &data);
            UnloadFileData(fileData);

            if (result != cgltf_result_success)
            {
                TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to load glTF data", fileName);
                if(data) cgltf_free(data);
                return nullptr;
            }

            result = cgltf_load_buffers(&options, data, fileName);
            if (result != cgltf_result_success) {
                TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to load animation buffers", fileName);
                cgltf_free(data);
                return nullptr;
            }

            return data;
        }

        void makeBones()
        {
            if(!skin) return;
            bones.resize(skin->joints_count);
            for (unsigned int i = 0; i < skin->joints_count; i++)
            {
                cgltf_node& node = *skin->joints[i];
                int parentIndex = -1;
                for (unsigned int j = 0; j < skin->joints_count; j++)
                {
                    if (skin->joints[j] == node.parent)
                    {
                        parentIndex = (int)j;
                        break;
                    }
                }
                bones[i].parent = parentIndex;
            }
        }

        void makeAnimations()
        {
            if (!data || !skin) return;
            animations.resize(data->animations_count);
            for (unsigned int i = 0; i < data->animations_count; i++)
            {
                cgltf_animation& animData = data->animations[i];
                animations[i].anim = &animData;
                animations[i].boneChannels.resize((int)skin->joints_count);
                animations[i].duration = 0;
                BoneChannels* boneChannels = animations[i].boneChannels.data();

                auto inp = animData.channels[0].sampler->input;
                animations[i].keyFrames.resize(inp->count);
                for (int j = 0; j < (int)inp->count; j++) {
                    cgltf_accessor_read_float(inp, j, &animations[i].keyFrames[j], 1);
                }

                animations[i].transition = (inp->count > 1) ? animations[i].keyFrames[1] : 0;

                for (unsigned int j = 0; j < animData.channels_count; j++)
                {
                    cgltf_animation_channel channel = animData.channels[j];
                    int boneIndex = -1;

                    for (unsigned int k = 0; k < skin->joints_count; k++)
                    {
                        if (animData.channels[j].target_node == skin->joints[k])
                        {
                            boneIndex = (int)k;
                            break;
                        }
                    }

                    if (boneIndex == -1) continue;

                    boneChannels[boneIndex].interpolationType = animData.channels[j].sampler->interpolation;

                    if (animData.channels[j].sampler->interpolation != cgltf_interpolation_type_max_enum)
                    {
                        if (channel.target_path == cgltf_animation_path_type_translation)
                            boneChannels[boneIndex].translate = &animData.channels[j];
                        else if (channel.target_path == cgltf_animation_path_type_rotation)
                            boneChannels[boneIndex].rotate = &animData.channels[j];
                        else if (channel.target_path == cgltf_animation_path_type_scale)
                            boneChannels[boneIndex].scale = &animData.channels[j];
                        else
                            TRACELOG(LOG_WARNING, "MODEL: Unsupported target_path on channel %d", j);
                    }

                    float t = 0.0f;
                    cgltf_bool r = cgltf_accessor_read_float(channel.sampler->input, channel.sampler->input->count - 1, &t, 1);
                    if (r && t > animations[i].duration) {
                        animations[i].duration = t;
                    }
                }
            }
        }

        void CalcBounds() {
            Bounds& result = bounds;
            if (model.meshCount == 0) return;

            Vector3& minVertex = result.min;
            Vector3& maxVertex = result.max;
            Bounds newMeshBounds;
            Mesh* mesh;
            bool first = true;

            for (int i = 0; i < model.meshCount; i++) {
                mesh = &model.meshes[i];
                if (mesh->vertices == NULL) continue;
                
                GetMeshBounds(newMeshBounds, *mesh);

                if (first) {
                    result = newMeshBounds;
                    first = false;
                    continue;
                }

                minVertex.x = std::min(minVertex.x, newMeshBounds.min.x);
                minVertex.y = std::min(minVertex.y, newMeshBounds.min.y);
                minVertex.z = std::min(minVertex.z, newMeshBounds.min.z);
                maxVertex.x = std::max(maxVertex.x, newMeshBounds.max.x);
                maxVertex.y = std::max(maxVertex.y, newMeshBounds.max.y);
                maxVertex.z = std::max(maxVertex.z, newMeshBounds.max.z);
            }

            Vector3TransformRef(minVertex, model.transform);
            Vector3TransformRef(maxVertex, model.transform);
        }

        static void GetMeshBounds(Bounds& box, const Mesh& mesh)
        {
            auto& verts = mesh.animVertices; 
            if (verts == NULL) throw new exception("Verts is null");

            Vector3& minVertex = box.min;
            Vector3& maxVertex = box.max;
            minVertex = { verts[0], verts[1], verts[2] };
            maxVertex = { verts[0], verts[1], verts[2] };
            for (int i = 1; i < mesh.vertexCount; i++)
            {
                const float& x = verts[i*3];
                const float& y = verts[i*3 + 1];
                const float& z = verts[i*3 + 2];

                minVertex.x = std::min(minVertex.x, x);
                minVertex.y = std::min(minVertex.y, y);
                minVertex.z = std::min(minVertex.z, z);
                maxVertex.x = std::max(maxVertex.x, x);
                maxVertex.y = std::max(maxVertex.y, y);
                maxVertex.z = std::max(maxVertex.z, z);
            }
        }

        static cgltf_result LoadFileGLTFCallback(const struct cgltf_memory_options*, const struct cgltf_file_options*, const char* path, cgltf_size* size, void** data)
        {
            int filesize;
            unsigned char* filedata = LoadFileData(path, &filesize);
            if (filedata == NULL) return cgltf_result_io_error;
            *size = filesize;
            *data = filedata;
            return cgltf_result_success;
        }

        static void ReleaseFileGLTFCallback(const struct cgltf_memory_options*, const struct cgltf_file_options*, void* data)
        {
            UnloadFileData((unsigned char*)data);
        }

        static bool GetPoseAtTimeGLTF(cgltf_interpolation_type interpolationType, cgltf_accessor* input, cgltf_accessor* output, float time, void* data)
        {
            if (interpolationType >= cgltf_interpolation_type_max_enum) return false;

            float tstart = 0.0f;
            float tend = 0.0f;
            int keyframe = 0;

            for (int i = 0; i < (int)input->count - 1; i++)
            {
                cgltf_bool r1 = cgltf_accessor_read_float(input, i, &tstart, 1);
                if (!r1) return false;
                cgltf_bool r2 = cgltf_accessor_read_float(input, i + 1, &tend, 1);
                if (!r2) return false;

                if ((tstart <= time) && (time < tend))
                {
                    keyframe = i;
                    break;
                }
            }

            if (FloatEquals(tend, tstart)) return true;

            float duration = fmaxf((tend - tstart), EPSILON);
            float t = (time - tstart) / duration;
            t = (t < 0.0f) ? 0.0f : t;
            t = (t > 1.0f) ? 1.0f : t;

            if (output->component_type != cgltf_component_type_r_32f) return false;

            if (output->type == cgltf_type_vec3)
            {
                switch (interpolationType)
                {
                case cgltf_interpolation_type_step:
                {
                    float tmp[3] = { 0.0f };
                    cgltf_accessor_read_float(output, keyframe, tmp, 3);
                    *(Vector3*)data = { tmp[0], tmp[1], tmp[2] };
                } break;
                case cgltf_interpolation_type_linear:
                {
                    float tmp[3] = { 0.0f };
                    cgltf_accessor_read_float(output, keyframe, tmp, 3);
                    Vector3 v1 = { tmp[0], tmp[1], tmp[2] };
                    cgltf_accessor_read_float(output, keyframe + 1, tmp, 3);
                    Vector3 v2 = { tmp[0], tmp[1], tmp[2] };
                    *(Vector3*)data = Vector3Lerp(v1, v2, t);
                } break;
                case cgltf_interpolation_type_cubic_spline:
                {
                    float tmp[3] = { 0.0f };
                    cgltf_accessor_read_float(output, 3 * keyframe + 1, tmp, 3);
                    Vector3 v1 = { tmp[0], tmp[1], tmp[2] };
                    cgltf_accessor_read_float(output, 3 * keyframe + 2, tmp, 3);
                    Vector3 tangent1 = { tmp[0], tmp[1], tmp[2] };
                    cgltf_accessor_read_float(output, 3 * (keyframe + 1) + 1, tmp, 3);
                    Vector3 v2 = { tmp[0], tmp[1], tmp[2] };
                    cgltf_accessor_read_float(output, 3 * (keyframe + 1), tmp, 3);
                    Vector3 tangent2 = { tmp[0], tmp[1], tmp[2] };
                    *(Vector3*)data = Vector3CubicHermite(v1, tangent1, v2, tangent2, t);
                } break;
                default: break;
                }
            }
            else if (output->type == cgltf_type_vec4)
            {
                switch (interpolationType)
                {
                case cgltf_interpolation_type_step:
                {
                    float tmp[4] = { 0.0f };
                    cgltf_accessor_read_float(output, keyframe, tmp, 4);
                    *(Vector4*)data = { tmp[0], tmp[1], tmp[2], tmp[3] };
                } break;
                case cgltf_interpolation_type_linear:
                {
                    float tmp[4] = { 0.0f };
                    cgltf_accessor_read_float(output, keyframe, tmp, 4);
                    Vector4 v1 = { tmp[0], tmp[1], tmp[2], tmp[3] };
                    cgltf_accessor_read_float(output, keyframe + 1, tmp, 4);
                    Vector4 v2 = { tmp[0], tmp[1], tmp[2], tmp[3] };
                    *(Vector4*)data = QuaternionSlerp(v1, v2, t);
                } break;
                case cgltf_interpolation_type_cubic_spline:
                {
                    float tmp[4] = { 0.0f };
                    cgltf_accessor_read_float(output, 3 * keyframe + 1, tmp, 4);
                    Vector4 v1 = { tmp[0], tmp[1], tmp[2], tmp[3] };
                    cgltf_accessor_read_float(output, 3 * keyframe + 2, tmp, 4);
                    Vector4 outTangent1 = { tmp[0], tmp[1], tmp[2], 0.0f };
                    cgltf_accessor_read_float(output, 3 * (keyframe + 1) + 1, tmp, 4);
                    Vector4 v2 = { tmp[0], tmp[1], tmp[2], tmp[3] };
                    cgltf_accessor_read_float(output, 3 * (keyframe + 1), tmp, 4);
                    Vector4 inTangent2 = { tmp[0], tmp[1], tmp[2], 0.0f };
                    
                    v1 = QuaternionNormalize(v1);
                    v2 = QuaternionNormalize(v2);
                    if (Vector4DotProduct(v1, v2) < 0.0f) v2 = Vector4Negate(v2);

                    outTangent1 = Vector4Scale(outTangent1, duration);
                    inTangent2 = Vector4Scale(inTangent2, duration);

                    *(Vector4*)data = QuaternionCubicHermiteSpline(v1, outTangent1, v2, inTangent2, t);
                } break;
                default: break;
                }
            }
            return true;
        }
    };

}