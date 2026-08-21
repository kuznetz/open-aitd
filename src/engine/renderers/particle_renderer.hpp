#pragma once
#include <vector>
#include "../../common/raylib_cpp.hpp"
#include "../world/particles.h"

using namespace std;
using namespace raylib;

namespace openAITD {

  class ParticleRenderer {
  private:
      World& world;
      const int textureSize = 128;
      Texture2D circleTexture;

      void createCircleTexture() {
        Image circleImage = GenImageColor(textureSize, textureSize, BLANK);
        ImageDrawCircle(&circleImage, textureSize/2, textureSize/2, textureSize/2 - 4, WHITE);
        circleTexture = LoadTextureFromImage(circleImage);
        UnloadImage(circleImage);
      }

  public:
      ParticleRenderer(World& world) : circleTexture(), world(world) {}

      ~ParticleRenderer() {
        if (circleTexture.id != 0) {
          UnloadTexture(circleTexture);
        }
      }

      void render(const ParticleGroup& group, const Camera3D& camera) {
          if (circleTexture.id == 0) {
              createCircleTexture();
          }

          const Vector3& pos = group.position;
          const Vector3& roomPos = world.curStage->rooms[group.roomId].origPosition;

          Matrix matr = MatrixIdentity();
          matr = MatrixMultiply(MatrixTranslate(roomPos.x, roomPos.y, roomPos.z), matr);
          auto m = rlGetMatrixModelview();
          rlSetMatrixModelview(MatrixMultiply(m, matr));

          BeginBlendMode(BLEND_ALPHA);
          //DrawBillboard(camera, circleTexture, group.position, 0.1f, {255,255,255,255}); //center
          for (const auto& p : group.particles) {
              if (!p.active) continue;
              DrawBillboard(camera, circleTexture, p.position, p.size, p.color);
          }
          EndBlendMode();

    			rlSetMatrixModelview(m);
      }

  };

}