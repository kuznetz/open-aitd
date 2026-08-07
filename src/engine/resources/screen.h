#pragma once
#include <vector>
#include <map>
#include <string>

#include "../../common/raylib_cpp.hpp"
#include "config.h"

using namespace std;
using namespace raylib;
namespace openAITD {

    class Screen
    {
    public:
        Config& config;
        RenderTexture sceneTex;
				bool fullscreen = false;
				bool initialized = false;

				Vector3 brightnessFactor = { 1.0f, 1.0f, 1.0f };
				Shader brightnessShader = { 0 };
				int shUniformLoc = 0;
				
        Screen(Config& config):
				 config(config)
				{
				}

        ~Screen() {
            if (!initialized) return;
            UnloadRenderTexture(sceneTex);
            UnloadShader(brightnessShader);
        }

        void init() {
						InitWindow(config.screenW, config.screenH, "Open-AITD");
						int m = GetCurrentMonitor();
						int monitorW = GetMonitorWidth(m);
						int monitorH = GetMonitorHeight(m);
						fullscreen = config.fulllscreen;
						if (fullscreen) {
								config.screenW = monitorH * 4 / 3;
								config.screenH = monitorH;
								config.screenX = (monitorW - config.screenW) / 2;
								config.screenY = 0;
								ToggleBorderlessWindowed();
								SetWindowSize(monitorW, monitorH);
						}
						else {
							SetWindowPosition(
									(monitorW - config.screenW) / 2,
									(monitorH - config.screenH) / 2
							);
						}
						config.targetFps = GetMonitorRefreshRate(m);
						SetWindowState(FLAG_VSYNC_HINT);
						SetTargetFPS(config.targetFps);					

						sceneTex = LoadRenderTexture(config.screenW, config.screenH);

						brightnessShader = LoadShader(
							"newdata/shaders/glsl330/brightness.vs",
							"newdata/shaders/glsl330/brightness.fs"
						);
					  shUniformLoc = GetShaderLocation(brightnessShader, "brightness");
            SetShaderValue(brightnessShader, shUniformLoc, &brightnessFactor, SHADER_UNIFORM_VEC3);
						
						initialized = true;
        }

				void reinit() {
						if (!initialized) {
								init();
								return;
						}

						int m = GetCurrentMonitor();
						int monitorW = GetMonitorWidth(m);
						int monitorH = GetMonitorHeight(m);

						if (config.fulllscreen) {
								if (!fullscreen) ToggleBorderlessWindowed();
								config.screenW = monitorH * 4 / 3;
								config.screenH = monitorH;
								config.screenX = (monitorW - config.screenW) / 2;
								config.screenY = 0;
								SetWindowSize(monitorW, monitorH);
								SetWindowPosition(0, 0);
						} else {
								if (fullscreen) ToggleBorderlessWindowed();
								config.screenX = (monitorW - config.screenW) / 2;
								config.screenY = (monitorH - config.screenH) / 2;
								SetWindowSize(config.screenW, config.screenH);
								SetWindowPosition(config.screenX, config.screenY);
						}

						UnloadRenderTexture(sceneTex);
						sceneTex = LoadRenderTexture(config.screenW, config.screenH);
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

				void saveScreenshot(const string& path, int outWidth = 0, int outHeight = 0) {
						Image img = LoadImageFromTexture(sceneTex.texture);
						ImageFlipVertical(&img);
						if (outWidth > 0 && outHeight > 0) {
								ImageResize(&img, outWidth, outHeight);
						}
						ExportImage(img, path.c_str());
						UnloadImage(img);
				}

    private:
    };
}