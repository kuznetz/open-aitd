#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../raylib.h"
#include "./common_renderers.h"

using namespace std;
namespace openAITD {

	class CameraRenderer {
	public:
		World* world;
		Resources* resources;

		Camera mainCamera = {
			{ 0.0f, 0, -5 },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },   // mainCamera up vector (rotation towards target)
			60.,
			CAMERA_PERSPECTIVE
		};

		RenderOrder renderQueue[100];
		RenderOrder* renderStart;
		RenderOrder* renderIter;
		RenderOrder* renderIterPrev;
		int renderQueueCount = 0;

		Transform tempPose[100];

		int curStageId = -1;
		int curCameraId = -1;
		WCamera* curCamera = 0;
		Background* curBackground = 0;

		const int getScreenW() { return resources->config.screenW; }
		const int getScreenH() { return resources->config.screenH; }

		CameraRenderer(Resources* res, World* world) {
			this->resources = res;
			this->world = world;
		}

		~CameraRenderer() {
			clearCamera();
		}

		Vector3 GetWorldToScreenZ(Vector3 position) {
			const int& width = getScreenW();
			const int& height = getScreenH();
			auto& matProj = curCamera->perspective;
			Matrix matView = MatrixLookAt(mainCamera.position, mainCamera.target, mainCamera.up);
			Vector3 depth = Vector3Transform(position, matView);
			Quaternion worldPos = { position.x, position.y, position.z, 1.0f };
			worldPos = QuaternionTransform(worldPos, matView);
			worldPos = QuaternionTransform(worldPos, matProj);
			Vector3 ndcPos = { worldPos.x / worldPos.w, -worldPos.y / worldPos.w, worldPos.z / worldPos.w };
			Vector3 screenPosition = { (ndcPos.x + 1.0f) / 2.0f * (float)width, (ndcPos.y + 1.0f) / 2.0f * (float)height, -depth.z };
			return screenPosition;
		}

		void fillScreenBounds(RenderOrder& ord, GameObject& gobj)
		{
			ord.next = 0;
			ord.gobj = &gobj;

			//Calc matrix
			Vector3& roomPos = world->curStage->rooms[gobj.location.roomId].position;
			auto pos = Vector3Add(roomPos, gobj.location.position);
			Matrix matTranslation = MatrixTranslate(pos.x, pos.y, pos.z);
			auto& rot = gobj.location.rotation;
			Matrix matRotation = QuaternionToMatrix(rot);
			Matrix matScale = MatrixScale(1, 1, 1);
			Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

			auto rmodel = resources->models.getModel(gobj.modelId);
			auto& bb = rmodel->model.bounds;
			Vector3 vecs[8];
			vecs[0] = { bb.min.x, bb.max.y, bb.max.z }; // Top left
			vecs[1] = { bb.max.x, bb.max.y, bb.max.z }; // Top right
			vecs[2] = { bb.min.x, bb.min.y, bb.max.z }; // Bottom left
			vecs[3] = { bb.max.x, bb.min.y, bb.max.z }; // Bottom right
			// Back face
			vecs[4] = { bb.min.x, bb.max.y, bb.min.z }; // Top left
			vecs[5] = { bb.max.x, bb.max.y, bb.min.z }; // Top right
			vecs[6] = { bb.min.x, bb.min.y, bb.min.z }; // Bottom left
			vecs[7] = { bb.max.x, bb.min.y, bb.min.z }; // Bottom right

			for (int i = 0; i < 8; i++) {
				Vector3 v = GetWorldToScreenZ(Vector3Transform(vecs[i], matTransform));
				if (i == 0 || ord.zPos < v.z) {
					ord.zPos = v.z;
				}
				if (i == 0 || ord.screenMin.x > v.x) {
					ord.screenMin.x = v.x;
				}
				if (i == 0 || ord.screenMin.y > v.y) {
					ord.screenMin.y = v.y;
				}
				if (i == 0 || ord.screenMax.x < v.x) {
					ord.screenMax.x = v.x;
				}
				if (i == 0 || ord.screenMax.y < v.y) {
					ord.screenMax.y = v.y;
				}
			}
		}

		void clearCamera()
		{
			if (curCameraId == -1) return;
			curCameraId = -1;
			curCamera = 0;
		}

