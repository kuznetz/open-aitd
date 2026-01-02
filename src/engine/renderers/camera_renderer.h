#pragma once
#include <vector>
#include <string>
#include "../raylib-cpp.h"
#include "../world/world.h"
#include "../resources/resources.h"
#include "./base_renderer.h"

using namespace std;
namespace openAITD {

	struct RenderOrder {
		RenderOrder* next = 0;
		GameObject* gobj = 0;
		float zPos;
		Bounds bb;
		Vector2 screenMin;
		Vector2 screenMax;
	};

	class CameraRenderer : public BaseRenderer {
	public:
		Resources* resources;

		RenderOrder renderQueue[100];
		RenderOrder* renderStart = 0;
		RenderOrder* renderIter = 0;
		RenderOrder* renderIterPrev = 0;
		int renderQueueCount = 0;

		Background* curBackground = 0;

		Shader maskShader = { 0 };
		int shTextureColorLoc = 0;
		int shTextureMaskLoc = 0;

		RenderTexture2D maskTex;
		RenderTexture2D colorTex;
		float scale3dTex = 1;

		CameraRenderer(World* world) : BaseRenderer(world) {
			resources = world->resources;
		}

		void initShaders() {
			auto& cfg = world->resources->config;
			
			colorTex = LoadRenderTexture(cfg.screenW * scale3dTex, cfg.screenH * scale3dTex);
			SetTextureFilter(colorTex.texture, (scale3dTex == 1.0)? TEXTURE_FILTER_POINT: TEXTURE_FILTER_BILINEAR);
			maskTex = LoadRenderTexture(cfg.screenW, cfg.screenH);

			//maskShader = LoadShaderFromMemory(vertexShaderSrc, fragmentShaderSrc);
			maskShader = LoadShader(
				"newdata/shaders/glsl330/mask.vs",
				"newdata/shaders/glsl330/mask.fs"
			);
			// Get shader uniform locations
			shTextureColorLoc = GetShaderLocation(maskShader, "texture0");
			shTextureMaskLoc = GetShaderLocation(maskShader, "texture1");
		}

		void loadCamera(int newCameraId) override
		{
			BaseRenderer::loadCamera(newCameraId);
			curBackground = resources->backgrounds.get(world->curStageId, newCameraId);
		}

		bool checkOverlay(GCameraOverlay& ovl, GameObject& gobj) {
			Vector3 pos = gobj.location.position;
			for (int i = 0; i < ovl.bounds.size(); i++) {
				auto& b = ovl.bounds[i].getExpanded(-0.01);
				//if (CheckCollisionBoxes(objBnd, ovl.bounds[i])) {
				if (pos.x >= b.min.x && pos.x <= b.max.x && pos.z >= b.min.z && pos.z <= b.max.z) {
					return true;
				}
			}
			return false;
		}

		void renderMask(const raylib::Rectangle& r) {
			BeginTextureMode(maskTex);
			ClearBackground(BLACK);
			BeginBlendMode(BLEND_ADDITIVE);
			for (int camRoomIdx = 0; camRoomIdx < curCamera->rooms.size(); camRoomIdx++) {
				if (renderIter->gobj->location.roomId != curCamera->rooms[camRoomIdx].roomId) continue;
				for (int ovlIdx = 0; ovlIdx < curCamera->rooms[camRoomIdx].overlays.size(); ovlIdx++) {
					auto& ovl = curCamera->rooms[camRoomIdx].overlays[ovlIdx];
					if (checkOverlay(ovl, *renderIter->gobj)) {
						renderOverlay(curBackground->overlays[camRoomIdx][ovlIdx]);
					}
				}
			}
			EndBlendMode();
			EndTextureMode();

			/*
			auto testImg = LoadImageFromTexture(maskTex.texture);
    		raylib::ExportImage(testImg, "./image-test.png");
			UnloadImage(testImg);
			*/

			/*
			BeginTextureMode(resources->screen.screenTex);
			BeginBlendMode(BLEND_ADDITIVE);
			//DrawTexturePro( maskTex.texture, r, r, { 0, 0 }, 0, WHITE );
			DrawTextureRec(maskTex.texture, { 0, 0, (float)maskTex.texture.width, (float)-maskTex.texture.height }, { 0, 0 }, WHITE);
			EndBlendMode();
			EndTextureMode();
			*/
		}

