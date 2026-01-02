#pragma once
#include <vector>
#include <string>
#include "../raylib-cpp.h"
#include "../world/world.h"
#include "../resources/resources.h"

using namespace std;
using namespace raylib;
namespace openAITD {

	class FoundScreen {
	public:
		World* world;
		Resources* resources;
		bool leave = false;
		int foundItem = -1;
		Camera3D modelCamera = {
				{ 0, 2.5, -5 },      // position
				{ 0, 0, 0 },        // target  
				{ 0, 1, 0 },        // up
				50.0f,              // fovy
				CAMERA_PERSPECTIVE  // projection
		};
		float modelRotate = 0;
		RenderTexture renderTexture;

		FoundScreen(World* world) {
			this->world = world;
			this->resources = world->resources;
		}

		~FoundScreen() {
		}

		void submit() {
			auto& gobj = this->world->gobjects[foundItem];
			if (leave) {
				gobj.invItem.foundTimeout = world->chrono + 5;
				foundItem = -1;
				return;
			}
			world->take(foundItem);
			foundItem = -1;
		}

		void process(float timeDelta) {
			modelRotate += timeDelta * 180;
			if (IsKeyPressed(KEY_LEFT)) {
				leave = true;
			}
			if (IsKeyPressed(KEY_RIGHT)) {
				leave = false;
			}
			if (IsKeyPressed(KEY_ESCAPE)) {
				leave = true;
				submit();
				return;
			}
			if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
				submit();
				return;
			}
		}

		void renderModel() {
			auto& gobj = this->world->gobjects[foundItem];
			RModel* rmodel = resources->models.getModel(gobj.invItem.modelId);

			BeginTextureMode(renderTexture);
			ClearBackground(BLACK);
			BeginMode3D(modelCamera);
			//const auto& screenW = this->resources->config.screenW;
			//const auto& screenH = this->resources->config.screenH;
			//rlViewport(	0, screenH * 0.05, screenW, screenH * 0.9	);
			rlMatrixMode(RL_MODELVIEW);
			rlRotatef(modelRotate, 0, 1, 0);
			if (rmodel) {
				rmodel->model.Render();
			}
			EndMode3D();
			EndTextureMode();
		}

		void DrawRenderTex(const Texture2D texture) {
			float screenW = (float)this->resources->config.screenW;
			float screenH = (float)this->resources->config.screenH;
			rlSetTexture(texture.id);
			rlColor4f(1, 1, 1, 1);
			rlBegin(RL_QUADS);
			// Top-left corner for texture and quad
			rlTexCoord2f(0, 1);
			rlVertex2f(0, 0);
			// Bottom-left corner for texture and quad
			rlTexCoord2f(0, 0);
			rlVertex2f(0, screenH);
			// Bottom-right corner for texture and quad
			rlTexCoord2f(1, 0);
			rlVertex2f(screenW, screenH);
			// Top-right corner for texture and quad
			rlTexCoord2f(1, 1);
			rlVertex2f(screenW, 0);
			rlEnd();
	  }

		void render() {
			const auto& screenW = this->resources->config.screenW;
			const auto& screenH = this->resources->config.screenH;

			DrawRenderTex(renderTexture.texture);

			auto& gobj = this->world->gobjects[foundItem];
			auto& name = resources->texts[gobj.invItem.nameId];

			auto& f = resources->screen.mainFont;
			const char* m = "New item:";
			raylib::Rectangle r = { 0, screenH * 0.05, screenW, 0 };
			resources->screen.drawCentered("New item:", r, WHITE);
			r.y += f.baseSize;
			resources->screen.drawCentered(name.c_str(), r, GOLD);

			r = { (screenW / 4.f), (screenH * 0.95f) - f.baseSize, (screenW / 4.f), 0 };
			resources->screen.drawCentered("Leave", r, leave ? YELLOW : GRAY);
			r = { r.x + r.width, r.y, r.width, 0 };
			resources->screen.drawCentered("Take", r, leave ? GRAY : YELLOW);
		}

		void main(int newFoundItem) {
			auto aa = resources->config.antialiasing;
			renderTexture = LoadRenderTexture(
				resources->config.screenW * aa,
				resources->config.screenH * aa
			);
			SetTextureFilter(
				renderTexture.texture,
				(aa == 1.0)? TEXTURE_FILTER_POINT: TEXTURE_FILTER_BILINEAR
			);			

			bool firstFrame = true;
			foundItem = newFoundItem;
			float timeDelta;
			while (foundItem != -1) {
				if (firstFrame) {
					firstFrame = false;
				}
				else {
  				timeDelta = GetFrameTime();
					process(timeDelta);
				}
				if (foundItem == -1) break;

				renderModel();
				resources->screen.begin();
				render();				
  			resources->screen.end();
			}

			UnloadRenderTexture(renderTexture);
		}

	};

}