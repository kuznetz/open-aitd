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

    struct KeyFrame {
        float time;
        vector<Transform> bonePoses;
    };

    struct Animation {
        vector<KeyFrame> keyframes;
        vector<Transform> rootMotion;
        float duration;
    };

    struct Bone {
        int parent;
    };

    struct GLTFBoneChannels {
        cgltf_animation_channel* translate;
        cgltf_animation_channel* rotate;
        cgltf_animation_channel* scale;
        cgltf_interpolation_type interpolationType;
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

        int GetKeyFrame(const Animation& anim, float animTime) const {
            for (size_t i = 1; i < anim.keyframes.size(); i++) {
                if (animTime < anim.keyframes[i].time) {
                    return static_cast<int>(i) - 1;
                }
            }
            return static_cast<int>(anim.keyframes.size()) - 1;
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
                animations[i].duration = 0;

                vector<GLTFBoneChannels> boneChannels(skin->joints_count);
                auto inp = animData.channels[0].sampler->input;
                
                animations[i].keyframes.resize(inp->count);
                for (int j = 0; j < (int)inp->count; j++) {
                    cgltf_accessor_read_float(inp, j, &animations[i].keyframes[j].time, 1);
                    animations[i].keyframes[j].bonePoses.reserve(skin->joints_count);
                }

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

                LoadKeyframePoses(animations[i], boneChannels);
            }
        }

        void LoadKeyframePoses(Animation& animation, const vector<GLTFBoneChannels>& boneChannels) {
            if (!skin) return;
            
            const int boneCount = skin->joints_count;
            const int keyframeCount = animation.keyframes.size();
            
            for (int kf = 0; kf < keyframeCount; kf++) {
                animation.keyframes[kf].bonePoses.resize(boneCount);
                ComputePoseFromGLTF(animation.keyframes[kf].bonePoses.data(), animation, kf, boneChannels);
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

        void ComputePoseFromGLTF(Transform* pose, const Animation& anim, int keyFrameIndex, const vector<GLTFBoneChannels>& boneChannels) const {
            if (!skin) return;
            if (keyFrameIndex < 0 || keyFrameIndex >= anim.keyframes.size()) return;
            
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