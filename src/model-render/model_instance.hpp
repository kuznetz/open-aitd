#pragma once
#include "./model.hpp"

namespace openAITD {

    using namespace ::std;
    using namespace ::raylib;

    class ModelInstance {
    public:
        const Model* modelData = nullptr;
        Transform* fromPose = nullptr;
        Transform* toPose = nullptr;
        Transform* curPose = nullptr;

        raylib::Mesh* curMeshes;
        
        int animId = 0;
        int frameId = 0;
        float animTime = 0.0f;
        float frameTime = 0.0f;
        bool isPlaying = true;

        ModelInstance() = default;

        ~ModelInstance() {
            if (curPose) delete[] curPose;
        }

        void Init(const Model& mData) {
            modelData = &mData;
            curMeshes = new raylib::Mesh[modelData->model.meshCount];
            for (int i = 0; i < modelData->model.meshCount; i++)
            {
                curMeshes[i].vertices = new float[modelData->model.meshes[i].vertexCount * 3];
                curMeshes[i].boneMatrices = new Matrix[modelData->model.meshes[i].boneCount];
            }

            if (modelData->skin) {
                if (curPose) delete[] curPose;
                curPose = new Transform[modelData->skin->joints_count];
                for (int i = 0; i < modelData->skin->joints_count; ++i) {
                    curPose[i] = { {0,0,0}, {0,0,0,1}, {1,1,1} };
                }
            }
        }

        void Process(float dt) {
            if (!modelData || !isPlaying || modelData->animations.empty()) return;
            
            const auto& anim = modelData->animations[animId];
            animTime += dt;
            //Animation ended
            if (animTime >= anim.duration) {
                animTime = fmod(animTime, anim.duration);
            }
            ApplyPoseForTime(animTime);
        }

        void SetAnimation(int id) {
            if (!modelData || id < 0 || id >= (int)modelData->animations.size()) return;
            animId = id;
            animTime = 0.0f;
            frameId = 0;
            frameTime = 0.0f;
            ApplyPoseForTime(0.0f);
            //rootMotion = {0,0,0};
        }

        void ApplyPoseForTime(float time) {
            if (!modelData || !curPose) return;
            modelData->CalcPoseByTime(curPose, animId, time);
            ApplyInternalPose(curPose);
        }

        void LerpPose(const Transform* poseFrom, const Transform* poseTo, float t) {
            if (!modelData || !curPose) return;
            modelData->PoseLerp(curPose, poseFrom, poseTo, t);
            ApplyInternalPose(curPose);
        }

        void Render() const {
            if (!modelData || !modelData->data) return;
            for (int i = 0; i < modelData->model.meshCount; i++)
            {
                DrawMesh(modelData->model.meshes[i], modelData->model.materials[modelData->model.meshMaterial[i]], modelData->model.transform);
            }
        }        

    private:
        void ApplyInternalPose(Transform* pose) {
            if (!modelData) return;
            ApplyParentJoints(pose);
            UpdateBones(pose);
            UpdateSkin();
            UpdateBuffer();
        }

        void ApplyParentJoints(Transform* transforms)
        {
            if (!modelData || !modelData->skin) return;
            for (int i = 0; i < modelData->skin->joints_count; i++)
            {
                if (modelData->bones[i].parent >= 0)
                {
                    if (modelData->bones[i].parent > i)
                    {
                        TRACELOG(LOG_WARNING, "Assumes bones are toplogically sorted, but bone %d has parent %d. Skipping.", i, modelData->bones[i].parent);
                        continue;
                    }
                    transforms[i].rotation = QuaternionMultiply(transforms[modelData->bones[i].parent].rotation, transforms[i].rotation);
                    transforms[i].translation = Vector3RotateByQuaternion(transforms[i].translation, transforms[modelData->bones[i].parent].rotation);
                    transforms[i].translation = Vector3Add(transforms[i].translation, transforms[modelData->bones[i].parent].translation);
                    transforms[i].scale = Vector3Multiply(transforms[i].scale, transforms[modelData->bones[i].parent].scale);
                }
            }
        }

        void UpdateBuffer()
        {
            if (!modelData) return;
            for (int m = 0; m < modelData->model.meshCount; m++)
            {
                auto& mesh = modelData->model.meshes[m];
                rlUpdateVertexBuffer(mesh.vboId[0], mesh.animVertices, mesh.vertexCount * 3 * sizeof(float), 0);
                if (mesh.normals != NULL) rlUpdateVertexBuffer(mesh.vboId[2], mesh.animNormals, mesh.vertexCount * 3 * sizeof(float), 0);
            }
        }

        void UpdateBones(Transform* pose)
        {
            const raylib::Model& model = this->modelData->model;
            int firstMeshWithBones = -1;
            for (int i = 0; i < model.meshCount; i++)
            {
                if (model.meshes[i].boneMatrices)
                {
                    firstMeshWithBones = i;
                    break;
                }
            }

            if (firstMeshWithBones == -1) return;

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
                    Vector3Multiply(outScale, invTranslation), outRotation
                ), outTranslation);
                Quaternion boneRotation = QuaternionMultiply(outRotation, invRotation);
                Vector3 boneScale = Vector3Multiply(outScale, invScale);

                Matrix boneMatrix = MatrixMultiply(MatrixMultiply(
                        QuaternionToMatrix(boneRotation),
                        MatrixTranslate(boneTranslation.x, boneTranslation.y, boneTranslation.z)
                    ),
                    MatrixScale(boneScale.x, boneScale.y, boneScale.z)
                );

                model.meshes[firstMeshWithBones].boneMatrices[boneId] = boneMatrix;
            }

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

        void UpdateSkin()
        {
            const raylib::Model& model = this->modelData->model;
            for (int m = 0; m < model.meshCount; m++)
            {
                Mesh& mesh = model.meshes[m];
                Vector3 animVertex = { 0 };
                int boneId = 0;
                int boneCounter = 0;
                float boneWeight = 0.0;
                bool updated = false;
                const int vValues = mesh.vertexCount * 3;

                if ((mesh.boneWeights == NULL) || (mesh.boneIds == NULL)) continue;

                for (int vCounter = 0; vCounter < vValues; vCounter += 3)
                {
                    mesh.animVertices[vCounter] = 0;
                    mesh.animVertices[vCounter + 1] = 0;
                    mesh.animVertices[vCounter + 2] = 0;

                    for (int j = 0; j < 4; j++, boneCounter++)
                    {
                        boneWeight = mesh.boneWeights[boneCounter];
                        boneId = mesh.boneIds[boneCounter];

                        if (boneWeight == 0.0f) continue;
                        
                        animVertex = { mesh.vertices[vCounter], mesh.vertices[vCounter + 1], mesh.vertices[vCounter + 2] };
                        Vector3TransformRef(animVertex, model.meshes[m].boneMatrices[boneId]);
                        
                        mesh.animVertices[vCounter] += animVertex.x * boneWeight;
                        mesh.animVertices[vCounter + 1] += animVertex.y * boneWeight;
                        mesh.animVertices[vCounter + 2] += animVertex.z * boneWeight;
                        updated = true;
                    }
                }
            }
        }
    };

}