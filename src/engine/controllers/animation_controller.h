#pragma once
#include "../resources/resources.h"
#include "../world/world.h"


using namespace std;

inline void BuildPoseFromParentJoints(BoneInfo* bones, int boneCount, Transform* transforms)
{
    for (int i = 0; i < boneCount; i++)
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

namespace openAITD {

	const float frameT = 1. / 60;

	class AnimationController {
	public:
		World* world;
		Resources* resources;
		
		AnimationController(Resources* res, World* world) {
			this->resources = res;
			this->world = world;
		}

        void UpdateModelAnimationSkin(Model& model)
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

		void process(float timeDelta) {
			for (int i = 0; i < this->world->gobjects.size(); i++) {
				auto& gobj = this->world->gobjects[i];
				if (gobj.location.stageId != this->world->curStageId) continue;
				if (gobj.model.id == -1) continue;
				if (gobj.model.animId == -1) continue;

				auto mdl = resources->models.getModel(gobj.model.id);
				auto anim = mdl->animations[gobj.model.animId];
				if (anim == 0) {
					printf("Miss animation %d in model %d", gobj.model.animId, gobj.model.id);
					gobj.model.animId = -1;
					continue;
				}
				gobj.model.animTime += timeDelta;
				gobj.model.animFrame = (gobj.model.animTime / frameT);
				auto& curFrame = gobj.model.animFrame;
				if (curFrame >= anim->frameCount) {
					gobj.model.animEnd = 1;
					if (!gobj.model.bitField.repeat) {
						gobj.model.animId = gobj.model.nextAnimId;
						gobj.model.animTime = 0;
						curFrame = 0;
						gobj.model.flags = 1;
					}
					else {
						while (curFrame >= anim->frameCount) {
							gobj.model.animTime -= anim->frameCount * frameT;
							curFrame -= anim->frameCount;
						}
                        gobj.physics.prevMoveRoot = { 0,0,0 };
                    }
				}
				if (gobj.model.animId != -1) {
                    if (gobj.model.prevAnimId != gobj.model.animId) {
                        gobj.physics.prevMoveRoot = { 0,0,0 };
                    }
                    gobj.physics.moveRoot = anim->framePoses[curFrame][0].translation;
                    UpdateModelAnimationBones(mdl->model, *anim, curFrame);
                    UpdateModelAnimationSkin(mdl->model);
				}

                gobj.model.prevAnimId = gobj.model.animId;
			}
		}
	};

}