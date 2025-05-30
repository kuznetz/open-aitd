﻿#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../raylib.h"
#include "./common_renderers.h"

using namespace std;

struct Debug3DStruct {
	Vector2 screenpos;
	Color color = WHITE;
	char label[100] = "";
};

namespace openAITD {

	class FreelookRenderer {
	public:
		bool freeLook = true;
		Debug3DStruct debug3d[100];
		Debug3DStruct* endDebug3d;
		World* world;
		Resources* resources;
		int screenW = 0;
		int screenH = 0;

		int invX = 0;
		int invZ = 0;

		Camera initCamera = {
			{ 0.0f, 0, -5 },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },   // mainCamera up vector (rotation towards target)
			60.,
			CAMERA_PERSPECTIVE
		};

		Stage* curStage = 0;
		int curStageId = -1;
		WCamera* curCamera = 0;
		int curCameraId = -1;
		Transform tempPose[100];

		FreelookRenderer(Resources* res, World* world) {
			this->resources = res;
			this->world = world;
		}

		~FreelookRenderer() {
			clearCamera();
			clearStage();
		}

		Vector3 GetWorldToScreenZ(Vector3 position) {
			int width = GetScreenWidth();
			int height = GetScreenHeight();
			//auto& matProj = initCamera.;
			Matrix matView = MatrixLookAt(initCamera.position, initCamera.target, initCamera.up);
			Quaternion worldPos = { position.x, position.y, position.z, 1.0f };
			worldPos = QuaternionTransform(worldPos, matView);
			//worldPos = QuaternionTransform(worldPos, matProj);
			Vector3 ndcPos = { worldPos.x / worldPos.w, -worldPos.y / worldPos.w, worldPos.z / worldPos.w };
			Vector3 screenPosition = { (ndcPos.x + 1.0f) / 2.0f * (float)width, (ndcPos.y + 1.0f) / 2.0f * (float)height, ndcPos.z };
			return screenPosition;
		}

		void clearStage()
		{
		}

		void loadStage(int newStageId)
		{
			if (newStageId == curStageId) return;
			clearStage();
			curStage = &this->resources->stages[newStageId];
			curStageId = newStageId;
			curCameraId = -1;
		}

		void clearCamera()
		{
			if (curCameraId == -1) return;
			curCameraId = -1;
		}

		void DrawBounds(Bounds bb, Color color)
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

			curCameraId = newCameraId;
			if (curCameraId == -1) return;

			curCamera = &curStage->cameras[newCameraId];
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
				int curRoomId = world->followTarget->location.roomId;
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
			int curRoomId = world->followTarget->location.roomId;
			if (curRoomId == -1) return;
			auto& room = curStage->rooms[curRoomId];
			rlPushMatrix();
			rlTranslatef(room.position.x, room.position.y, room.position.z);
			for (int z = 0; z < room.zones.size(); z++) {
				DrawBounds(room.zones[z].bounds, YELLOW);
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
			auto& p = initCamera.position;
			auto& t = initCamera.target;
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

		void render3dDebug(Vector2 screenpos, const char* label, Color color = WHITE) {
			endDebug3d->screenpos = screenpos;
			strcpy_s(endDebug3d->label, 100, label);
			endDebug3d->color = color;
			endDebug3d++;
		}

		void renderModel(GameObject& gobj)
		{
			if (gobj.modelId == -1) return;
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

		void renderObject(GameObject& gobj, Color tint)
		{
			renderModel(gobj);

			rlPushMatrix();
			Vector3& roomPos = curStage->rooms[gobj.location.roomId].position;
			rlMultMatrixf(MatrixToFloat(MatrixTranslate(roomPos.x,roomPos.y,roomPos.z)));

			DrawCube(gobj.location.position, 0.1, 0.1, 0.1, RED);
			render3dDebug(GetWorldToScreen(Vector3Add(roomPos, gobj.location.position), initCamera), to_string(gobj.id).c_str());
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
				DrawLine3D(gobj.location.position, gobj.track.targetPos, ORANGE);
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
				UpdateCamera(&initCamera, CAMERA_FREE);
			}
		}

		void render() {
			if (world->curStageId != curStageId || world->curCameraId != curCameraId) {
				loadCamera(world->curStageId, world->curCameraId);
			}

			endDebug3d = debug3d;

			/*
			for (int i = 0; i < this->world->gobjects.size(); i++) {
				auto& gobj = this->world->gobjects[i];
				if (gobj.model.id == -1) continue;
				if (gobj.location.stageId != curStageId) continue;
				renderObject(gobj, WHITE);
			}
			*/

			//TODO: Remove memory allocation
			//list<RenderOrder> renderQueue;
			for (int i = 0; i < this->world->gobjects.size(); i++) {
				auto& gobj = this->world->gobjects[i];
				if (gobj.location.stageId != curStageId) continue;
				
				Vector3 pos = gobj.location.position;
				Vector3& roomPos = curStage->rooms[gobj.location.roomId].position;

				//auto& screenPos = GetWorldToScreenZ(pos);
				//if (screenPos.z < 0) continue;
				BeginMode3D(initCamera);
				if (curCamera) rlSetMatrixProjection(curCamera->perspective);
				renderObject(gobj, WHITE);
				EndMode3D();

				string s = to_string(i);
				pos = Vector3Add(roomPos, pos);
				DrawText(s.c_str(), (int)(pos.x), (int)(pos.y), 20, WHITE);
			}
			

			//int num = 1;
			//for (auto it = renderQueue.begin(); it != renderQueue.end(); it++) {
			//	BeginMode3D(initCamera);
			//		//rlSetMatrixModelview(curCamera->modelview);
			//		rlSetMatrixProjection(curCamera->perspective);
			//		renderObject(*it->obj, WHITE);
			//	EndMode3D();

			//	//auto s = to_string(num)+" R" + to_string(it->obj->location.roomId);
			//	//auto s = to_string(it->obj->);
			//	//it->marker = s;
			//	num++;
			//}

			BeginMode3D(initCamera);
				//rlSetMatrixModelview(curCamera->modelview);
				if (curCamera) rlSetMatrixProjection(curCamera->perspective);
				DrawCube({ 0,0,0 }, 0.2, 0.2, 0.2, GREEN);
				renderBounds();
				renderZones();
				renderCameraZones();
				renderOvlBounds();
				//renderTrack();
			EndMode3D();

			for (int i = 0; i < 100; i++) {
				if (&debug3d[i] == endDebug3d) break;
				DrawText((char*)debug3d[i].label, (int)debug3d[i].screenpos.x, (int)debug3d[i].screenpos.y, 16, debug3d[i].color);
			}

			renderCamPos();
		}

	};

}