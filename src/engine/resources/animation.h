#pragma once
#include <vector>
#include <string>
//#include <tiny_gltf.h>
#include "../raylib.h"

using namespace raylib;
using namespace std;

namespace GLTFAnimations {
    #include <external/cgltf.h>

    struct BoneChannels {
        cgltf_animation_channel* translate;
        cgltf_animation_channel* rotate;
        cgltf_animation_channel* scale;
        cgltf_interpolation_type interpolationType;
    };

    class Animations {
    public:
        cgltf_data* data;
        int* parents;
        BoneChannels** boneChannels;
        
        ~Animations() {
            //TODO: destructor
        }

        void load(const char* fileName) {
            data = LoadData(fileName);
            parents = makeBoneParents(data);
            boneChannels = makeBoneChannels(data);
        }

        void applyPose(Model& model, int animationId, float time) {
            Transform* pose = GetTimePose(data, boneChannels[animationId], parents, time);
            UpdateModelAnimationBones(model, pose);
            delete[] pose;
        }

    private:

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

        static int* makeBoneParents(cgltf_data* data)
        {
            const cgltf_skin& skin = data->skins[0];
            int* bones = new int[(int)skin.joints_count];
            for (unsigned int i = 0; i < skin.joints_count; i++)
            {
                cgltf_node& node = *skin.joints[i];
                // Find parent bone index
                int parentIndex = -1;
                for (unsigned int j = 0; j < skin.joints_count; j++)
                {
                    if (skin.joints[j] == node.parent)
                    {
                        parentIndex = (int)j;
                        break;
                    }
                }
                bones[i] = parentIndex;
            }
            return bones;
        }

        static BoneChannels** makeBoneChannels(cgltf_data* data)
        {
            if (data->skins_count == 0) {
                TRACELOG(LOG_WARNING, "MODEL: [%s] No skins", fileName);
                return 0;
            }

            if (data->skins_count > 1)
            {
                TRACELOG(LOG_WARNING, "MODEL: [%s] expected exactly one skin to load animation data from, but found %i", fileName, data->skins_count);
            }

            const cgltf_skin& skin = data->skins[0];
            BoneChannels** result = new BoneChannels * [data->animations_count];
            for (unsigned int i = 0; i < data->animations_count; i++)
            {
                cgltf_animation& animData = data->animations[i];

                BoneChannels* boneChannels = new BoneChannels[(int)skin.joints_count];
                result[i] = boneChannels;

                for (unsigned int j = 0; j < animData.channels_count; j++)
                {
                    cgltf_animation_channel channel = animData.channels[j];
                    int boneIndex = -1;

                    for (unsigned int k = 0; k < skin.joints_count; k++)
                    {
                        if (animData.channels[j].target_node == skin.joints[k])
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

                }

                //TRACELOG(LOG_INFO, "MODEL: [%s] Loaded animation: %s (%d frames, %fs)", fileName, (animData.name != NULL) ? animData.name : "NULL", animations[i].frameCount, animDuration);
                //RL_FREE(boneChannels);
            }

            //cgltf_free(data);
            return result;
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

        static void BuildPoseFromParentJoints(Transform* transforms, cgltf_data* data, int* boneParent)
        {
            const cgltf_skin& skin = data->skins[0];
            for (int i = 0; i < skin.joints_count; i++)
            {
                if (boneParent[i] >= 0)
                {
                    if (boneParent[i] > i)
                    {
                        TRACELOG(LOG_WARNING, "Assumes bones are toplogically sorted, but bone %d has parent %d. Skipping.", i, boneParent[i]);
                        continue;
                    }
                    transforms[i].rotation = QuaternionMultiply(transforms[boneParent[i]].rotation, transforms[i].rotation);
                    transforms[i].translation = Vector3RotateByQuaternion(transforms[i].translation, transforms[boneParent[i]].rotation);
                    transforms[i].translation = Vector3Add(transforms[i].translation, transforms[boneParent[i]].translation);
                    transforms[i].scale = Vector3Multiply(transforms[i].scale, transforms[boneParent[i]].scale);
                }
            }
        }

        static Transform* GetTimePose(cgltf_data* data, BoneChannels* boneChannels, int* parents, float time) {
            const cgltf_skin& skin = data->skins[0];

            Transform* framePoses = new Transform[skin.joints_count];
            for (int k = 0; k < skin.joints_count; k++)
            {
                Vector3 translation = { skin.joints[k]->translation[0], skin.joints[k]->translation[1], skin.joints[k]->translation[2] };
                Quaternion rotation = { skin.joints[k]->rotation[0], skin.joints[k]->rotation[1], skin.joints[k]->rotation[2], skin.joints[k]->rotation[3] };
                Vector3 scale = { skin.joints[k]->scale[0], skin.joints[k]->scale[1], skin.joints[k]->scale[2] };

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

                framePoses[k] = {
                    translation,
                    rotation,
                    scale
                };
            }

            return framePoses;
            BuildPoseFromParentJoints(framePoses, data, parents);
        }

        static void UpdateModelAnimationBones(Model& model, Transform* pose)
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

    };

}