#pragma once
#include <vector>
#include "../../common/raylib_cpp.hpp"

using namespace std;
using namespace raylib;

namespace openAITD {

  class Particle {
  public:
    Vector3 position;
    Color color;
    float size;
    float lifetime;
  };

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

      void init() {
          if (circleTexture.id == 0) {
              createCircleTexture();
          }
      }

      void render(const vector<Particle>& particles, const Camera3D& camera) {
          if (particles.empty() || circleTexture.id == 0) return;
          BeginBlendMode(BLEND_ALPHA);
          for (const auto& p : particles) {
              DrawBillboard(camera, circleTexture, p.position, p.size, p.color);
          }
          EndBlendMode();
      }
  };

}