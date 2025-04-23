#pragma once
#include <vector>
#include <string>
#include <cstring>
#include "../raylib.h"
#include <external/cgltf.h>

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
        vector <vector<Transform>> bakedPoses;
        float transition;
        float duration;
    };

    struct Bone {
        int parent;
    };

    class Model {
    public:
        raylib::Model model;
        cgltf_data* data = 0;
        cgltf_skin* skin = 0;
        vector<Bone> bones;
        vector<Animation> animations;
        Transform* curPose = 0;
        BoundingBox bounds;
        
        ~Model() {
            if (!data) return;
            UnloadModel(model);
            if (curPose) delete[] curPose;
            cgltf_free(data);
        }

        void load(const char* fileName) {
            data = LoadData(fileName);
            model = raylib::LoadModel(fileName);
            bounds = GetModelBoundingBox(model);
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

        void bakePoses(int fps) {
            for (int i = 0; i < animations.size(); i++) {
                auto& anim = animations[i];
                float t = 0;
                int frameCount = ceil(anim.duration * fps) + 1;
                anim.bakedPoses.resize(frameCount);
                for (int j = 0; j < frameCount; j++) {
                    anim.bakedPoses[j].resize(bones.size());
                    CalcPoseByTime(anim.bakedPoses[j].data(), i, t);
                    t = (float)j / fps;
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
            bounds = GetModelBoundingBox(model);
        }

        void Render() const {
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
                cgltf_accessor_read_float(inp, 1, &animations[i].transition, 1);

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
                Mesh mesh = model.meshes[m];
                Vector3 animVertex = { 0 };
                Vector3 animNormal = { 0 };
                int boneId = 0;
                int boneCounter = 0;
                float boneWeight = 0.0;
                bool updated = false; // Flag to check when anim vertex information is updated
                const int vValues = mesh.vertexCount * 3;

                // Skip if missing bone data, causes segfault without on some models
                if ((mesh.boneWeights == NULL) || (mesh.boneIds == NULL)) continue;

                for (int vCounter = 0; vCounter < vValues; vCounter += 3)
                {
                    mesh.animVertices[vCounter] = 0;
                    mesh.animVertices[vCounter + 1] = 0;
                    mesh.animVertices[vCounter + 2] = 0;
                    if (mesh.animNormals != NULL)
                    {
                        mesh.animNormals[vCounter] = 0;
                        mesh.animNormals[vCounter + 1] = 0;
                        mesh.animNormals[vCounter + 2] = 0;
                    }

                    // Iterates over 4 bones per vertex
                    for (int j = 0; j < 4; j++, boneCounter++)
                    {
                        boneWeight = mesh.boneWeights[boneCounter];
                        boneId = mesh.boneIds[boneCounter];

                        // Early stop when no transformation will be applied
                        if (boneWeight == 0.0f) continue;
                        animVertex = { mesh.vertices[vCounter], mesh.vertices[vCounter + 1], mesh.vertices[vCounter + 2] };
                        animVertex = Vector3Transform(animVertex, model.meshes[m].boneMatrices[boneId]);
                        mesh.animVertices[vCounter] += animVertex.x * boneWeight;
                        mesh.animVertices[vCounter + 1] += animVertex.y * boneWeight;
                        mesh.animVertices[vCounter + 2] += animVertex.z * boneWeight;
                        updated = true;

                        // Normals processing
                        // NOTE: We use meshes.baseNormals (default normal) to calculate meshes.normals (animated normals)
                        if ((mesh.normals != NULL) && (mesh.animNormals != NULL))
                        {
                            animNormal = { mesh.normals[vCounter], mesh.normals[vCounter + 1], mesh.normals[vCounter + 2] };
                            animNormal = Vector3Transform(animNormal, MatrixTranspose(MatrixInvert(model.meshes[m].boneMatrices[boneId])));
                            mesh.animNormals[vCounter] += animNormal.x * boneWeight;
                            mesh.animNormals[vCounter + 1] += animNormal.y * boneWeight;
                            mesh.animNormals[vCounter + 2] += animNormal.z * boneWeight;
                        }
                    }
                }

                if (updated)
                {
                    rlUpdateVertexBuffer(mesh.vboId[0], mesh.animVertices, mesh.vertexCount * 3 * sizeof(float), 0); // Update vertex position
                    if (mesh.normals != NULL) rlUpdateVertexBuffer(mesh.vboId[2], mesh.animNormals, mesh.vertexCount * 3 * sizeof(float), 0); // Update vertex normals
                }
            }
        }

    };

}