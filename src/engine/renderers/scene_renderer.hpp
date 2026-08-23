#pragma once
#include <vector>
#include "../../common/raylib_cpp.hpp"
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;
using namespace raylib;

namespace openAITD {

  class SceneRenderer {
  private:
      World& world;
      Resources& resources;

  public:

      Vector3 brightnessFactor = { 1.0f, 1.0f, 1.0f };
      Shader brightnessShader = { 0 };
      int shUniformLoc = 0;
        
      SceneRenderer(World& world) : world(world), resources(*world.resources) {}

      void init() {
          brightnessShader = LoadShader(
            "newdata/shaders/glsl330/brightness.vs",
            "newdata/shaders/glsl330/brightness.fs"
          );
          shUniformLoc = GetShaderLocation(brightnessShader, "brightness");
          SetShaderValue(brightnessShader, shUniformLoc, &brightnessFactor, SHADER_UNIFORM_VEC3);
      }

      ~SceneRenderer() {
          if (brightnessShader.id) {
            UnloadShader(brightnessShader);
          }          
      }

      void render(float brightness = 1) {
          auto& c = resources.config;
          float offX = world.shake.offsetX;
          float offY = world.shake.offsetY;

          if (brightness != 1) {
              BeginShaderMode(brightnessShader);
              brightnessFactor = { brightness, brightness, brightness };
              SetShaderValue(brightnessShader, shUniformLoc, &brightnessFactor, SHADER_UNIFORM_VEC3);
              DrawTextureRec(resources.screen.sceneTex.texture,
                            { 0, 0, (float)c.screenW, (float)-c.screenH },
                            { offX, offY },
                            WHITE);
              EndShaderMode();
          } else {
              DrawTextureRec(resources.screen.sceneTex.texture,
                            { 0, 0, (float)c.screenW, (float)-c.screenH },
                            { offX, offY },
                            WHITE);
          }
      }

  };

}