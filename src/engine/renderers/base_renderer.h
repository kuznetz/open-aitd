#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../raylib-cpp.h"

using namespace std;
namespace openAITD {

	class BaseRenderer {
	public:
		World* world;
		Resources* resources;
		Camera mainCamera = {
			{ 0.0f, 0, -5 },
			{ 0.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },   // mainCamera up vector (rotation towards target)
			60.,
			CAMERA_PERSPECTIVE
		};
		int curStageId = -1;
		int curCameraId = -1;
		WCamera* curCamera = 0;
		Matrix perspective;

		Transform tempPose[100];

		const int getScreenW() { return resources->config.screenW; }
		const int getScreenH() { return resources->config.screenH; }

		BaseRenderer(World* world) {
			this->world = world;
			this->resources = world->resources;
		}

		~BaseRenderer() {
			clearCamera();
		}

		virtual void loadCamera(int newCameraId)
		{
			printf("Load Camera %d\n", newCameraId);
			curCameraId = newCameraId;
			curCamera = &resources->stages[world->curStageId].cameras[newCameraId];
			auto& camPers = curCamera->pers;
			perspective = MatrixPerspective(camPers.yfov, camPers.aspectRatio, camPers.znear, camPers.zfar);

			mainCamera.position = curCamera->position;
			mainCamera.target = Vector3Add(curCamera->position, Vector3Negate(Vector3RotateByQuaternion({ 0,0,1 }, curCamera->rotation)));
			mainCamera.up = Vector3RotateByQuaternion({ 0,1,0 }, curCamera->rotation);
		}

		void clearCamera()
		{
			if (curCameraId == -1) return;
			curCameraId = -1;
			curCamera = 0;
		}

		Vector3 GetWorldToScreenZ(Vector3 position) {
			const int& width = getScreenW();
			const int& height = getScreenH();
			auto& matProj = perspective;
			Matrix matView = MatrixLookAt(mainCamera.position, mainCamera.target, mainCamera.up);
			Vector3 depth = Vector3Transform(position, matView);
			Quaternion worldPos = { position.x, position.y, position.z, 1.0f };
			worldPos = QuaternionTransform(worldPos, matView);
			worldPos = QuaternionTransform(worldPos, matProj);
			Vector3 ndcPos = { worldPos.x / worldPos.w, -worldPos.y / worldPos.w, worldPos.z / worldPos.w };
			Vector3 screenPosition = { (ndcPos.x + 1.0f) / 2.0f * (float)width, (ndcPos.y + 1.0f) / 2.0f * (float)height, -depth.z };
			return screenPosition;
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

		void ProcessPose(GameObject& gobj, Model& model)
		{
			if (!model.skin || gobj.animation.id == -1) return;

			int bonesSize = model.skin->joints_count;
			if (bonesSize != gobj.animation.curPose.size()) {
				gobj.animation.curPose.resize(bonesSize);
				gobj.animation.transitionPose.resize(bonesSize);
				gobj.animation.hasPose = false;
			}

			auto& curAnim = model.animations[gobj.animation.animIdx];
			auto& newPose = curAnim.bakedPoses[gobj.animation.animFrame];

			if (gobj.animation.hasPose && gobj.animation.animChanged) {
				//Set current pose to transition start
				memcpy_s(
					gobj.animation.transitionPose.data(), gobj.animation.transitionPose.size() * sizeof(Transform),
					gobj.animation.curPose.data(), gobj.animation.curPose.size() * sizeof(Transform)
				);
			}

			Transform* curPose;
			bool isTransition = (curAnim.duration > 0) && (gobj.animation.animTime <= curAnim.transition);
			if (isTransition && gobj.animation.hasPose) {
				//newPose[0].translation = { 0,0,0 };
				model.PoseLerp(tempPose, gobj.animation.transitionPose.data(), newPose.data(), gobj.animation.animTime / curAnim.transition);
				//anim2.CalcPoseByTime(newPose, animIndex, 0);
				curPose = tempPose;
			}
			else {
				curPose = newPose.data();
				//curPose[0].translation = { 0,0,0 };
				//anim2.CalcPoseByTime(curPose, animIndex, animTime);
			}
			model.ApplyPose(curPose);
			model.UpdateBuffer();
			memcpy_s(
				gobj.animation.curPose.data(), gobj.animation.curPose.size() * sizeof(Transform),
				curPose, newPose.size() * sizeof(Transform)
			);
			if (!gobj.animation.hasPose) {
				//Init transition start pose
				memcpy_s(
					gobj.animation.transitionPose.data(), gobj.animation.transitionPose.size() * sizeof(Transform),
					curPose, newPose.size() * sizeof(Transform)
				);
				gobj.animation.hasPose = true;
			}
		}

		void renderObject(GameObject& gobj, Color tint)
		{
			auto rmodel = resources->models.getModel(gobj.modelId);
			auto& model = rmodel->model;

			//if (model.skin && gobj.animation.id != -1) {
			//	ProcessPose(gobj, model);
			//}

			Vector3 pos = gobj.location.position;
			Vector3& roomPos = world->curStage->rooms[gobj.location.roomId].position;
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


		void renderMessage() {
			if (world->messageTime > 0) {
				auto& f = resources->screen.mainFont;
				const char* m = world->messageText.c_str();
				auto mt = MeasureTextEx(f, m, f.baseSize, 0);
				Vector2 v = { (int)(getScreenW() - mt.x) / 2, getScreenH() - (f.baseSize * 2) };
				DrawTextEx(f, m, v, f.baseSize, 0, WHITE);
			}
		}

	};

}