		void fillRenderOrder(RenderOrder& ord, GameObject& gobj)
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

			rmodel->model.CalcBounds();
			auto& bb = ord.bb;
			bb = rmodel->model.bounds;
			bb = bb.getRotatedBounds(rot);
			Vector3TransformRef(bb.min, matTranslation);
			Vector3TransformRef(bb.max, matTranslation);
			
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
				Vector3 v = GetWorldToScreenZ(vecs[i]);
				if (i == 0) {
					ord.zPos = v.z;
					ord.screenMin.x = v.x;
					ord.screenMin.y = v.y;
					ord.screenMax.x = v.x;
					ord.screenMax.y = v.y;
					continue;
				}
				if (ord.zPos > v.z) {
					ord.zPos = v.z;
				}
				if (ord.screenMin.x > v.x) {
					ord.screenMin.x = v.x;
				}
				if (ord.screenMin.y > v.y) {
					ord.screenMin.y = v.y;
				}
				if (ord.screenMax.x < v.x) {
					ord.screenMax.x = v.x;
				}
				if (ord.screenMax.y < v.y) {
					ord.screenMax.y = v.y;
				}
			}
		}

		void renderMasked(const Texture2D tex, const raylib::Rectangle& r) {
			float width = maskTex.texture.width;
			float height = maskTex.texture.height;
			Vector2 topLeft = { 
				r.x / width,
				(height - r.y) / height
			};
			Vector2 botRight = {
				(r.x + r.width) / width,
				(height - (r.y+r.height)) / height
			};
			
			rlSetTexture(tex.id);
			rlBegin(RL_QUADS);
			//rlColor4ub(1,1,1,1);
			//rlNormal3f(0.0f, 0.0f, 1.0f);
			// Top-left corner for texture and quad
			rlTexCoord2f(topLeft.x, topLeft.y);
			rlVertex2f(r.x, r.y);
			// Bottom-left corner for texture and quad
			rlTexCoord2f(topLeft.x, botRight.y);
			rlVertex2f(r.x, r.y + r.height);
			// Bottom-right corner for texture and quad
			rlTexCoord2f(botRight.x, botRight.y);
			rlVertex2f(r.x + r.width, r.y + r.height);
			// Top-right corner for texture and quad
			rlTexCoord2f(botRight.x, topLeft.y);
			rlVertex2f(r.x + r.width, r.y);
			rlEnd();
		}

		void renderOverlay(const BackgroundOverlay ovl) {
			auto& b = ovl.bounds;
			rlSetTexture(ovl.texture.id);
			rlBegin(RL_QUADS);
			//rlColor4ub(1,1,1,1);
			//rlNormal3f(0.0f, 0.0f, 1.0f);
			// Top-left corner for texture and quad
			rlTexCoord2f(0, 0);
			rlVertex2f(b.x, b.y);
			// Bottom-left corner for texture and quad
			rlTexCoord2f(0, 1);
			rlVertex2f(b.x, b.y + b.height);
			// Bottom-right corner for texture and quad
			rlTexCoord2f(1, 1);
			rlVertex2f(b.x + b.width, b.y + b.height);
			// Top-right corner for texture and quad
			rlTexCoord2f(1, 0);
			rlVertex2f(b.x + b.width, b.y);
			rlEnd();
		}

		void render() {
			if (maskShader.id == 0) {
				initShaders();
			}

			if (world->inDark) {
				resources->screen.begin();
				renderMessage();
				resources->screen.end();
				return;
			}

    		if (world->curStageId == -1 || world->curCameraId == -1) return;
			if (world->curStageId != curStageId || world->curCameraId != curCameraId) {
				curStageId = world->curStageId;
				loadCamera(world->curCameraId);
			}

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

				if (gobj.modelId != -1) {
					auto rmodel = resources->models.getModel(gobj.modelId);
					ProcessPose(gobj, rmodel->model);
				}

				//auto& screenPos = GetWorldToScreenZ(pos);
				//if (screenPos.z < 0) continue;

				RenderOrder& ro = renderQueue[renderQueueCount++];
				fillRenderOrder(ro, gobj);
				if (ro.zPos < 0) continue;
				if (ro.screenMax.x < 0 || ro.screenMin.x > getScreenW()) continue;
				if (ro.screenMax.y < 0 || ro.screenMin.y > getScreenH()) continue;

				//renderObject(gobj, WHITE);				
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

			BeginTextureMode(resources->screen.screenTex);
			ClearBackground(BLACK);

			DrawTexturePro(
				curBackground->texture,
				{ 0, 0, (float)getScreenW(), (float)getScreenH() },
				{ 0, 0, (float)getScreenW(), (float)getScreenH() },
				{ 0, 0 }, 0, WHITE
			);

			EndTextureMode();

			if (renderStart) {
				int num = 1;
				renderIter = renderStart;
				while (true) {
					//auto s = to_string(num)+" R" + to_string(it->obj->location.roomId);
					//auto s = to_string(it->obj->);

					raylib::Rectangle r = {
						renderIter->screenMin.x,
						renderIter->screenMin.y,
						(renderIter->screenMax.x - renderIter->screenMin.x) + 1,
						(renderIter->screenMax.y - renderIter->screenMin.y) + 1
					};
					raylib::Rectangle r2 = {
						renderIter->screenMin.x,
						GetScreenHeight() - renderIter->screenMin.y,
						(renderIter->screenMax.x - renderIter->screenMin.x) + 1,
						- (renderIter->screenMax.y - renderIter->screenMin.y) - 1
					};
					renderMask(r);
					
					BeginTextureMode(colorTex);
					ClearBackground(BLANK);
					BeginMode3D(mainCamera);
					//rlSetMatrixModelview(curCamera->modelview);
					rlSetMatrixProjection(perspective);
					renderObject(*renderIter->gobj, WHITE);
					//DrawBounds(renderIter->bb, GREEN);
					EndMode3D();
					EndTextureMode();

					/*
					auto testImg = LoadImageFromTexture(colorTex.texture);
					raylib::ExportImage(testImg, "./image-test.png");
					UnloadImage(testImg);
					*/

					BeginTextureMode(resources->screen.screenTex);
					//BeginBlendMode(BLEND_ALPHA);
					BeginShaderMode(maskShader);
					SetShaderValueTexture(maskShader, shTextureColorLoc, colorTex.texture);
					SetShaderValueTexture(maskShader, shTextureMaskLoc, maskTex.texture);
					
					//DrawTextureRec(colorTex.texture, { 0, 0, (float)colorTex.texture.width, ((float)-colorTex.texture.height) / 2 }, { 0, 0 }, WHITE);
					renderMasked(colorTex.texture, r);
					//DrawRectangleLinesEx(r2, 1, RED);

					EndShaderMode();
					//EndBlendMode();
					EndTextureMode();

					//it->marker = s;
					num++;
					renderIter = renderIter->next;
					if (!renderIter) break;
				}
			}

			BeginTextureMode(resources->screen.screenTex);
			renderMessage();
			EndTextureMode();
			resources->screen.finalRender();

			//for (auto it = renderQueue.begin(); it != renderQueue.end(); it++) {
			//	DrawLine(it->screenMin.x, it->screenMin.y, it->screenMax.x, it->screenMin.y, RED);
			//	DrawLine(it->screenMin.x, it->screenMin.y, it->screenMin.x, it->screenMax.y, RED);
			//	DrawLine(it->screenMax.x, it->screenMax.y, it->screenMax.x, it->screenMin.y, RED);
			//	DrawLine(it->screenMax.x, it->screenMax.y, it->screenMin.x, it->screenMax.y, RED);
			//}

		}

	};

}