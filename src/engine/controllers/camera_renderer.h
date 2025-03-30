﻿#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../raylib.h"

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

	class CameraRenderer {
	public:
		World* world;
		Resources* resources;
		int screenW = 0;
		int screenH = 0;

		vector<RenderBackground> backgrounds;
		Texture2D backgroundTex = {0,0,0};
		vector<vector<Texture2D>> overlayTextures;

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
						msk2 = LoadImage((dir + "/mask_" + to_string(camRooms[cr].roomId) + "_" + to_string(i) + ".png").c_str());
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
			for (int i = 0; i < overlayTextures.size(); i++) {
				for (int j = 0; j < overlayTextures[i].size(); j++) {
					UnloadTexture(overlayTextures[i][j]);
				}
			}
			overlayTextures.clear();

			curCameraId = -1;
			vector<vector<int>> overlayTextures;
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
				((uint8_t*)maskImageScaled.data)[i * 4 + 3] = ((uint8_t*)fullBg.data)[i * 4 + 0];
				((uint8_t*)maskImageScaled.data)[i * 4 + 0] = ((uint8_t*)fullBg.data)[i * 4 + 0];
				((uint8_t*)maskImageScaled.data)[i * 4 + 1] = ((uint8_t*)fullBg.data)[i * 4 + 1];
				((uint8_t*)maskImageScaled.data)[i * 4 + 2] = ((uint8_t*)fullBg.data)[i * 4 + 2];
			}
			Texture2D result = LoadTextureFromImage(maskImageScaled);
			UnloadImage(maskImageScaled);
			return result;
		}

		void DrawBounds(BoundingBox bb, Color color)
		{
			Vector3 vecs[8];
			// Front face        
			vecs[0] = { bb.min.x, bb.max.y, bb.max.z }; // Top left
			vecs[1] = { bb.max.x, bb.max.y, bb.max.z }; // Top right
			vecs[2] = { bb.min.x, bb.min.y, bb.max.z }; // Bottom left
			vecs[3] = { bb.max.x, bb.min.y, bb.max.z }; // Bottom right
			// Back face
			vecs[4] = { bb.min.x, bb.max.y, bb.min.z }; // Top left
			vecs[5] = { bb.max.x, bb.max.y, bb.min.z }; // Top right
			vecs[6] = { bb.min.x, bb.min.y, bb.min.z }; // Bottom left
			vecs[7] = { bb.max.x, bb.min.y, bb.min.z }; // Bottom right

			DrawLine3D(vecs[0], vecs[1], color);
			DrawLine3D(vecs[1], vecs[3], color);
			DrawLine3D(vecs[3], vecs[2], color);
			DrawLine3D(vecs[2], vecs[0], color);

			DrawLine3D(vecs[4], vecs[5], color);
			DrawLine3D(vecs[5], vecs[7], color);
			DrawLine3D(vecs[7], vecs[6], color);
			DrawLine3D(vecs[6], vecs[4], color);

			DrawLine3D(vecs[0], vecs[4], color);
			DrawLine3D(vecs[1], vecs[5], color);
			DrawLine3D(vecs[2], vecs[6], color);
			DrawLine3D(vecs[3], vecs[7], color);
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
			auto& m = backgrounds[curCameraId].masks;
			overlayTextures.resize(m.size());
			for (int i = 0; i < m.size(); i++) {
				auto& m2 = m[i];
				overlayTextures[i].resize(m2.size());
				for (int j = 0; j < m2.size(); j++) {
					overlayTextures[i][j] = generateMask(fullBg, m[i][j]);
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

		void renderBounds() {
			//for (int r = 0; r < curCamera->rooms.size(); r++) {
			//auto& room = curStage->rooms[curCamera->rooms[r].roomId];
			for (int r = 0; r < curStage->rooms.size(); r++) {
				auto& room = curStage->rooms[r];
				for (int collId = 0; collId < room.colliders.size(); collId++) {
					if (room.colliders[collId].isZone) continue;
					DrawBounds(room.colliders[collId].bounds, DARKBLUE);
				}
			}
		}

		void renderCamPos() {
			auto& p = initCamera.position;
			auto& t = initCamera.target;
			auto s1 = string("POS: ") + to_string(p.x) + " " + to_string(p.y) + " " + to_string(p.z);
			auto s2 = string("TAR: ") + to_string(t.x) + " " + to_string(t.y) + " " + to_string(t.z);
			auto s3 = string("FOV: ") + to_string(initCamera.fovy);

			int y = 0;
			DrawText(s1.c_str(), 0, y, 30, WHITE);
			auto text_size = MeasureTextEx(GetFontDefault(), s1.c_str(), 30, 1);
			y += text_size.y;
			DrawText(s2.c_str(), 0, y, 30, WHITE);
			text_size = MeasureTextEx(GetFontDefault(), s2.c_str(), 30, 1);
			y += text_size.y;
			DrawText(s3.c_str(), 0, y, 30, WHITE);
			text_size = MeasureTextEx(GetFontDefault(), s3.c_str(), 30, 1);
			y += text_size.y;
		}

		void renderObject(GameObject& gobj, Color tint)
		{
			//Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
			//Matrix matRotation = MatrixRotate(rotationAxis, rotationAngle * DEG2RAD);
			Matrix matScale = MatrixScale(1,1,1);
			
			//Matrix matRotation = MatrixRotate({0,1,0}, 0);
			//Matrix matRotation = MatrixRotate({ 0,1,0 }, PI);
			Vector3 pos = gobj.location.position;
			auto rot = gobj.location.rotation;
			if (pos.x < 0) {
				rot.y += 180;
			}
			Matrix mx = MatrixRotateX(rot.x * 2 * PI / 360);
			Matrix my = MatrixRotateY(rot.y * 2 * PI / 360);
			Matrix mz = MatrixRotateZ(rot.z * 2 * PI / 360);
			Matrix matRotation = MatrixTranspose(MatrixMultiply(MatrixMultiply(my, mx), mz));

			Vector3& roomPos = curStage->rooms[gobj.location.roomId].position;
			pos = Vector3Add(roomPos, pos);
			//Vector3& pos = gobj.location.position;
			Matrix matTranslation = MatrixTranslate(pos.x, pos.y, pos.z);

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

		void process() {
			ClearBackground(DARKGRAY);

			DrawTexturePro(
				backgroundTex,
				{ 0, 0, (float)screenW, (float)screenH },
				{ 0, 0, (float)screenW, (float)screenH },
				{ 0, 0 }, 0, WHITE
			);

			//for (int i = 0; i < overlayTextures.size(); i++) {
			//	for (int j = 0; j < overlayTextures[i].size(); j++) {
			//		DrawTexturePro(
			//			overlayTextures[i][j],
			//			{ 0, 0, (float)screenW, (float)screenH },
			//			{ 0, 0, (float)screenW, (float)screenH },
			//			{ 0, 0 }, 0, WHITE
			//		);
			//	}
			//}

			UpdateCamera(&initCamera, CAMERA_FREE);

			BeginMode3D(initCamera);
			//rlSetMatrixModelview(curCamera->modelview);
			rlSetMatrixProjection(curCamera->perspective);
			
			DrawCube({0,0,0}, 0.2, 0.2, 0.2, GREEN);
			renderBounds();

			for (int i = 0; i < this->world->gobjects.size(); i++) {
				auto& gobj = this->world->gobjects[i];
				if (gobj.model.id == -1) continue;
				if (gobj.location.stageId != curStageId) continue;
				renderObject(gobj, WHITE);
			}
			
			EndMode3D();

			renderCamPos();
		}

	};

}