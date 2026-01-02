#pragma once
#include <vector>
#include <map>
#include <string>

#include "../raylib-cpp.h"
#include "config.h"

using namespace std;
using namespace raylib;
namespace openAITD {

	class Screen
	{
	public:
		string fontPath = "newdata/font.ttf";
		Config* config = 0;
		Font mainFont = { 0 };
		RenderTexture screenTex;

		Screen() {
		}

		~Screen() {
			UnloadFont(mainFont);
			UnloadRenderTexture(screenTex);
		}

		void begin() {
			BeginTextureMode(screenTex);
			ClearBackground(BLACK);
		}

		void end() {
			EndTextureMode();
			finalRender(screenTex.texture);
		}

		void finalRender(const Texture texture) {
			BeginDrawing();
			auto& c = *config;
			DrawTextureRec(texture, { 0, 0, (float)c.screenW, (float)-c.screenH }, { (float)c.screenX, (float)c.screenY }, WHITE);
			if (c.showFps) {
				DrawFPS(c.screenX + 10, 10);
			}
			EndDrawing();
		}

		void drawCentered(const char* text, raylib::Rectangle r, Color color) {
			auto& f = this->mainFont;
			auto mt = MeasureTextEx(f, text, f.baseSize, 0);
			int x = (int)(r.x + ((r.width - mt.x) / 2));
			Vector2 v = { x, (int)r.y };
			raylib::DrawTextEx(f, text, v, f.baseSize, 0, color);
		}

		void init() {
			mainFont = LoadFontEx(fontPath.c_str(), 16 * config->screenH / 200, 0, 95);
			screenTex = LoadRenderTexture(config->screenW, config->screenH);
		}

	private:

	};

}