#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../raylib.h"
#include "./base_renderer.h"

using namespace std;
namespace openAITD {

	struct RenderOrder {
		RenderOrder* next = 0;
		GameObject* gobj = 0;
		float zPos;
		Vector2 screenMin;
		Vector2 screenMax;
	};

	class CameraRenderer : public BaseRenderer {
	public:
		RenderOrder renderQueue[100];
		RenderOrder* renderStart;
		RenderOrder* renderIter;
		RenderOrder* renderIterPrev;
		int renderQueueCount = 0;

		Background* curBackground = 0;

		CameraRenderer(World* world) : BaseRenderer(world) {}

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

			if (renderStart) {
				int num = 1;
				renderIter = renderStart;
				while (true) {
					BeginMode3D(mainCamera);
					//rlSetMatrixModelview(curCamera->modelview);
					rlSetMatrixProjection(perspective);
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