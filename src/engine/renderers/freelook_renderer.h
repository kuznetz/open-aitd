#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../raylib-cpp.h"
#include "./base_renderer.h"

using namespace std;

namespace openAITD {

	struct Debug3DText {
		Vector2 screenpos;
		string text;
		Color color;
	};

	class FreelookRenderer : public BaseRenderer {
	public:
		bool freeLook = true;
		Debug3DText debugObjs[100];
		Debug3DText* endDebugObjs;

		int invX = 0;
		int invZ = 0;

		FreelookRenderer(World* world) : BaseRenderer(world)
		{

		}

		void renderBounds() {
			//for (int r = 0; r < curCamera->rooms.size(); r++) {
			//auto& room = curStage->rooms[curCamera->rooms[r].roomId];
			for (int r = 0; r < world->curStage->rooms.size(); r++) {
				auto& room = world->curStage->rooms[r];
				int curRoomId = -1;
				if (world->followTarget) {
					curRoomId = world->followTarget->location.roomId;
				}
				Color c = (r == curRoomId) ? WHITE : DARKBLUE;
				DrawCube(room.position, 0.1, 0.1, 0.1, c);
				rlPushMatrix();
				rlTranslatef(room.position.x, room.position.y, room.position.z);
				for (int collId = 0; collId < room.colliders.size(); collId++) {
					c = DARKBLUE;
					if (r == curRoomId) {
						c = (room.colliders[collId].type == 9) ? RED : WHITE;
					}
					DrawBounds(room.colliders[collId].bounds, c);
				}
				rlPopMatrix();
				/*for (int collId = 0; collId < room.zondes.size(); collId++) {
					DrawBounds(room.colliders[collId].bounds, DARKBLUE);
				}*/
			}
		}

		void renderZones() {
			if (!world->followTarget) return;
			int curRoomId = world->followTarget->location.roomId;
			if (curRoomId == -1) return;
			auto& room = world->curStage->rooms[curRoomId];
			rlPushMatrix();
			rlTranslatef(room.position.x, room.position.y, room.position.z);
			for (int z = 0; z < room.zones.size(); z++) {
				auto& zone = room.zones[z];
				if (zone.type == RoomZoneType::ChangeStage) {
					DrawBounds(zone.bounds, YELLOW);
					renderDebugText3D(Vector3Add(room.position, zone.bounds.min), to_string(zone.parameter), YELLOW);
					
				} else {
					DrawBounds(zone.bounds, MAGENTA);
				}
			}
			rlPopMatrix();
		}

		void renderOvlBounds() {
			if (!curCamera) return;
			for (int r = 0; r < curCamera->rooms.size(); r++) {
				auto& room = curCamera->rooms[r];
				for (int ovId = 0; ovId < room.overlays.size(); ovId++) {
					if (ovId != 2) continue;
					for (int b = 0; b < room.overlays[ovId].bounds.size(); b++) {
						DrawBounds(room.overlays[ovId].bounds[b], DARKGREEN);
					}
				}
			}
		}

		void renderCameraZones() {
			if (!curCamera) return;
			for (int i = 0; i < curCamera->coverZones.size(); i++) {
				auto& poly = curCamera->coverZones[i];				
				Vector3 oldP;
				Vector3 p = { poly[poly.size() - 1].x, 0 ,poly[poly.size() - 1].y };
				for (int j = 0; j < poly.size(); j++) {
					oldP = p;
					p = { poly[j].x, 0 ,poly[j].y };
					DrawLine3D(oldP, p, RED);
				}
			}
		}

