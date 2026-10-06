#pragma once
#include <vector>
#include <string>
#include <variant>
#include "common/raylib_cpp.hpp"
#include "engine/world/world.h"
#include "engine/resources/resources.h"
#include "../base_renderer.hpp"
#include "./particle_renderer.hpp"
#include "./object_renderer.hpp"
#include "./mask_renderer.hpp"

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
        MaskRenderer maskRenderer;

        std::vector<RenderOrder> renderQueue;
        RenderOrder* renderStart = 0;
        RenderOrder* renderIter = 0;
        RenderOrder* renderIterPrev = 0;
        int renderQueueCount = 0;

        bool curAltBg = false;
        Background* curBackground = 0;

        RenderTexture2D colorTex;
        float scale3dTex = 1;

        CameraRenderer(World* world) : BaseRenderer(world), particleRend(*world), objectRend(*world) {
            resources = world->resources;
            renderQueue.resize(50);
        }

        void initShaders() {
            auto& cfg = world->resources->config;
            maskRenderer.init(cfg);

            scale3dTex = resources->config.antialiasing;
            colorTex = LoadRenderTexture(cfg.screenW * scale3dTex, cfg.screenH * scale3dTex);
            SetTextureFilter(colorTex.texture, (scale3dTex == 1.0) ? TEXTURE_FILTER_POINT : TEXTURE_FILTER_BILINEAR);
        }

        void loadCamera(int newCameraId) override {
            BaseRenderer::loadCamera(newCameraId);
            curBackground = resources->backgrounds.get(world->curStageId, newCameraId);
        }

        void fillRenderOrder(RenderOrder& ord, GameObject& gobj) {
            ord.next = 0;
            ord.renderable = &gobj;
            auto rmodel = resources->models.getModel(gobj.modelId, world->altModels);
            processSkin(gobj, rmodel->model);
            ord.bb = gobj.getRenderBounds();
            boundsToScreen(ord.bb, ord.screenRect, ord.zPos);
        }

        // Returns true when the whole bounding box is at or behind the camera
        // plane. The camera looks down -Z in view space, so a point is in front
        // when its view-space z is negative. Objects at/near the camera still
        // have at least one corner in front, so they must remain in the queue
        // (the near clipping plane itself is handled by the GPU).
        bool isBehindCamera(const Bounds& bb) {
            const Vector3 corners[8] = {
                { bb.min.x, bb.min.y, bb.min.z }, { bb.max.x, bb.min.y, bb.min.z },
                { bb.min.x, bb.max.y, bb.min.z }, { bb.max.x, bb.max.y, bb.min.z },
                { bb.min.x, bb.min.y, bb.max.z }, { bb.max.x, bb.min.y, bb.max.z },
                { bb.min.x, bb.max.y, bb.max.z }, { bb.max.x, bb.max.y, bb.max.z },
            };
            for (int i = 0; i < 8; i++) {
                Vector3 v = Vector3Transform(corners[i], world->cameraView);
                if (v.z < 0.0f) return false; // at least one corner is in front
            }
            return true;
        }

        void MyBeginMode3D() {
            rlDrawRenderBatchActive();
            rlPushMatrix();
            rlSetMatrixProjection(world->cameraProjection);
            rlSetMatrixModelview(world->cameraView);
            rlEnableDepthTest();
        }

        void render() {
            if (maskRenderer.maskShader.id == 0) {
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

            // ---- Collect objects into queue ----
            for (auto& gobj : this->world->gobjects) {
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

                RenderOrder& ro = renderQueue[renderQueueCount++];
                fillRenderOrder(ro, gobj);
                if (isBehindCamera(ro.bb)) continue;
                if ((ro.screenRect.x + ro.screenRect.width) < 0 || ro.screenRect.x > getScreenW()) continue;
                if ((ro.screenRect.y + ro.screenRect.height) < 0 || ro.screenRect.y > getScreenH()) continue;

                insertIntoSortedQueue(ro);
            }

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

                RenderOrder& ro = renderQueue[renderQueueCount++];
                ro.next = 0;
                ro.renderable = &pg;
                pg.calcBounds();

								const Vector3& roomPos = resources->stages[pg.stageId].rooms[pg.roomId].origPosition;
								Vector3 pos = Vector3Add(pg.position, roomPos);

                ro.bb = pg.getRenderBounds();
								ro.bb.min = Vector3Add(ro.bb.min, roomPos);
								ro.bb.max = Vector3Add(ro.bb.max, roomPos);
								ro.bb.correctBounds();

                boundsToScreen(ro.bb, ro.screenRect, ro.zPos);
                ro.zPos = world->WorldToScreenZ(pos).z;

                if (isBehindCamera(ro.bb)) continue;
                if ((ro.screenRect.x + ro.screenRect.width) < 0 || ro.screenRect.x > getScreenW()) continue;
                if ((ro.screenRect.y + ro.screenRect.height) < 0 || ro.screenRect.y > getScreenH()) continue;

                insertIntoSortedQueue(ro);
            }

            // ---- Background rendering ----
            BeginTextureMode(resources->screen.sceneTex);
            ClearBackground(BLACK);
            DrawTexturePro(
                curBackground->texture,
                { 0, 0, (float)getScreenW(), (float)getScreenH() },
                { 0, 0, (float)getScreenW(), (float)getScreenH() },
                { 0, 0 }, 0, WHITE
            );
            EndTextureMode();

            // ---- Process each object in the queue ----
            if (renderStart) {
                renderIter = renderStart;
                while (renderIter) {
                    raylib::Rectangle& r = renderIter->screenRect;

                    // 1. Render the mask for the current object
                    int roomId;
                    Vector3 pos;
                    if (auto* gobjPtr = std::get_if<GameObject*>(&renderIter->renderable)) {
                        GameObject* gobj = *gobjPtr;
                        roomId = gobj->getRoomId();
                        pos = gobj->getPosition();
                    } else if (auto* pgPtr = std::get_if<ParticleGroup*>(&renderIter->renderable)) {
                        ParticleGroup* pg = *pgPtr;
                        roomId = pg->roomId;
                        pos = pg->position;
                    } else {
                        renderIter = renderIter->next;
                        continue;
                    }
                    maskRenderer.renderMask(roomId, pos, *curCamera, *curBackground);										

                    // 2. Render the color texture (the object itself)
                    BeginTextureMode(colorTex);
                    ClearBackground(BLANK);
                    if (auto* gobjPtr = std::get_if<GameObject*>(&renderIter->renderable)) {
                        MyBeginMode3D();
                        objectRend.renderObject(**gobjPtr);
                        EndMode3D();
                    } else if (auto* pgPtr = std::get_if<ParticleGroup*>(&renderIter->renderable)) {
                        particleRend.render(**pgPtr);
                        // MyBeginMode3D();
												// DrawBounds(renderIter->bb, RED);
                        // EndMode3D();
                    }
                    EndTextureMode();

                    // 3. Output the color texture with the mask applied.
                    // The object color texture holds premultiplied alpha (see
                    // ObjectRenderer), so composite it with the matching
                    // premultiplied-alpha blend mode to keep translucent
                    // materials blended correctly over the background and over
                    // previously drawn objects.
                    BeginTextureMode(resources->screen.sceneTex);
                    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
                    maskRenderer.renderMasked(colorTex.texture, r);
                    EndBlendMode();
                    EndTextureMode();

                    renderIter = renderIter->next;
                }
            }
        }

        // Helper method to insert into the sorted list (remains unchanged)
        void insertIntoSortedQueue(RenderOrder& ro) {
            if (renderStart) {
                bool inserted = false;
                renderIterPrev = 0;
                renderIter = renderStart;
                while (true) {
                    if (renderIter->zPos < ro.zPos) {
                        if (renderIterPrev) renderIterPrev->next = &ro;
                        else renderStart = &ro;
                        ro.next = renderIter;
                        inserted = true;
                        break;
                    }
                    if (!renderIter->next) break;
                    renderIterPrev = renderIter;
                    renderIter = renderIter->next;
                }
                if (!inserted) renderIter->next = &ro;
            } else {
                renderStart = &ro;
            }
        }
    };
}