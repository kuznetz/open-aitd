#pragma once
#include <vector>
#include "common/raylib_cpp.hpp"
#include "engine/world/particles.h"

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

      // Custom implementation of DrawBillboard.
      // Unlike raylib::DrawBillboard (which is "locked on axis-Y"),
      // the quad is oriented using the passed screen-space camera basis
      // (right/up), so it always faces exactly toward the camera.
      void MyDrawBillboard(const Texture2D& texture,
                           const Vector3& position, float size, Color color,
                           const Vector3& right, const Vector3& up) {
          float half = size * 0.5f;
          Vector3 p1 = Vector3Add(position, Vector3Add(Vector3Scale(right, -half), Vector3Scale(up, -half)));
          Vector3 p2 = Vector3Add(position, Vector3Add(Vector3Scale(right,  half), Vector3Scale(up, -half)));
          Vector3 p3 = Vector3Add(position, Vector3Add(Vector3Scale(right,  half), Vector3Scale(up,  half)));
          Vector3 p4 = Vector3Add(position, Vector3Add(Vector3Scale(right, -half), Vector3Scale(up,  half)));

          rlSetTexture(texture.id);
          rlBegin(RL_QUADS);
              rlColor4ub(color.r, color.g, color.b, color.a);
              rlTexCoord2f(0.0f, 0.0f); rlVertex3f(p1.x, p1.y, p1.z);
              rlTexCoord2f(1.0f, 0.0f); rlVertex3f(p2.x, p2.y, p2.z);
              rlTexCoord2f(1.0f, 1.0f); rlVertex3f(p3.x, p3.y, p3.z);
              rlTexCoord2f(0.0f, 1.0f); rlVertex3f(p4.x, p4.y, p4.z);
          rlEnd();
          rlSetTexture(0);
      }

  public:
      ParticleRenderer(World& world) : circleTexture(), world(world) {}

      ~ParticleRenderer() {
        if (circleTexture.id != 0) {
          UnloadTexture(circleTexture);
        }
      }

      void render(const ParticleGroup& group) {
          if (circleTexture.id == 0) {
              createCircleTexture();
          }

          const Vector3& roomPos = world.curStage->rooms[group.roomId].origPosition;

          rlSetMatrixModelview(world.cameraView);
          rlSetMatrixProjection(world.cameraProjection);

          // We take the camera's screen basis directly from the current model-view
          // matrix (it is already set above): row 0 is right, row 1 is up.
          // This guarantees that the billboard is oriented exactly the same way the
          // camera looks (including any roll/pitch), and does not depend on camera.up.
          const Matrix& view = world.cameraView;
          Vector3 right = Vector3Normalize({ view.m0, view.m4, view.m8 });
          Vector3 up    = Vector3Normalize({ view.m1, view.m5, view.m9 });

          BeginBlendMode(BLEND_ALPHA);
          Vector3 pos = Vector3Add(roomPos, group.position);
          //MyDrawBillboard(circleTexture, pos, 0.1f, RED, right, up); //center debug
          for (const auto& p : group.particles) {
              if (!p.active) continue;
              pos = Vector3Add(roomPos, p.position);
              MyDrawBillboard(circleTexture, pos, p.size, p.color, right, up);
          }
          EndBlendMode();

      }
  };
}