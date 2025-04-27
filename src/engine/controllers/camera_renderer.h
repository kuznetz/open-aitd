#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../raylib.h"
#include "./common_renderers.h"

using namespace std;
namespace openAITD {

	Camera initCamera = {
		{ 0.0f, 0, -5 },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },   // mainCamera up vector (rotation towards target)
		60.,
		CAMERA_PERSPECTIVE
	};

	struct RenderBackground {
		Image background;
		vector<vector<Image>> masks;
	};

	struct RenderOverlay {
		//int x,y,w,h;
		Texture2D texture;
		int roomId;
		GCameraOverlay* res;
		//Render overlay after this object
		GameObject* renderAfterObj = 0;
	};

	class CameraRenderer {
	public:
		World* world;
		Resources* resources;

		int invX = 0;
		int invZ = 0;

		vector<RenderBackground> backgrounds;
		Texture2D backgroundTex = {0,0,0};
		vector<RenderOverlay> curOverlays;

		RenderOrder renderQueue[100];
		RenderOrder* renderStart;
		RenderOrder* renderIter;
		RenderOrder* renderIterPrev;
		int renderQueueCount = 0;

		Transform tempPose[100];

		Stage* curStage = 0;
		int curStageId = -1;
		WCamera* curCamera = 0;
		int curCameraId = -1;

		const int getScreenW() { return resources->config.screenW; }
		const int getScreenH() { return resources->config.screenH; }

		CameraRenderer(Resources* res, World* world) {
			this->resources = res;
			this->world = world;
		}

		~CameraRenderer() {
			clearCamera();
			clearStage();
		}

		Vector3 GetWorldToScreenZ(Vector3 position) {
			const int& width = getScreenW();
			const int& height = getScreenH();
			auto& matProj = curCamera->perspective;
			Matrix matView = MatrixLookAt(initCamera.position, initCamera.target, initCamera.up);
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
			Vector3& roomPos = curStage->rooms[gobj.location.roomId].position;
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

		void clearStage()
		{
			for (int i = 0; i < backgrounds.size(); i++) {
				auto& bg = backgrounds[i];
				if (bg.background.data) UnloadImage(bg.background);
				for (int i = 0; i < bg.masks.size(); i++) {
					for (int j = 0; j < bg.masks[i].size(); j++) {
						UnloadImage(bg.masks[i][j]);
					}
				}
			}
			backgrounds.clear();
		}

		void loadStage(int newStageId)
		{
			if (newStageId == curStageId) return;
			clearStage();
			curStage = &this->resources->stages[newStageId];

			for (int camId = 0; camId < curStage->cameras.size(); camId++ ) {
				auto dir = curStage->stageDir + "/camera_" + to_string(camId);
				auto& bg = backgrounds.emplace_back();
				bg.background = LoadImage((dir + "/background.png").c_str());
				auto& camRooms = curStage->cameras[camId].rooms;
				for (int cr = 0; cr < camRooms.size(); cr++) {
					auto& msk = bg.masks.emplace_back();
					for (int i = 0; i < camRooms[cr].overlays.size(); i++) {
						auto& msk2 = msk.emplace_back();
						msk2 = LoadImage((dir + "/mask_" + to_string(cr) + "_" + to_string(i) + ".png").c_str());
						//auto& o = camRooms[cr].overlays[camId];
					}
				}
			}
			curStageId = newStageId;
			curCameraId = -1;
		}

		void clearCamera()
		{
			if (curCameraId == -1) return;
			UnloadTexture(backgroundTex);
			for (int i = 0; i < curOverlays.size(); i++) {
				UnloadTexture(curOverlays[i].texture);
			}
			curOverlays.clear();
			curCameraId = -1;
		}

		Image resizeImg(Image& src, int w, int h) {
			Image res = { 0, w, h, 1, src.format };
			stbir_pixel_layout l = STBIR_RGB;
			if (src.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) l = STBIR_RGBA;
			if (src.format == RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA) l = STBIR_RA;
			res.data = stbir_resize_uint8_linear((unsigned char*)src.data, src.width, src.height, 0, 0, w, h, 0, l);
			return res;
		}		

		Texture2D generateMask(Image& fullBg, Image& mask) {
			Image maskImageScaled = resizeImg(mask, getScreenW(), getScreenH());
			Image resImg = { 0, getScreenW(), getScreenH(), 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
			resImg.data = new uint8_t[getScreenW() * getScreenH() * 4];
			for (int i = 0; i < (getScreenW() * getScreenH()); i++) {
				((uint8_t*)resImg.data)[i * 4 + 0] = ((uint8_t*)fullBg.data)[i * 3 + 0];
				((uint8_t*)resImg.data)[i * 4 + 1] = ((uint8_t*)fullBg.data)[i * 3 + 1];
				((uint8_t*)resImg.data)[i * 4 + 2] = ((uint8_t*)fullBg.data)[i * 3 + 2];
				((uint8_t*)resImg.data)[i * 4 + 3] = ((uint8_t*)maskImageScaled.data)[i * 2 + 1];
			}
			Texture2D result = LoadTextureFromImage(resImg);
			UnloadImage(maskImageScaled);
			UnloadImage(resImg);
			return result;
		}

		void loadCamera(int newStageId, int newCameraId)
		{
			if ((newStageId == curStageId) && (newCameraId == curCameraId)) return;
			clearCamera();
			loadStage(newStageId);
			curCamera = &curStage->cameras[newCameraId];
			curCameraId = newCameraId;

			auto& fullBg = resizeImg(backgrounds[curCameraId].background, getScreenW(), getScreenH());
			backgroundTex = LoadTextureFromImage(fullBg);
			auto& masks = backgrounds[curCameraId].masks;			
			for (int i = 0; i < masks.size(); i++) {
				auto& m2 = masks[i];
				for (int j = 0; j < m2.size(); j++) {
					auto& ovl = curOverlays.emplace_back();
					ovl.roomId = curCamera->rooms[i].roomId;
					ovl.res = &curCamera->rooms[i].overlays[j];
					ovl.texture = generateMask(fullBg, m2[j]);
				}
			}
			UnloadImage(fullBg);

			auto& camPers = curCamera->pers;
			curCamera->perspective = MatrixPerspective(camPers.yfov, camPers.aspectRatio, camPers.znear, camPers.zfar);
			//It's ugly, but it didn't get any better.
			initCamera.position = curCamera->position;
			Matrix vw = QuaternionToMatrix(curCamera->rotation);
			initCamera.target = Vector3Add(curCamera->position, Vector3Negate({ vw.m8, vw.m9, vw.m10 }));
			//initCamera.target = { 0, 0, 0 };
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
			Vector3& roomPos = curStage->rooms[gobj.location.roomId].position;
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

		void process() {
			if (world->curStageId == -1 || world->curCameraId == -1) return;
			if (world->curStageId != curStageId || world->curCameraId != curCameraId) {
				loadCamera(world->curStageId, world->curCameraId);
			}

			ClearBackground(BLACK);

			DrawTexturePro(
				backgroundTex,
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
				Vector3& roomPos = curStage->rooms[gobj.location.roomId].position;
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

			for (auto i = 0; i < curOverlays.size(); i++) {
				curOverlays[i].renderAfterObj = 0;
			}

			//TODO: Revese for can be faster (no overwrite result)
			//for (auto it = renderQueue.begin(); it != renderQueue.end(); it++) {
			//	auto& obj = *it->obj;
			//	for (auto i = 0; i < curOverlays.size(); i++) {
			//		auto& ovl = curOverlays[i];					
			//		if ((ovl.roomId == it->obj->location.roomId) && checkOverlay(*ovl.res, obj)) {
			//			ovl.renderAfterObj = &obj;
			//		}
			//	}
			//}

			if (renderStart) {
				int num = 1;
				renderIter = renderStart;
				while (true) {
					BeginMode3D(initCamera);
					//rlSetMatrixModelview(curCamera->modelview);
					rlSetMatrixProjection(curCamera->perspective);
					renderObject(*renderIter->gobj, WHITE);
					EndMode3D();

					//auto s = to_string(num)+" R" + to_string(it->obj->location.roomId);
					//auto s = to_string(it->obj->);

					Rectangle r = {
						renderIter->screenMin.x,
						renderIter->screenMin.y,
						(renderIter->screenMax.x - renderIter->screenMin.x) + 1,
						(renderIter->screenMax.y - renderIter->screenMin.y) + 1
					};

					for (auto i = 0; i < curOverlays.size(); i++) {
						if (renderIter->gobj->location.roomId != curOverlays[i].roomId) continue;
						if (checkOverlay(*curOverlays[i].res, *renderIter->gobj)) {
							DrawTexturePro(
								curOverlays[i].texture, r, r, { 0, 0 }, 0, WHITE
							);
						}
					}
					//it->marker = s;
					num++;
					renderIter = renderIter->next;
					if (!renderIter) break;
				}
			}


			if (world->messageTime > 0) {
				auto& f = resources->mainFont;
				const char* m = world->messageText.c_str();
				auto mt = MeasureTextEx(f, m, f.baseSize, 0);
				Vector2 v = { (int)(getScreenW() - mt.x) / 2, getScreenH() - (f.baseSize * 2) };
				DrawTextEx(f, m, v, f.baseSize, 0, WHITE);
			}


			//for (auto it = renderQueue.begin(); it != renderQueue.end(); it++) {
			//	DrawLine(it->screenMin.x, it->screenMin.y, it->screenMax.x, it->screenMin.y, RED);
			//	DrawLine(it->screenMin.x, it->screenMin.y, it->screenMin.x, it->screenMax.y, RED);
			//	DrawLine(it->screenMax.x, it->screenMax.y, it->screenMax.x, it->screenMin.y, RED);
			//	DrawLine(it->screenMax.x, it->screenMax.y, it->screenMin.x, it->screenMax.y, RED);
			//}

		}

	};

}