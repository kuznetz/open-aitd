﻿#pragma once
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
		int screenW = 0;
		int screenH = 0;

		int invX = 0;
		int invZ = 0;

		vector<RenderBackground> backgrounds;
		Texture2D backgroundTex = {0,0,0};
		vector<RenderOverlay> curOverlays;


		Stage* curStage = 0;
		int curStageId = -1;
		WCamera* curCamera = 0;
		int curCameraId = -1;

		CameraRenderer(Resources* res, World* world) {
			this->resources = res;
			this->world = world;
		}

		~CameraRenderer() {
			clearCamera();
			clearStage();
		}

		Vector3 GetWorldToScreenZ(Vector3 position) {
			int width = GetScreenWidth();
			int height = GetScreenHeight();
			auto& matProj = curCamera->perspective;
			Matrix matView = MatrixLookAt(initCamera.position, initCamera.target, initCamera.up);
			Quaternion worldPos = { position.x, position.y, position.z, 1.0f };
			worldPos = QuaternionTransform(worldPos, matView);
			worldPos = QuaternionTransform(worldPos, matProj);
			Vector3 ndcPos = { worldPos.x / worldPos.w, -worldPos.y / worldPos.w, worldPos.z / worldPos.w };
			Vector3 screenPosition = { (ndcPos.x + 1.0f) / 2.0f * (float)width, (ndcPos.y + 1.0f) / 2.0f * (float)height, ndcPos.z };
			return screenPosition;
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
			Texture2D tex = LoadTextureFromImage(src);
			RenderTexture2D imgTex = LoadRenderTexture(w, h);
			BeginTextureMode(imgTex);
			SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
			DrawTexturePro(
				tex,
				{ 0, 0, (float)tex.width, (float)tex.height },
				{ 0, 0, (float)w, (float)h },
				{ 0, 0 }, 0, WHITE
			);
			EndTextureMode();
			Image imageScaled = LoadImageFromTexture(imgTex.texture);
			ImageFlipVertical(&imageScaled);
			return imageScaled;
		}

		Texture2D generateMask(Image& fullBg, Image& mask) {
			Image maskImageScaled = resizeImg(mask, screenW, screenH);
			for (int i = 0; i < (screenW * screenH); i++) {
				((uint8_t*)maskImageScaled.data)[i * 4 + 3] = ((uint8_t*)maskImageScaled.data)[i * 4 + 0];
				((uint8_t*)maskImageScaled.data)[i * 4 + 0] = ((uint8_t*)fullBg.data)[i * 4 + 0];
				((uint8_t*)maskImageScaled.data)[i * 4 + 1] = ((uint8_t*)fullBg.data)[i * 4 + 1];
				((uint8_t*)maskImageScaled.data)[i * 4 + 2] = ((uint8_t*)fullBg.data)[i * 4 + 2];
			}
			Texture2D result = LoadTextureFromImage(maskImageScaled);
			UnloadImage(maskImageScaled);
			return result;
		}

		void loadCamera(int newStageId, int newCameraId)
		{
			if ((newStageId == curStageId) && (newCameraId == curCameraId)) return;
			clearCamera();
			loadStage(newStageId);
			curCamera = &curStage->cameras[newCameraId];
			curCameraId = newCameraId;

			auto& fullBg = resizeImg(backgrounds[curCameraId].background, screenW, screenH);
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
			Vector3 pos = gobj.location.position;
			Vector3& roomPos = curStage->rooms[gobj.location.roomId].position;
			pos = Vector3Add(roomPos, pos);
			Matrix matTranslation = MatrixTranslate(pos.x, pos.y, pos.z);

			auto& rot = gobj.location.rotation;
			Matrix matRotation = QuaternionToMatrix(rot);

			Matrix matScale = MatrixScale(1, 1, 1);

			Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);
			auto rmodel = resources->models.getModel(gobj.model.id);
			auto& model = rmodel->model;

			// Combine model transformation matrix (model.transform) with matrix generated by function parameters (matTransform)
			auto transform = MatrixMultiply(model.transform, matTransform);
			for (int i = 0; i < model.meshCount; i++)
			{
				Color color = model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color;

				Color colorTint = WHITE;
				colorTint.r = (unsigned char)(((int)color.r * (int)tint.r) / 255);
				colorTint.g = (unsigned char)(((int)color.g * (int)tint.g) / 255);
				colorTint.b = (unsigned char)(((int)color.b * (int)tint.b) / 255);
				colorTint.a = (unsigned char)(((int)color.a * (int)tint.a) / 255);

				model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = colorTint;
				DrawMesh(model.meshes[i], model.materials[model.meshMaterial[i]], transform);
				model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = color;
			}
		}

		bool checkOverlay(GCameraOverlay& ovl, GameObject& gobj) {
			auto* mdl = resources->models.getModel(gobj.model.id);
			auto objBnd = mdl->bounds;

			Vector3 pos = gobj.location.position;
			//Vector3& roomPos = curStage->rooms[gobj.location.roomId].position;
			//pos = Vector3Add(roomPos, pos);
			objBnd.min.x += pos.x;
			objBnd.max.x += pos.x;
			objBnd.min.z += pos.z;
			objBnd.max.z += pos.z;
			objBnd.min.y = -10000;
			objBnd.max.y = 10000;
			for (int i = 0; i < ovl.bounds.size(); i++) {
				if (CheckCollisionBoxes(objBnd, ovl.bounds[i])) {
					return true;
				}
			}
			return false;
		}

		void process() {
			if (world->curStageId != curStageId || world->curCameraId != curCameraId) {
				loadCamera(world->curStageId, world->curCameraId);
			}

			ClearBackground(BLACK);

			DrawTexturePro(
				backgroundTex,
				{ 0, 0, (float)screenW, (float)screenH },
				{ 0, 0, (float)screenW, (float)screenH },
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

			//TODO: Remove memory allocation
			list<RenderOrder> renderQueue;
			for (int i = 0; i < this->world->gobjects.size(); i++) {
				auto& gobj = this->world->gobjects[i];
				if (gobj.model.id == -1) continue;
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

				auto& screenPos = GetWorldToScreenZ(pos);
				if (screenPos.z < 0) continue;
				renderObject(gobj, WHITE);
				
				bool inserted = false;
				string s = to_string(i);
				for (auto it = renderQueue.begin(); it != renderQueue.end(); it++) {
					if ((*it).zPos.z < screenPos.z) {
						renderQueue.insert( it, { &gobj, screenPos, s });
						inserted = true;
						break;
					}
				}
				if (!inserted) {
					renderQueue.push_back({ &gobj, screenPos, s });
				}
			}

			for (auto i = 0; i < curOverlays.size(); i++) {
				curOverlays[i].renderAfterObj = 0;
			}

			//TODO: Revese for can be faster (no overwrite result)
			for (auto it = renderQueue.begin(); it != renderQueue.end(); it++) {
				auto& obj = *it->obj;
				for (auto i = 0; i < curOverlays.size(); i++) {
					/*if (obj.model.id == 24 && i == 6) {
						[]() {}();
					}*/
					auto& ovl = curOverlays[i];					
					if ((ovl.roomId == it->obj->location.roomId) && checkOverlay(*ovl.res, obj)) {
						ovl.renderAfterObj = &obj;
					}
				}
			}

			

			int num = 1;
			for (auto it = renderQueue.begin(); it != renderQueue.end(); it++) {
				BeginMode3D(initCamera);
					//rlSetMatrixModelview(curCamera->modelview);
					rlSetMatrixProjection(curCamera->perspective);
					renderObject(*it->obj, WHITE);
				EndMode3D();

				//auto s = to_string(num)+" R" + to_string(it->obj->location.roomId);
				//auto s = to_string(it->obj->);

				for (auto i = 0; i < curOverlays.size(); i++) {
					if (it->obj == curOverlays[i].renderAfterObj) {
						DrawTexturePro(
							curOverlays[i].texture,
							{ 0, 0, (float)screenW, (float)screenH },
							{ 0, 0, (float)screenW, (float)screenH },
							{ 0, 0 }, 0, WHITE
						);
						//s += string(" ") + to_string(i)+"."+to_string(curOverlays[i].roomId);
					}
				}
				//it->marker = s;
				num++;
			}

		}

	};

}