		void renderCamPos() {
			auto& p = mainCamera.position;
			auto& t = mainCamera.target;
			auto s1 = string("POS: ") + to_string(p.x) + " " + to_string(p.y) + " " + to_string(p.z);
			auto s2 = string("TAR: ") + to_string(t.x) + " " + to_string(t.y) + " " + to_string(t.z);
			auto s3 = string("IND: ") + to_string(invX) + " " + to_string(invZ);

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

		void renderDebugText3D(Vector3 pos, const string& text, Color color) {
			auto screenPos = GetWorldToScreenZ(pos);
			if (screenPos.z > 0) {
				renderDebugText({ screenPos.x, screenPos.y }, text, color);
			}
		}

		void renderDebugText(Vector2 screenpos, const string& text, Color color) {
			endDebugObjs->screenpos = screenpos;
			endDebugObjs->text = text;
			endDebugObjs->color = color;
			endDebugObjs++;
		}

		void renderObjectEx(GameObject& gobj, Color tint)
		{
			if (gobj.modelId != -1) {
				auto rmodel = resources->models.getModel(gobj.modelId);
				ProcessPose(gobj, rmodel->model);
				renderObject(gobj, tint);
			}

			rlPushMatrix();
  		Vector3& pos = gobj.location.position;			
			Vector3& roomPos = world->curStage->rooms[gobj.location.roomId].position;
			rlMultMatrixf(MatrixToFloat(MatrixTranslate(roomPos.x,roomPos.y,roomPos.z)));

			DrawCube(pos, 0.1, 0.1, 0.1, RED);

			string debugStr = to_string(gobj.id);
  		renderDebugText3D(Vector3Add(roomPos, pos), debugStr, RED);

			if (gobj.physics.boundsCached) {
				DrawBounds(gobj.physics.bounds, RED);
			}
			if (gobj.hit.active) {
				DrawBounds(gobj.hit.bounds, RED);
			}

			//rlMultMatrixf(MatrixToFloat(MatrixTranslate(pos.x, pos.y, pos.z)));
			//auto m = rmodel;
			//Vector3 mMin = Vector3RotateByQuaternion(m->bounds.min, gobj.location.rotation);
			//Vector3 mMax = Vector3RotateByQuaternion(m->bounds.max, gobj.location.rotation);
			//DrawCube(mMin, 0.1, 0.1, 0.1, YELLOW);
			//DrawCube(mMax, 0.1, 0.1, 0.1, MAGENTA);

			if (gobj.track.mode == GOTrackMode::track) {

				DrawLine3D(pos, gobj.track.targetPos, ORANGE);
				
				/*Vector3 forw = { 
					pos.x + gobj.track.debug.forward2D.x,
					pos.y,
					pos.z + gobj.track.debug.forward2D.y
				};				
				DrawCube(forw, 0.1, 0.1, 0.1, PINK);*/
				Vector3 targDir = { 
					pos.x + gobj.track.debug.targetDir.x,
					pos.y,
					pos.z + gobj.track.debug.targetDir.y
				};				
				DrawCube(targDir, 0.1, 0.1, 0.1, BROWN);
				renderDebugText3D(Vector3Add(roomPos, targDir), to_string(gobj.track.debug.angle), PINK);
			}

			rlPopMatrix();
		}

		void renderTrack()
		{
			auto& track = resources->tracks[29];
			Vector3 from = { 0,0,0 };
			Vector3 to;
			for (int i = 0; i < track.size(); i++) {
				if (track[i].type == TrackItemType::GOTO_POS) {
					to = Vector3Add(track[i].pos, world->curStage->rooms[track[i].room].position);
					DrawLine3D(from, to, ORANGE);
					from = to;
				}
			}
		}

		void process() {
			if (freeLook) {
				UpdateCamera(&mainCamera, CAMERA_FREE);
			}
		}

		void render() {
			if (world->curStageId == -1 || world->curCameraId == -1) return;
			if (world->curStageId != curStageId || world->curCameraId != curCameraId) {
				curStageId = world->curStageId;
				loadCamera(world->curCameraId);
			}

			endDebugObjs = debugObjs;

			/*
			for (int i = 0; i < this->world->gobjects.size(); i++) {
				auto& gobj = this->world->gobjects[i];
				if (gobj.model.id == -1) continue;
				if (gobj.location.stageId != curStageId) continue;
				renderObjectEx(gobj, WHITE);
			}
			*/

			//TODO: Remove memory allocation
			//list<RenderOrder> renderQueue;
			for (int i = 0; i < this->world->gobjects.size(); i++) {
				auto& gobj = this->world->gobjects[i];
				if (gobj.location.stageId != curStageId) continue;
				
				Vector3 pos = gobj.location.position;
				Vector3& roomPos = world->curStage->rooms[gobj.location.roomId].position;

				//auto& screenPos = GetWorldToScreenZ(pos);
				//if (screenPos.z < 0) continue;
				BeginMode3D(mainCamera);
				if (curCamera) rlSetMatrixProjection(perspective);
				renderObjectEx(gobj, WHITE);
				EndMode3D();

				string s = to_string(i);
				pos = Vector3Add(roomPos, pos);
				DrawText(s.c_str(), (int)(pos.x), (int)(pos.y), 20, WHITE);
			}
			

			//int num = 1;
			//for (auto it = renderQueue.begin(); it != renderQueue.end(); it++) {
			//	BeginMode3D(mainCamera);
			//		//rlSetMatrixModelview(curCamera->modelview);
			//		rlSetMatrixProjection(perspective);
			//		renderObject(*it->obj, WHITE);
			//	EndMode3D();

			//	//auto s = to_string(num)+" R" + to_string(it->obj->location.roomId);
			//	//auto s = to_string(it->obj->);
			//	//it->marker = s;
			//	num++;
			//}

			BeginMode3D(mainCamera);
				//rlSetMatrixModelview(curCamera->modelview);
				if (curCamera) rlSetMatrixProjection(perspective);
				DrawCube({ 0,0,0 }, 0.2, 0.2, 0.2, GREEN);
				renderBounds();
				renderZones();
				renderCameraZones();
				//renderOvlBounds();
				//renderTrack();
			EndMode3D();

			for (int i = 0; i < 100; i++) {
				if (&debugObjs[i] == endDebugObjs) break;
				auto& dobj = debugObjs[i];
				DrawText(dobj.text.c_str(), (int)dobj.screenpos.x, (int)dobj.screenpos.y, 24, dobj.color);
			}

			renderCamPos();
		}

	};

}