		void loadCamera(int newCameraId)
		{
			printf("Load Camera %d\n", newCameraId);
			curCameraId = newCameraId;
			curCamera = &resources->stages[world->curStageId].cameras[newCameraId];
			curBackground = resources->backgrounds.get(world->curStageId, newCameraId);

			auto& camPers = curCamera->pers;
			curCamera->perspective = MatrixPerspective(camPers.yfov, camPers.aspectRatio, camPers.znear, camPers.zfar);
			
			mainCamera.position = curCamera->position;
			mainCamera.target = Vector3Add(curCamera->position, Vector3Negate(Vector3RotateByQuaternion({ 0,0,1 }, curCamera->rotation)));
			mainCamera.up = Vector3RotateByQuaternion({ 0,1,0 }, curCamera->rotation);
		}

		void renderObject(GameObject& gobj, Color tint)
		{
			auto rmodel = resources->models.getModel(gobj.modelId);
			auto& model = rmodel->model;

			if (model.skin && gobj.animation.id != -1) {
				auto& curAnim = model.animations[gobj.animation.animIdx];
				auto& newPose = curAnim.bakedPoses[gobj.animation.animFrame];
				Transform* curPose;
				bool isTransition = gobj.animation.oldPose.size() && (curAnim.duration > 0) && (gobj.animation.animTime <= curAnim.transition);
				if (isTransition) {
					//newPose[0].translation = { 0,0,0 };
					model.PoseLerp(tempPose, gobj.animation.oldPose.data(), newPose.data(), gobj.animation.animTime / curAnim.transition);
					//anim2.CalcPoseByTime(newPose, animIndex, 0);
					curPose = tempPose;
				}
				else {
					curPose = newPose.data();
					//curPose[0].translation = { 0,0,0 };
					//anim2.CalcPoseByTime(curPose, animIndex, animTime);
				}
				model.ApplyPose(curPose);
				if (!gobj.animation.oldPose.size()) {
					gobj.animation.oldPose.resize(newPose.size());
				}
				if (!isTransition) {
					memcpy_s(
						gobj.animation.oldPose.data(), gobj.animation.oldPose.size() * sizeof(Transform),
						curPose, newPose.size() * sizeof(Transform)
					);
				}
			}

			Vector3 pos = gobj.location.position;
			Vector3& roomPos = world->curStage->rooms[gobj.location.roomId].position;
			pos = Vector3Add(roomPos, pos);
			Matrix matTranslation = MatrixTranslate(pos.x, pos.y, pos.z);
			auto& rot = gobj.location.rotation;
			Matrix matRotation = QuaternionToMatrix(rot);
			Matrix matTransform = MatrixMultiply(matRotation, matTranslation);

			// Combine model transformation matrix (model.transform) with matrix generated by function parameters (matTransform)
			auto m = rlGetMatrixModelview();
			rlSetMatrixModelview(MatrixMultiply(matTransform, m));
			model.Render();
			rlSetMatrixModelview(m);
		}

		bool checkOverlay(GCameraOverlay& ovl, GameObject& gobj) {
			Vector3 pos = gobj.location.position;
			for (int i = 0; i < ovl.bounds.size(); i++) {
				auto& b = ovl.bounds[i];
				//if (CheckCollisionBoxes(objBnd, ovl.bounds[i])) {
				if (pos.x >= b.min.x && pos.x <= b.max.x && pos.z >= b.min.z && pos.z <= b.max.z) {
					return true;
				}
			}
			return false;
		}

		void renderMessage() {
			if (world->messageTime > 0) {
				auto& f = resources->mainFont;
				const char* m = world->messageText.c_str();
				auto mt = MeasureTextEx(f, m, f.baseSize, 0);
				Vector2 v = { (int)(getScreenW() - mt.x) / 2, getScreenH() - (f.baseSize * 2) };
				DrawTextEx(f, m, v, f.baseSize, 0, WHITE);
			}
		}

