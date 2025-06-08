﻿#pragma once
#include <vector>
#include <string>
#include <cstring>
#include "../raylib.h"
#include <external/cgltf.h>
#include "bounds.h"

//using namespace raylib;

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

    void Vector3TransformRef(Vector3& result, const Matrix& mat)
    {
        float x = result.x;
        float y = result.y;
        float z = result.z;
        result.x = mat.m0 * x + mat.m4 * y + mat.m8 * z + mat.m12;
        result.y = mat.m1 * x + mat.m5 * y + mat.m9 * z + mat.m13;
        result.z = mat.m2 * x + mat.m6 * y + mat.m10 * z + mat.m14;
    }

    void MatrixInvertRef(Matrix& result)
    {
        Matrix mat = result;
        // Cache the matrix values (speed optimization)
        float a00 = mat.m0, a01 = mat.m1, a02 = mat.m2, a03 = mat.m3;
        float a10 = mat.m4, a11 = mat.m5, a12 = mat.m6, a13 = mat.m7;
        float a20 = mat.m8, a21 = mat.m9, a22 = mat.m10, a23 = mat.m11;
        float a30 = mat.m12, a31 = mat.m13, a32 = mat.m14, a33 = mat.m15;

        float b00 = a00 * a11 - a01 * a10;
        float b01 = a00 * a12 - a02 * a10;
        float b02 = a00 * a13 - a03 * a10;
        float b03 = a01 * a12 - a02 * a11;
        float b04 = a01 * a13 - a03 * a11;
        float b05 = a02 * a13 - a03 * a12;
        float b06 = a20 * a31 - a21 * a30;
        float b07 = a20 * a32 - a22 * a30;
        float b08 = a20 * a33 - a23 * a30;
        float b09 = a21 * a32 - a22 * a31;
        float b10 = a21 * a33 - a23 * a31;
        float b11 = a22 * a33 - a23 * a32;

        // Calculate the invert determinant (inlined to avoid double-caching)
        float invDet = 1.0f / (b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06);

        result.m0 = (a11 * b11 - a12 * b10 + a13 * b09) * invDet;
        result.m1 = (-a01 * b11 + a02 * b10 - a03 * b09) * invDet;
        result.m2 = (a31 * b05 - a32 * b04 + a33 * b03) * invDet;
        result.m3 = (-a21 * b05 + a22 * b04 - a23 * b03) * invDet;
        result.m4 = (-a10 * b11 + a12 * b08 - a13 * b07) * invDet;
        result.m5 = (a00 * b11 - a02 * b08 + a03 * b07) * invDet;
        result.m6 = (-a30 * b05 + a32 * b02 - a33 * b01) * invDet;
        result.m7 = (a20 * b05 - a22 * b02 + a23 * b01) * invDet;
        result.m8 = (a10 * b10 - a11 * b08 + a13 * b06) * invDet;
        result.m9 = (-a00 * b10 + a01 * b08 - a03 * b06) * invDet;
        result.m10 = (a30 * b04 - a31 * b02 + a33 * b00) * invDet;
        result.m11 = (-a20 * b04 + a21 * b02 - a23 * b00) * invDet;
        result.m12 = (-a10 * b09 + a11 * b07 - a12 * b06) * invDet;
        result.m13 = (a00 * b09 - a01 * b07 + a02 * b06) * invDet;
        result.m14 = (-a30 * b03 + a31 * b01 - a32 * b00) * invDet;
        result.m15 = (a20 * b03 - a21 * b01 + a22 * b00) * invDet;
    }

    void MatrixTransposeRef(Matrix& mat)
    {
        // Меняем элементы попарно
        std::swap(mat.m1, mat.m4);
        std::swap(mat.m2, mat.m8);   // строки 1-3 меняются столбцами
        std::swap(mat.m3, mat.m12);  // строки 1-4 меняются столбцами
        std::swap(mat.m6, mat.m9);   // строки 2-3 меняются столбцами
        std::swap(mat.m7, mat.m13);  // строки 2-4 меняются столбцами
        std::swap(mat.m11, mat.m14); // строки 3-4 меняются столбцами
    }

    class Model {
    public:
        raylib::Model model;
        cgltf_data* data = 0;
        cgltf_skin* skin = 0;
        vector<Bone> bones;
        vector<Animation> animations;
        Transform* curPose = 0;
        Bounds bounds;
        
        ~Model() {
            if (!data) return;
            UnloadModel(model);
            if (curPose) delete[] curPose;
            cgltf_free(data);
        }

        void load(const char* fileName) {
            model = raylib::LoadModel(fileName);
            data = LoadData(fileName);
            if (!data) {
                return;
            }
            auto b = GetBounds();
            bounds.min = b.min;
            bounds.max = b.max;

            if (data->skins_count > 1)
            {
                TRACELOG(LOG_WARNING, "MODEL: [%s] expected exactly one skin to load animation data from, but found %i", fileName, data->skins_count);
            }
            if (!data->skins_count) return;
            skin = &data->skins[0];
            makeBones();
            makeAnimations();
        }

        void CalcPoseByTime(Transform* pose, int animationId, float time) const {
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
                        TRACELOG(LOG_INFO, "MODEL: [%s] Failed to load translate pose data for bone %s", fileName, animations[i].bones[k].name);
                    }
                }

                if (boneChannels[k].rotate)
                {
                    if (!GetPoseAtTimeGLTF(boneChannels[k].interpolationType, boneChannels[k].rotate->sampler->input, boneChannels[k].rotate->sampler->output, time, &rotation))
                    {
                        TRACELOG(LOG_INFO, "MODEL: [%s] Failed to load rotate pose data for bone %s", fileName, animations[i].bones[k].name);
                    }
                }

                if (boneChannels[k].scale)
                {
                    if (!GetPoseAtTimeGLTF(boneChannels[k].interpolationType, boneChannels[k].scale->sampler->input, boneChannels[k].scale->sampler->output, time, &scale))
                    {
                        TRACELOG(LOG_INFO, "MODEL: [%s] Failed to load scale pose data for bone %s", fileName, animations[i].bones[k].name);
                    }
                }

                pose[k] = {
                    translation,
                    rotation,
                    scale
                };
            }
        }

        static void GetMeshBounds(Bounds& box, const Mesh& mesh)
        {
            if (mesh.vertices != NULL) return;

            // Get min and max vertex to construct bounds (AABB)
            Vector3& minVertex = box.min;
            Vector3& maxVertex = box.max;

            minVertex = { mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };
            maxVertex = { mesh.vertices[0], mesh.vertices[1], mesh.vertices[2] };
            for (int i = 1; i < mesh.vertexCount; i++)
            {
                const float& x = mesh.vertices[i];
                const float& y = mesh.vertices[i + 1];
                const float& z = mesh.vertices[i + 2];

                minVertex.x = std::min(minVertex.x, x);
                minVertex.y = std::min(minVertex.y, y);
                minVertex.z = std::min(minVertex.z, z);
                maxVertex.x = std::max(maxVertex.x, x);
                maxVertex.y = std::max(maxVertex.y, y);
                maxVertex.z = std::max(maxVertex.z, z);
            }
        }

        Bounds GetBounds() {
            Bounds result;
            if (model.meshCount == 0) return result;

            Vector3& minVertex = result.min;
            Vector3& maxVertex = result.max;
            Bounds meshBounds;
            Mesh* mesh;
            bool first = true;

            for (int i = 0; i < model.meshCount; i++) {
                mesh = &model.meshes[i];
                if (mesh->vertices != NULL) continue;
                GetMeshBounds(meshBounds, *mesh);
                if (first) {
                    result = meshBounds;
                    first = false;
                    continue;
                }

                minVertex.x = std::min(minVertex.x, meshBounds.min.x);
                minVertex.y = std::min(minVertex.y, meshBounds.min.y);
                minVertex.z = std::min(minVertex.z, meshBounds.min.z);
                maxVertex.x = std::max(maxVertex.x, meshBounds.max.x);
                maxVertex.y = std::max(maxVertex.y, meshBounds.max.y);
                maxVertex.z = std::max(maxVertex.z, meshBounds.max.z);
            }

            Vector3TransformRef(minVertex, model.transform);
            Vector3TransformRef(maxVertex, model.transform);
            return result;
        }

        void bakePoses(int fps) {
            for (int i = 0; i < animations.size(); i++) {
                auto& anim = animations[i];
                float t = 0;
                int frameCount = ceil(anim.duration * fps) + 1;
                BoundingBox b;
                anim.bakedPoses.resize(frameCount);
                //anim.bakedBounds.resize(frameCount);
                anim.rootMotion.resize(frameCount);
                for (int j = 0; j < frameCount; j++) {
                    t = (float)j / fps;
                    anim.bakedPoses[j].resize(bones.size());
                    CalcPoseByTime(anim.bakedPoses[j].data(), i, t);
                    anim.rootMotion[j] = anim.bakedPoses[j][0];
                    anim.bakedPoses[j][0].translation = { 0,0,0 };

                    //TOO Slow
                    //ApplyPose(anim.bakedPoses[j].data());
                    //anim.bakedBounds[j] = GetBounds();
                }
            }
        }

        void PoseLerp(Transform* result, const Transform* poseFrom, const Transform* poseTo, const float n) const {
            for (int i = 0; i < skin->joints_count; i++) {
                result[i].translation = Vector3Lerp(poseFrom[i].translation, poseTo[i].translation, n);
                result[i].rotation = QuaternionSlerp(poseFrom[i].rotation, poseTo[i].rotation, n);
                result[i].scale = Vector3Lerp(poseFrom[i].scale, poseTo[i].scale, n);
            }
        }

        void ApplyPose(Transform* pose) {
            memcpy(curPose, pose, sizeof Transform * bones.size());
            ApplyParentJoints(curPose);
            UpdateBones(model, curPose);
            UpdateSkin(model);
        }

        void UpdateBuffer()
        {
            for (int m = 0; m < model.meshCount; m++)
            {
                auto& mesh = model.meshes[m];
                rlUpdateVertexBuffer(mesh.vboId[0], mesh.animVertices, mesh.vertexCount * 3 * sizeof(float), 0); // Update vertex position
                if (mesh.normals != NULL) rlUpdateVertexBuffer(mesh.vboId[2], mesh.animNormals, mesh.vertexCount * 3 * sizeof(float), 0); // Update vertex normals
            }
        }

        int getKeyFrame(const Animation& anim, float animTime) {
            int i = 1;
            for (i = 1; i < anim.keyFrames.size(); i++) {
                if (animTime < anim.keyFrames[i]) {
                    return i - 1;
                }
            }
            return i - 1;
        }

        void Render() const {
            if (!data) return;
            for (int i = 0; i < model.meshCount; i++)
            {
                DrawMesh(model.meshes[i], model.materials[model.meshMaterial[i]], model.transform);
            }
        }

    private:

        void ApplyParentJoints(Transform* transforms)
        {
            for (int i = 0; i < skin->joints_count; i++)
            {
                if (bones[i].parent >= 0)
                {
                    if (bones[i].parent > i)
                    {
                        TRACELOG(LOG_WARNING, "Assumes bones are toplogically sorted, but bone %d has parent %d. Skipping.", i, bones[i].parent);
                        continue;
                    }
                    transforms[i].rotation = QuaternionMultiply(transforms[bones[i].parent].rotation, transforms[i].rotation);
                    transforms[i].translation = Vector3RotateByQuaternion(transforms[i].translation, transforms[bones[i].parent].rotation);
                    transforms[i].translation = Vector3Add(transforms[i].translation, transforms[bones[i].parent].translation);
                    transforms[i].scale = Vector3Multiply(transforms[i].scale, transforms[bones[i].parent].scale);
                }
            }
        }

        static cgltf_data* LoadData(const char* fileName) {
            // glTF file loading
            int dataSize = 0;
            unsigned char* fileData = LoadFileData(fileName, &dataSize);

            // glTF data loading
            cgltf_options options = { cgltf_file_type_invalid };
            options.file.read = LoadFileGLTFCallback;
            options.file.release = ReleaseFileGLTFCallback;
            cgltf_data* data = NULL;
            cgltf_result result = cgltf_parse(&options, fileData, dataSize, &data);
            UnloadFileData(fileData);

            if (result != cgltf_result_success)
            {
                TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to load glTF data", fileName);
                cgltf_free(data);
                return 0;
            }

            result = cgltf_load_buffers(&options, data, fileName);
            if (result != cgltf_result_success) {
                TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to load animation buffers", fileName);
                cgltf_free(data);
                return 0;
            }

            return data;
        }

        void makeBones()
        {
            bones.resize(skin->joints_count);
            curPose = new Transform[skin->joints_count];
            for (unsigned int i = 0; i < skin->joints_count; i++)
            {
                cgltf_node& node = *skin->joints[i];
                // Find parent bone index
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
            animations.resize(data->animations_count);
            for (unsigned int i = 0; i < data->animations_count; i++)
            {
                cgltf_animation& animData = data->animations[i];
                animations[i].anim = &animData;
                animations[i].boneChannels.resize((int)skin->joints_count);
                animations[i].duration = 0;
                BoneChannels* boneChannels = animations[i].boneChannels.data();

                //transition - second keyframe
                auto inp = animData.channels[0].sampler->input;
                animations[i].keyFrames.resize(inp->count);
                //cgltf_accessor_read_float(inp, 0, animations[i].keyFrames.data(), inp->count);
                for (int j = 0; j < inp->count; j++) {
                    cgltf_accessor_read_float(inp, j, &animations[i].keyFrames[j], 1);
                }

                //transition time - second keyframe
                animations[i].transition = (inp->count > 1) ? animations[i].keyFrames[1] : 0;

                for (unsigned int j = 0; j < animData.channels_count; j++)
                {
                    cgltf_animation_channel channel = animData.channels[j];
                    int boneIndex = -1;

                    for (unsigned int k = 0; k < skin->joints_count; k++)
                    {
                        if (animData.channels[j].target_node == skin->joints[k])
                        {
                            boneIndex = k;
                            break;
                        }
                    }

                    if (boneIndex == -1)
                    {
                        // Animation channel for a node not in the armature
                        continue;
                    }

                    boneChannels[boneIndex].interpolationType = animData.channels[j].sampler->interpolation;

                    if (animData.channels[j].sampler->interpolation != cgltf_interpolation_type_max_enum)
                    {
                        if (channel.target_path == cgltf_animation_path_type_translation)
                        {
                            boneChannels[boneIndex].translate = &animData.channels[j];
                        }
                        else if (channel.target_path == cgltf_animation_path_type_rotation)
                        {
                            boneChannels[boneIndex].rotate = &animData.channels[j];
                        }
                        else if (channel.target_path == cgltf_animation_path_type_scale)
                        {
                            boneChannels[boneIndex].scale = &animData.channels[j];
                        }
                        else
                        {
                            TRACELOG(LOG_WARNING, "MODEL: [%s] Unsupported target_path on channel %d's sampler for animation %d. Skipping.", fileName, j, i);
                        }
                    }
                    else {
                        TRACELOG(LOG_WARNING, "MODEL: [%s] Invalid interpolation curve encountered for GLTF animation.", fileName);
                    }

                    float t = 0.0f;
                    cgltf_bool r = cgltf_accessor_read_float(channel.sampler->input, channel.sampler->input->count - 1, &t, 1);

                    if (!r)
                    {
                        TRACELOG(LOG_WARNING, "MODEL: [%s] Failed to load input time", fileName);
                        continue;
                    }

                    if (t > animations[i].duration) {
                        animations[i].duration = t;
                    }
                }

                //TRACELOG(LOG_INFO, "MODEL: [%s] Loaded animation: %s (%d frames, %fs)", fileName, (animData.name != NULL) ? animData.name : "NULL", animations[i].frameCount, animDuration);
                //RL_FREE(boneChannels);
            }
        }

        static cgltf_result LoadFileGLTFCallback(const struct cgltf_memory_options* memoryOptions, const struct cgltf_file_options* fileOptions, const char* path, cgltf_size* size, void** data)
        {
            int filesize;
            unsigned char* filedata = LoadFileData(path, &filesize);

            if (filedata == NULL) return cgltf_result_io_error;

            *size = filesize;
            *data = filedata;

            return cgltf_result_success;
        }

        // Release file data callback for cgltf
        static void ReleaseFileGLTFCallback(const struct cgltf_memory_options* memoryOptions, const struct cgltf_file_options* fileOptions, void* data)
        {
            UnloadFileData((unsigned char*)data);
        }

        static bool GetPoseAtTimeGLTF(cgltf_interpolation_type interpolationType, cgltf_accessor* input, cgltf_accessor* output, float time, void* data)
        {
            if (interpolationType >= cgltf_interpolation_type_max_enum) return false;

            // Input and output should have the same count
            float tstart = 0.0f;
            float tend = 0.0f;
            int keyframe = 0;       // Defaults to first pose

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

            // Constant animation, no need to interpolate
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
                    Vector3 v1 = { tmp[0], tmp[1], tmp[2] };
                    Vector3* r = (Vector3*)data;

                    *r = v1;
                } break;
                case cgltf_interpolation_type_linear:
                {
                    float tmp[3] = { 0.0f };
                    cgltf_accessor_read_float(output, keyframe, tmp, 3);
                    Vector3 v1 = { tmp[0], tmp[1], tmp[2] };
                    cgltf_accessor_read_float(output, keyframe + 1, tmp, 3);
                    Vector3 v2 = { tmp[0], tmp[1], tmp[2] };
                    Vector3* r = (Vector3*)data;

                    *r = Vector3Lerp(v1, v2, t);
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
                    Vector3* r = (Vector3*)data;

                    *r = Vector3CubicHermite(v1, tangent1, v2, tangent2, t);
                } break;
                default: break;
                }
            }
            else if (output->type == cgltf_type_vec4)
            {
                // Only v4 is for rotations, so we know it's a quaternion
                switch (interpolationType)
                {
                case cgltf_interpolation_type_step:
                {
                    float tmp[4] = { 0.0f };
                    cgltf_accessor_read_float(output, keyframe, tmp, 4);
                    Vector4 v1 = { tmp[0], tmp[1], tmp[2], tmp[3] };
                    Vector4* r = (Vector4*)data;

                    *r = v1;
                } break;
                case cgltf_interpolation_type_linear:
                {
                    float tmp[4] = { 0.0f };
                    cgltf_accessor_read_float(output, keyframe, tmp, 4);
                    Vector4 v1 = { tmp[0], tmp[1], tmp[2], tmp[3] };
                    cgltf_accessor_read_float(output, keyframe + 1, tmp, 4);
                    Vector4 v2 = { tmp[0], tmp[1], tmp[2], tmp[3] };
                    Vector4* r = (Vector4*)data;

                    *r = QuaternionSlerp(v1, v2, t);
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
                    Vector4* r = (Vector4*)data;

                    v1 = QuaternionNormalize(v1);
                    v2 = QuaternionNormalize(v2);

                    if (Vector4DotProduct(v1, v2) < 0.0f)
                    {
                        v2 = Vector4Negate(v2);
                    }

                    outTangent1 = Vector4Scale(outTangent1, duration);
                    inTangent2 = Vector4Scale(inTangent2, duration);

                    *r = QuaternionCubicHermiteSpline(v1, outTangent1, v2, inTangent2, t);
                } break;
                default: break;
                }
            }

            return true;
        }

        static void UpdateBones(raylib::Model& model, Transform* pose)
        {
            // Get first mesh which have bones
            int firstMeshWithBones = -1;
            for (int i = 0; i < model.meshCount; i++)
            {
                if (model.meshes[i].boneMatrices)
                {
                    if (firstMeshWithBones == -1)
                    {
                        firstMeshWithBones = i;
                        break;
                    }
                }
            }

            // Update all bones and boneMatrices of first mesh with bones.
            int boneCount = model.meshes[firstMeshWithBones].boneCount;
            for (int boneId = 0; boneId < boneCount; boneId++)
            {
                Vector3 inTranslation = model.bindPose[boneId].translation;
                Quaternion inRotation = model.bindPose[boneId].rotation;
                Vector3 inScale = model.bindPose[boneId].scale;

                Vector3 outTranslation = pose[boneId].translation;
                Quaternion outRotation = pose[boneId].rotation;
                Vector3 outScale = pose[boneId].scale;

                Quaternion invRotation = QuaternionInvert(inRotation);
                Vector3 invTranslation = Vector3RotateByQuaternion(Vector3Negate(inTranslation), invRotation);
                Vector3 invScale = Vector3Divide({ 1.0f, 1.0f, 1.0f }, inScale);

                Vector3 boneTranslation = Vector3Add(Vector3RotateByQuaternion(
                    Vector3Multiply(outScale, invTranslation), outRotation), outTranslation);
                Quaternion boneRotation = QuaternionMultiply(outRotation, invRotation);
                Vector3 boneScale = Vector3Multiply(outScale, invScale);

                Matrix boneMatrix = MatrixMultiply(MatrixMultiply(
                    QuaternionToMatrix(boneRotation),
                    MatrixTranslate(boneTranslation.x, boneTranslation.y, boneTranslation.z)),
                    MatrixScale(boneScale.x, boneScale.y, boneScale.z));

                model.meshes[firstMeshWithBones].boneMatrices[boneId] = boneMatrix;
            }

            // Update remaining meshes with bones
            // NOTE: Using deep copy because shallow copy results in double free with 'UnloadModel()'
            if (firstMeshWithBones != -1)
            {
                for (int i = firstMeshWithBones + 1; i < model.meshCount; i++)
                {
                    if (model.meshes[i].boneMatrices)
                    {
                       memcpy(model.meshes[i].boneMatrices,
                            model.meshes[firstMeshWithBones].boneMatrices,
                            model.meshes[i].boneCount * sizeof(model.meshes[i].boneMatrices[0]));
                    }
                }
            }
        }

        static void UpdateSkin(raylib::Model& model)
        {
            for (int m = 0; m < model.meshCount; m++)
            {
                Mesh& mesh = model.meshes[m];
                Vector3 animVertex = { 0 };
                //Vector3 animNormal = { 0 };
                int boneId = 0;
                int boneCounter = 0;
                float boneWeight = 0.0;
                bool updated = false; // Flag to check when anim vertex information is updated
                const int vValues = mesh.vertexCount * 3;
                //Matrix curMatrix;

                // Skip if missing bone data, causes segfault without on some models
                if ((mesh.boneWeights == NULL) || (mesh.boneIds == NULL)) continue;

                for (int vCounter = 0; vCounter < vValues; vCounter += 3)
                {
                    mesh.animVertices[vCounter] = 0;
                    mesh.animVertices[vCounter + 1] = 0;
                    mesh.animVertices[vCounter + 2] = 0;
                    //if (mesh.animNormals != NULL)
                    //{
                    //    mesh.animNormals[vCounter] = 0;
                    //    mesh.animNormals[vCounter + 1] = 0;
                    //    mesh.animNormals[vCounter + 2] = 0;
                    //}

                    // Iterates over 4 bones per vertex
                    for (int j = 0; j < 4; j++, boneCounter++)
                    {
                        boneWeight = mesh.boneWeights[boneCounter];
                        boneId = mesh.boneIds[boneCounter];

                        // Early stop when no transformation will be applied
                        if (boneWeight == 0.0f) continue;
                        animVertex = { mesh.vertices[vCounter], mesh.vertices[vCounter + 1], mesh.vertices[vCounter + 2] };
                        Vector3TransformRef(animVertex, model.meshes[m].boneMatrices[boneId]);
                        
                        mesh.animVertices[vCounter] += animVertex.x * boneWeight;
                        mesh.animVertices[vCounter + 1] += animVertex.y * boneWeight;
                        mesh.animVertices[vCounter + 2] += animVertex.z * boneWeight;
                        updated = true;

                        // Normals processing
                        // NOTE: We use meshes.baseNormals (default normal) to calculate meshes.normals (animated normals)
                        //if ((mesh.normals != NULL) && (mesh.animNormals != NULL))
                        //{
                        //    curMatrix = model.meshes[m].boneMatrices[boneId];
                        //    MatrixInvertRef(curMatrix);
                        //    MatrixTransposeRef(curMatrix);
                        //    animNormal = { mesh.normals[vCounter], mesh.normals[vCounter + 1], mesh.normals[vCounter + 2] };
                        //    Vector3TransformRef(animNormal, curMatrix);
                        //    mesh.animNormals[vCounter] += animNormal.x * boneWeight;
                        //    mesh.animNormals[vCounter + 1] += animNormal.y * boneWeight;
                        //    mesh.animNormals[vCounter + 2] += animNormal.z * boneWeight;
                        //}
                    }
                }

                if (updated)
                {
                    //rlUpdateVertexBuffer(mesh.vboId[0], mesh.animVertices, mesh.vertexCount * 3 * sizeof(float), 0); // Update vertex position
                    //if (mesh.normals != NULL) rlUpdateVertexBuffer(mesh.vboId[2], mesh.animNormals, mesh.vertexCount * 3 * sizeof(float), 0); // Update vertex normals
                }
            }
        }

    };

}