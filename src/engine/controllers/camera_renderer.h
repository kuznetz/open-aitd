#pragma once
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
			
			auto& fullBg = resizeImg(backgrounds[newCameraId].background, screenW, screenH);
			backgroundTex = LoadTextureFromImage(fullBg);
			auto& m = backgrounds[newCameraId].masks;
			overlayTextures.resize(m.size());
			for (int i = 0; i < m.size(); i++) {
				auto& m2 = m[i];
				overlayTextures[i].resize(m2.size());
				for (int j = 0; j < m2.size(); j++) {
					overlayTextures[i][j] = generateMask(fullBg, m[i][j]);
				}
			}
			UnloadImage(fullBg);
			curCameraId = newCameraId;

			//It's ugly, but it didn't get any better.
			initCamera.position = curCamera->position;
			Matrix vw = QuaternionToMatrix(curCamera->rotation);
			initCamera.target = Vector3Add(curCamera->position, Vector3Negate({ vw.m8, vw.m9, vw.m10 }));
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

			//UpdateCamera(&initCamera, CAMERA_FREE);

			BeginMode3D(initCamera);
			//rlSetMatrixModelview(curCamera->modelview);
			rlSetMatrixProjection(curCamera->prespective);

			//DrawCube({0,0,0}, 0.5, 0.5, 0.5, GREEN);

			for (int r = 0; r < curCamera->rooms.size(); r++) {
				auto& room = curStage->rooms[curCamera->rooms[r].roomId];
				for (int collId = 0; collId < room.colliders.size(); collId++) {
					if (room.colliders[collId].isZone) continue;
					DrawBounds(room.colliders[collId].bounds, DARKBLUE);					
				}
			}

			for (int i = 0; i < this->world->gobjects.size(); i++) {
				auto& gobj = this->world->gobjects[i];
				if (gobj.model.id == -1) continue;
				if (gobj.location.stageId != curStageId) continue;
				Vector3& roomPos = curStage->rooms[gobj.location.roomId].position;
				Vector3 pos = Vector3Add(roomPos, gobj.location.position);
				auto mdl = resources->models.getModel(gobj.model.id);
			    DrawModelEx(mdl->model, pos, {0,1,0}, 0, {1,1,1}, WHITE);
			}
			
			EndMode3D();
		}

	};

}