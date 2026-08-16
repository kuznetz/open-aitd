#pragma once
#include <vector>
#include <vector>
#include <string>
#include <variant>
#include "../../common/raylib_cpp.hpp"
#include "../world/world.h"
#include "../resources/resources.h"
#include "./base_renderer.h"
#include "./particle_renderer.hpp"
#include "./object_renderer.hpp"

using namespace std;
namespace openAITD {

	struct RenderOrder {
		RenderOrder* next = 0;
		std::variant<GameObject*, ParticleGroup*> renderable;
		float zPos;
		Bounds bb;
		raylib::Rectangle screenRect;
	};

	class CameraRenderer : public BaseRenderer {
	public:
		Resources* resources;
		ParticleRenderer particleRend;
    ObjectRenderer objectRend;

		std::vector<RenderOrder> renderQueue;
		RenderOrder* renderStart = 0;
		RenderOrder* renderIter = 0;
		RenderOrder* renderIterPrev = 0;
		int renderQueueCount = 0;

		bool curAltBg = false;
		Background* curBackground = 0;

		Shader maskShader = { 0 };
		int shTextureColorLoc = 0;
		int shTextureMaskLoc = 0;

		RenderTexture2D maskTex;
		RenderTexture2D colorTex;
		float scale3dTex = 1;

		CameraRenderer(World* world) : BaseRenderer(world), particleRend(*world), objectRend(*world) {
			resources = world->resources;
			renderQueue.resize(50);
			//objectRend.setYCut(0.25f);
		}