		void render() {
			if (world->inDark) {
				renderMessage();
				return;
			}

			if (world->curStageId == -1 || world->curCameraId == -1) return;
			if (world->curStageId != curStageId || world->curCameraId != curCameraId) {
				curStageId = world->curStageId;
				loadCamera(world->curCameraId);
			}

			DrawTexturePro(
				curBackground->texture,
				{ 0, 0, (float)getScreenW(), (float)getScreenH() },
				{ 0, 0, (float)getScreenW(), (float)getScreenH() },
				{ 0, 0 }, 0, WHITE
			);

			/*
			for (int i = 0; i < this->world->gobjects.size(); i++) {
				auto& gobj = this->world->gobjects[i];
				if (gobj.model.id == -1) continue;
				if (gobj.location.stageId != curStageId) continue;
				renderObject(gobj, WHITE);
			}
			*/

			renderQueueCount = 0;
			renderStart = 0;

			for (int i = 0; i < this->world->gobjects.size(); i++) {
				auto& gobj = this->world->gobjects[i];
				if (gobj.modelId == -1) continue;
				if (gobj.location.stageId != curStageId) continue;
				
				int curCamRoom = -1;
				for (int j = 0; j < curCamera->rooms.size(); j++) {
					if (gobj.location.roomId == curCamera->rooms[j].roomId) {
						curCamRoom = j;
						break;
					}
				}
				if (curCamRoom == -1) continue;

				Vector3 pos = gobj.location.position;
				Vector3& roomPos = world->curStage->rooms[gobj.location.roomId].position;
				pos = Vector3Add(roomPos, pos);

				//auto& screenPos = GetWorldToScreenZ(pos);
				//if (screenPos.z < 0) continue;

				RenderOrder& ro = renderQueue[renderQueueCount++];
				fillScreenBounds(ro, gobj);
				if (ro.zPos < 0) continue;
				if (ro.screenMax.x < 0 || ro.screenMin.x > getScreenW()) continue;
				if (ro.screenMax.y < 0 || ro.screenMin.y > getScreenH()) continue;

				renderObject(gobj, WHITE);
				
				//string s = to_string(i);
								
				if (renderStart) {
					bool inserted = false;
					renderIterPrev = 0;
					renderIter = renderStart;
					while (true) {
						if (renderIter->zPos < ro.zPos) {
							if (renderIterPrev) {
								renderIterPrev->next = &ro;
							}
							else {
								renderStart = &ro;
							}
							ro.next = renderIter;
							inserted = true;
							break;
						}
						if (!renderIter->next) break;
						renderIterPrev = renderIter;
						renderIter = renderIter->next;
					}
					if (!inserted) {
						renderIter->next = &ro;
					}
				}
				else {
					renderStart = &ro;
				}

			}

			if (renderStart) {
				int num = 1;
				renderIter = renderStart;
				while (true) {
					BeginMode3D(mainCamera);
					//rlSetMatrixModelview(curCamera->modelview);
					rlSetMatrixProjection(curCamera->perspective);
					renderObject(*renderIter->gobj, WHITE);
					EndMode3D();

					//auto s = to_string(num)+" R" + to_string(it->obj->location.roomId);
					//auto s = to_string(it->obj->);

					raylib::Rectangle r = {
						renderIter->screenMin.x,
						renderIter->screenMin.y,
						(renderIter->screenMax.x - renderIter->screenMin.x) + 1,
						(renderIter->screenMax.y - renderIter->screenMin.y) + 1
					};

					for (int camRoomIdx = 0; camRoomIdx < curCamera->rooms.size(); camRoomIdx++) {
						if (renderIter->gobj->location.roomId != curCamera->rooms[camRoomIdx].roomId) continue;
						for (int ovlIdx = 0; ovlIdx < curCamera->rooms[camRoomIdx].overlays.size(); ovlIdx++) {
							auto& ovl = curCamera->rooms[camRoomIdx].overlays[ovlIdx];
							if (checkOverlay(ovl, *renderIter->gobj)) {
								DrawTexturePro(
									curBackground->overlays[camRoomIdx][ovlIdx].texture, r, r, { 0, 0 }, 0, WHITE
								);
							}
						}
					}
					//it->marker = s;
					num++;
					renderIter = renderIter->next;
					if (!renderIter) break;
				}
			}

			renderMessage();

			//for (auto it = renderQueue.begin(); it != renderQueue.end(); it++) {
			//	DrawLine(it->screenMin.x, it->screenMin.y, it->screenMax.x, it->screenMin.y, RED);
			//	DrawLine(it->screenMin.x, it->screenMin.y, it->screenMin.x, it->screenMax.y, RED);
			//	DrawLine(it->screenMax.x, it->screenMax.y, it->screenMax.x, it->screenMin.y, RED);
			//	DrawLine(it->screenMax.x, it->screenMax.y, it->screenMin.x, it->screenMax.y, RED);
			//}

		}

	};

}