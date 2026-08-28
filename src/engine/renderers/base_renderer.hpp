#pragma once
#include <vector>
#include <string>
#include "engine/world/world.h"
#include "engine/resources/resources.h"
#include "common/raylib_cpp.hpp"

using namespace std;
using namespace raylib;
namespace openAITD {

	class BaseRenderer {
	public:
		World* world;
		Resources* resources;

		int curStageId = -1;
		int curCameraId = -1;
		WCamera* curCamera = 0;

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
		}

		void clearCamera()
		{
			if (curCameraId == -1) return;
			curCameraId = -1;
			curCamera = 0;
		}

		void boundsToScreen(const Bounds& bb, Rectangle& rect, float& zPos) {
				// 8 corners of the AABB: bottom face (y = min) then top face (y = max)
				Vector3 corners[8];
				// Bottom face
				corners[0] = { bb.min.x, bb.min.y, bb.max.z }; // front-left
				corners[1] = { bb.max.x, bb.min.y, bb.max.z }; // front-right
				corners[2] = { bb.min.x, bb.min.y, bb.min.z }; // back-left
				corners[3] = { bb.max.x, bb.min.y, bb.min.z }; // back-right
				// Top face
				corners[4] = { bb.min.x, bb.max.y, bb.max.z }; // front-left
				corners[5] = { bb.max.x, bb.max.y, bb.max.z }; // front-right
				corners[6] = { bb.min.x, bb.max.y, bb.min.z }; // back-left
				corners[7] = { bb.max.x, bb.max.y, bb.min.z }; // back-right

				// Project the first corner to initialize extremes
				Vector3 first = world->WorldToScreenZ(corners[0]);
				float minX = first.x, maxX = first.x;
				float minY = first.y, maxY = first.y;
				float maxZ = first.z;

				// Process the remaining 7 corners
				for (int i = 1; i < 8; ++i) {
						Vector3 v = world->WorldToScreenZ(corners[i]);
						// Update 2D bounds
						if (v.x < minX) minX = v.x;
						if (v.x > maxX) maxX = v.x;
						if (v.y < minY) minY = v.y;
						if (v.y > maxY) maxY = v.y;
						// Keep the farthest depth (largest Z, assuming Z increases away from camera)
						if (v.z > maxZ) maxZ = v.z;
				}

				// Fill the output rectangle (top-left corner and size)
				rect.x = minX;
				rect.y = minY;
				rect.width  = maxX - minX;
				rect.height = maxY - minY;

				// Return the maximum depth for later sorting
				zPos = maxZ;
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

		void processSkin(GameObject& gobj, Model& model)
		{
			if (!model.skin || gobj.animation.id == -1) return;

			if (gobj.animation.curPose) {
				model.ApplyPose(gobj.animation.curPose);
				model.UpdateBuffer();
			}
		}

	};

}