		void initShaders() {
			auto& cfg = world->resources->config;
			
			scale3dTex = resources->config.antialiasing;
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

		bool checkOverlay(const GCameraOverlay& ovl, const Vector3& pos) {
				for (int i = 0; i < ovl.bounds.size(); i++) {
						auto& b = ovl.bounds[i].getExpanded(-0.01);
						if (pos.x >= b.min.x && pos.x <= b.max.x &&
								pos.z >= b.min.z && pos.z <= b.max.z) {
								return true;
						}
				}
				return false;
		}

		void renderMask(const raylib::Rectangle& r) {
				BeginTextureMode(maskTex);
				ClearBackground(BLACK);
				BeginBlendMode(BLEND_ADDITIVE);

				auto* gobjPtr = std::get_if<GameObject*>(&renderIter->renderable);
				if (gobjPtr) {
						GameObject* gobj = *gobjPtr;
						for (int camRoomIdx = 0; camRoomIdx < curCamera->rooms.size(); camRoomIdx++) {
								if (gobj->getRoomId() != curCamera->rooms[camRoomIdx].roomId) continue;
								for (int ovlIdx = 0; ovlIdx < curCamera->rooms[camRoomIdx].overlays.size(); ovlIdx++) {
										auto& ovl = curCamera->rooms[camRoomIdx].overlays[ovlIdx];
										if (checkOverlay(ovl, gobj->getPosition())) {
												renderOverlay(curBackground->overlays[camRoomIdx][ovlIdx]);
										}
								}
						}
				} else {
						auto* pgPtr = std::get_if<ParticleGroup*>(&renderIter->renderable);
						if (pgPtr) {
								ParticleGroup* pg = *pgPtr;
								for (int camRoomIdx = 0; camRoomIdx < curCamera->rooms.size(); camRoomIdx++) {
										if (pg->roomId != curCamera->rooms[camRoomIdx].roomId) continue;
										for (int ovlIdx = 0; ovlIdx < curCamera->rooms[camRoomIdx].overlays.size(); ovlIdx++) {
												auto& ovl = curCamera->rooms[camRoomIdx].overlays[ovlIdx];
												if (checkOverlay(ovl, pg->position)) {
														renderOverlay(curBackground->overlays[camRoomIdx][ovlIdx]);
												}
										}
								}
						}
				}

				EndBlendMode();
				EndTextureMode();
		}

		void fillRenderOrder(RenderOrder& ord, GameObject& gobj)
		{
			ord.next = 0;
			ord.renderable = &gobj;
			auto rmodel = resources->models.getModel(gobj.modelId, world->altModels);
			processSkin(gobj, rmodel->model);
			ord.bb = gobj.getRenderBounds();
			boundsToScreen(ord.bb, ord.screenRect, ord.zPos);
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

		void insertIntoSortedQueue(RenderOrder& ro) {
				if (renderStart) {
						bool inserted = false;
						renderIterPrev = 0;
						renderIter = renderStart;
						while (true) {
								if (renderIter->zPos < ro.zPos) {
										if (renderIterPrev) {
												renderIterPrev->next = &ro;
										} else {
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
				} else {
						renderStart = &ro;
				}
		}

		void render() {
				if (maskShader.id == 0) {
						initShaders();
				}

				if (world->curStageId == -1 || world->curCameraId == -1) return;

				if (curAltBg != resources->backgrounds.isAltBackgrounds) {
						curAltBg = resources->backgrounds.isAltBackgrounds;
						curCameraId = -1;
				}

				if (world->curStageId != curStageId || world->curCameraId != curCameraId) {
						curStageId = world->curStageId;
						loadCamera(world->curCameraId);
				}

				renderQueueCount = 0;
				renderStart = 0;

				//GameObjects
				for (int i = 0; i < this->world->gobjects.size(); i++) {
						auto& gobj = this->world->gobjects[i];
						if (gobj.modelId == -1) continue;
						if (gobj.getStageId() != curStageId) continue;

						int curCamRoom = -1;
						for (int j = 0; j < curCamera->rooms.size(); j++) {
								if (gobj.getRoomId() == curCamera->rooms[j].roomId) {
										curCamRoom = j;
										break;
								}
						}
						if (curCamRoom == -1) continue;

						Vector3 pos = gobj.getPosition();
						Vector3& roomPos = world->curStage->rooms[gobj.getRoomId()].origPosition;
						pos = Vector3Add(roomPos, pos);

						RenderOrder& ro = renderQueue[renderQueueCount++];
						fillRenderOrder(ro, gobj);
						if (ro.zPos < 0) continue;
						if ((ro.screenRect.x + ro.screenRect.width) < 0 || (ro.screenRect.x) > getScreenW()) continue;
						if ((ro.screenRect.y + ro.screenRect.height) < 0 || (ro.screenRect.y) > getScreenH()) continue;

						insertIntoSortedQueue(ro); 
				}

				//ParticleGroups
				for (auto& pg : world->partGroups.groups) {
						if (!pg.active) continue;
						if (pg.stageId != curStageId) continue;

						int curCamRoom = -1;
						for (int j = 0; j < curCamera->rooms.size(); j++) {
								if (pg.roomId == curCamera->rooms[j].roomId) {
										curCamRoom = j;
										break;
								}
						}
						if (curCamRoom == -1) continue;

						Vector3 roomPos = world->curStage->rooms[pg.roomId].origPosition;
						Vector3 worldPos = Vector3Add(roomPos, pg.position);

						RenderOrder& ro = renderQueue[renderQueueCount++];
						ro.next = 0;
						ro.renderable = &pg;
						pg.calcBounds();
						ro.bb = pg.getRenderBounds();
						boundsToScreen(ro.bb, ro.screenRect, ro.zPos);
						ro.zPos = GetWorldToScreenZ(pg.position).z;

						if (ro.zPos < 0) continue;
						if ((ro.screenRect.x + ro.screenRect.width) < 0 || (ro.screenRect.x) > getScreenW()) continue;
						if ((ro.screenRect.y + ro.screenRect.height) < 0 || (ro.screenRect.y) > getScreenH()) continue;

						insertIntoSortedQueue(ro);
				}

				BeginTextureMode(resources->screen.sceneTex);
				ClearBackground(BLACK);
				DrawTexturePro(
						curBackground->texture,
						{ 0, 0, (float)getScreenW(), (float)getScreenH() },
						{ 0, 0, (float)getScreenW(), (float)getScreenH() },
						{ 0, 0 }, 0, WHITE
				);
				EndTextureMode();

				if (renderStart) {
						renderIter = renderStart;
						while (renderIter) {
								raylib::Rectangle& r = renderIter->screenRect;
								renderMask(r);
								BeginTextureMode(colorTex);
								ClearBackground(BLANK);
								
								BeginMode3D(mainCamera);
								rlSetMatrixProjection(perspective);

								auto* gobjPtr = std::get_if<GameObject*>(&renderIter->renderable);
								if (gobjPtr) {
										objectRend.renderObject(**gobjPtr);
								} else {
										auto* pgPtr = std::get_if<ParticleGroup*>(&renderIter->renderable);
										if (pgPtr) {
												particleRend.render(**pgPtr, mainCamera);
										}
								}

								EndMode3D();
								EndTextureMode();

								BeginTextureMode(resources->screen.sceneTex);
								BeginBlendMode(BLEND_ALPHA);
								BeginShaderMode(maskShader);
								SetShaderValueTexture(maskShader, shTextureColorLoc, colorTex.texture);
								SetShaderValueTexture(maskShader, shTextureMaskLoc, maskTex.texture);
								renderMasked(colorTex.texture, r);
								EndShaderMode();
								EndBlendMode();
								EndTextureMode();

								renderIter = renderIter->next;
						}
				}
		}

	};

}