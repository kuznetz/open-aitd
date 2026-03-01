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

    struct KeyFrame {
        float time;
        vector<Transform> bonePoses;
    };

    struct Animation {
        cgltf_animation* anim;
        vector<BoneChannels> boneChannels;
        vector<KeyFrame> keyframes;
        vector<Transform> rootMotion;
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

        void PoseLerp(Transform* result, const Transform* poseFrom, const Transform* poseTo, const float n) const {
            if (!skin) return;
            for (int i = 0; i < skin->joints_count; i++) {
                result[i].translation = Vector3Lerp(poseFrom[i].translation, poseTo[i].translation, n);
                result[i].rotation = QuaternionSlerp(poseFrom[i].rotation, poseTo[i].rotation, n);
                result[i].scale = Vector3Lerp(poseFrom[i].scale, poseTo[i].scale, n);
            }
        }

        int GetKeyFrame(const Animation& anim, float animTime) const {
            for (size_t i = 1; i < anim.keyframes.size(); i++) {
                if (animTime < anim.keyframes[i].time) {
                    return static_cast<int>(i) - 1;
                }
            }
            return static_cast<int>(anim.keyframes.size()) - 1;
        }

        void GetKeyFramePose(raylib::Transform* pose, const Animation& anim, const int keyFrame) {
            if (!skin) return;
            if (keyFrame < 0 || keyFrame >= anim.keyframes.size()) return;
            
            const auto& bonePoses = anim.keyframes[keyFrame].bonePoses;
            std::copy(bonePoses.begin(), bonePoses.end(), pose);
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
                
                // === Инициализация keyframes ===
                animations[i].keyframes.resize(inp->count);
                for (int j = 0; j < (int)inp->count; j++) {
                    cgltf_accessor_read_float(inp, j, &animations[i].keyframes[j].time, 1);
                    animations[i].keyframes[j].bonePoses.reserve(skin->joints_count);
                }

                // === Привязка каналов к костям ===
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
                    }

                    float t = 0.0f;
                    cgltf_bool r = cgltf_accessor_read_float(channel.sampler->input, channel.sampler->input->count - 1, &t, 1);
                    if (r && t > animations[i].duration) {
                        animations[i].duration = t;
                    }
                }

                LoadKeyframePoses(animations[i]);
            }
        }

        void LoadKeyframePoses(Animation& animation) {
            if (!skin) return;
            
            const int boneCount = skin->joints_count;
            const int keyframeCount = animation.keyframes.size();
            
            for (int kf = 0; kf < keyframeCount; kf++) {
                animation.keyframes[kf].bonePoses.resize(boneCount);
                ComputePoseFromGLTF(animation.keyframes[kf].bonePoses.data(), animation, kf);
            }
        }

        void CalcBounds() {
            Bounds& result = bounds;
            if (model.meshCount == 0) return;

            raylib::Vector3& minVertex = result.min;
            raylib::Vector3& maxVertex = result.max;
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

            raylib::Vector3& minVertex = box.min;
            raylib::Vector3& maxVertex = box.max;
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

        static bool ReadKeyFrameValue(const cgltf_accessor* output, const int keyFrame, void* data) {
            if (!output) return false;
            if (output->component_type != cgltf_component_type_r_32f) return false;
            
            if (output->type == cgltf_type_vec3) {
                float tmp[3] = { 0.0f };
                cgltf_accessor_read_float(output, keyFrame, tmp, 3);
                *(Vector3*)data = { tmp[0], tmp[1], tmp[2] };
                return true;
            }
            else if (output->type == cgltf_type_vec4) {
                float tmp[4] = { 0.0f };
                cgltf_accessor_read_float(output, keyFrame, tmp, 4);
                *(Vector4*)data = { tmp[0], tmp[1], tmp[2], tmp[3] };
                return true;
            }
            
            return false;
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

        void ComputePoseFromGLTF(Transform* pose, const Animation& anim, int keyFrameIndex) const {
            if (!skin) return;
            if (keyFrameIndex < 0 || keyFrameIndex >= anim.keyframes.size()) return;
            
            const auto* boneChannels = anim.boneChannels.data();
            
            for (int k = 0; k < skin->joints_count; k++) {
                // Bind pose по умолчанию
                raylib::Vector3 translation = { skin->joints[k]->translation[0], 
                                                skin->joints[k]->translation[1], 
                                                skin->joints[k]->translation[2] };
                Quaternion rotation = { skin->joints[k]->rotation[0], 
                                    skin->joints[k]->rotation[1], 
                                    skin->joints[k]->rotation[2], 
                                    skin->joints[k]->rotation[3] };
                raylib::Vector3 scale = { skin->joints[k]->scale[0], 
                                        skin->joints[k]->scale[1], 
                                        skin->joints[k]->scale[2] };

                if (boneChannels[k].translate) {
                    ReadKeyFrameValue(boneChannels[k].translate->sampler->output, 
                                    keyFrameIndex, &translation);
                }                
                if (boneChannels[k].rotate) {
                    ReadKeyFrameValue(boneChannels[k].rotate->sampler->output, 
                                    keyFrameIndex, &rotation);
                }                
                if (boneChannels[k].scale) {
                    ReadKeyFrameValue(boneChannels[k].scale->sampler->output, 
                                    keyFrameIndex, &scale);
                }                
                pose[k] = { translation, rotation, scale };
            }
        }    
    };

}