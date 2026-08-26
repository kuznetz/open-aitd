#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../../common/raylib_cpp.hpp"

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

		Vector3 GetWorldToScreenZ(Vector3 position) {
				// Get screen dimensions (in pixels)
				const int width  = getScreenW();
				const int height = getScreenH();

				// Retrieve the view and projection matrices from the camera
				const Matrix& view = world->cameraView;
				const Matrix& proj = world->cameraProjection;

				// ---- Helper functions to multiply a vector by a 4x4 matrix ----
				// Multiply a 3D vector (with w implicitly = 1) by a 4x4 matrix, returning a 4D vector
				auto multiply3 = [](Vector3 v, const Matrix& m) -> Vector4 {
						return {
								v.x * m.m0 + v.y * m.m4 + v.z * m.m8  + 1.0f * m.m12,
								v.x * m.m1 + v.y * m.m5 + v.z * m.m9  + 1.0f * m.m13,
								v.x * m.m2 + v.y * m.m6 + v.z * m.m10 + 1.0f * m.m14,
								v.x * m.m3 + v.y * m.m7 + v.z * m.m11 + 1.0f * m.m15
						};
				};

				// Multiply a 4D vector by a 4x4 matrix
				auto multiply4 = [](const Vector4& v, const Matrix& m) -> Vector4 {
						return {
								v.x * m.m0 + v.y * m.m4 + v.z * m.m8  + v.w * m.m12,
								v.x * m.m1 + v.y * m.m5 + v.z * m.m9  + v.w * m.m13,
								v.x * m.m2 + v.y * m.m6 + v.z * m.m10 + v.w * m.m14,
								v.x * m.m3 + v.y * m.m7 + v.z * m.m11 + v.w * m.m15
						};
				};
				// ----------------------------------------------------------

				// 1. Transform world position to view space (camera space)
				Vector4 viewPos = multiply3(position, view);

				// 2. Transform view-space position to clip space (homogeneous coordinates)
				Vector4 clipPos = multiply4(viewPos, proj);

				// 3. Perspective division: convert from clip space to normalized device coordinates (NDC)
				if (clipPos.w != 0.0f) {
						clipPos.x /= clipPos.w;
						clipPos.y /= clipPos.w;
						clipPos.z /= clipPos.w;
				}

				// 4. NDC → screen coordinates
				//    NDC x,y in [-1,1] map to [0,width] and [0,height].
				//    In raylib, the screen Y axis points downward, so we flip the Y coordinate.
				float screenX = (clipPos.x + 1.0f) * 0.5f * width;
				float screenY = (1.0f - (clipPos.y + 1.0f) * 0.5f) * height;
				float screenZ = clipPos.z;   // depth in NDC range [-1,1], useful for sorting

				return { screenX, screenY, screenZ };
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
				Vector3 first = GetWorldToScreenZ(corners[0]);
				float minX = first.x, maxX = first.x;
				float minY = first.y, maxY = first.y;
				float maxZ = first.z;

				// Process the remaining 7 corners
				for (int i = 1; i < 8; ++i) {
						Vector3 v = GetWorldToScreenZ(corners[i]);
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
