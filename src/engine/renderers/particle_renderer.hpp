#pragma once
#include <vector>
#include "../../common/raylib_cpp.hpp"
#include "../world/particles.h"

using namespace std;
using namespace raylib;

namespace openAITD {

  class ParticleRenderer {
  private:
      const int textureSize = 128;
      Texture2D circleTexture;

      void createCircleTexture() {
        Image circleImage = GenImageColor(textureSize, textureSize, BLANK);
        ImageDrawCircle(&circleImage, textureSize/2, textureSize/2, textureSize/2 - 4, WHITE);
        circleTexture = LoadTextureFromImage(circleImage);
        UnloadImage(circleImage);
      }

  public:
      ParticleRenderer() : circleTexture() {}

      ~ParticleRenderer() {
        if (circleTexture.id != 0) {
          UnloadTexture(circleTexture);
        }
      }

      void render(const ParticleGroup& group, const Camera3D& camera) {
          if (circleTexture.id == 0) {
              createCircleTexture();
          }
          if (!group.active || group.particles.empty() || circleTexture.id == 0) return;
          BeginBlendMode(BLEND_ALPHA);
          for (const auto& p : group.particles) {
              if (!p.active) return;
              DrawBillboard(camera, circleTexture, p.position, p.size, p.color);
          }
          EndBlendMode();
      }

  };

}