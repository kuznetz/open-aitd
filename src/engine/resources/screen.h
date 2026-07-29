#pragma once
#include <vector>
#include <map>
#include <string>

#include "../../raylib-cpp/raylib-cpp.h"
#include "config.h"

using namespace std;
using namespace raylib;
namespace openAITD {

    class Screen
    {
    public:
        string fontPath = "newdata/font.ttf";
        Config& config;
        Font mainFont = { 0 };
        RenderTexture sceneTex;

				Vector3 brightnessFactor = { 1.0f, 1.0f, 1.0f };
				Shader brightnessShader = { 0 };
				int shUniformLoc = 0;
				
        Screen(Config& config):
				 config(config)
				 {}

        ~Screen() {
            UnloadFont(mainFont);
        }

        void init() {
            mainFont = LoadFontEx(fontPath.c_str(), 16 * config.screenH / 200, 0, 95);
						sceneTex = LoadRenderTexture(config.screenW, config.screenH);

						brightnessShader = LoadShader(
							"newdata/shaders/glsl330/brightness.vs",
							"newdata/shaders/glsl330/brightness.fs"
						);
					  shUniformLoc = GetShaderLocation(brightnessShader, "brightness");
            SetShaderValue(brightnessShader, shUniformLoc, &brightnessFactor, SHADER_UNIFORM_VEC3);
        }

        void renderScene(float brightness = 1) {
						auto& c = config;
					  if (brightness != 1) {
							BeginShaderMode(brightnessShader);
							brightnessFactor = {brightness,brightness,brightness};
							SetShaderValue(brightnessShader, shUniformLoc, &brightnessFactor, SHADER_UNIFORM_VEC3);
							DrawTextureRec(sceneTex.texture,
														{ 0, 0, (float)c.screenW, (float)-c.screenH },
														{ 0, 0 },
														WHITE);
							EndShaderMode();
						} else {
							DrawTextureRec(sceneTex.texture, { 0, 0, (float)c.screenW, (float)-c.screenH }, { 0,0 }, WHITE);
						}
        }

				void begin() {
						BeginDrawing();
						ClearBackground(BLACK);
						resetViewport();
				}

				void resetViewport() {
				    rlMatrixMode(RL_PROJECTION);
    				rlLoadIdentity();
    				rlOrtho(0, config.screenW, config.screenH, 0, -1.0f, 1.0f);

				    rlMatrixMode(RL_MODELVIEW);
    				rlLoadIdentity();

				    rlViewport(config.screenX, config.screenY, config.screenW, config.screenH);
				}

				void end() {
						if (config.showFps) {
								DrawFPS(10, 10);
						}
						EndDrawing();
				}

        void drawLeft(const char* text, raylib::Rectangle r, Color color) {
            auto& f = this->mainFont;
            Vector2 v = { r.x, r.y };
            DrawTextEx(f, text, v, f.baseSize, 0, color);
        }

        void drawCentered(const char* text, raylib::Rectangle r, Color color) {
            auto& f = this->mainFont;
            Vector2 mt = MeasureTextEx(f, text, f.baseSize, 0);
            int x = (int)(r.x + ((r.width - mt.x) / 2));
            Vector2 v = { (float)x, r.y };
            DrawTextEx(f, text, v, f.baseSize, 0, color);
        }

    private:
    };
}