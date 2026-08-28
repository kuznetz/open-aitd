#pragma once
#include <vector>
#include "common/raylib_cpp.hpp"
#include "world/particles.h"

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

      // Собственная реализация DrawBillboard
      void MyDrawBillboard(const Camera3D& camera, const Texture2D& texture,
                               const Vector3& position, float size, Color color) {
          // Вычисляем векторы камеры
          Vector3 forward = Vector3Normalize(Vector3Subtract(camera.position, position));
          Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
          Vector3 up = Vector3Normalize(Vector3CrossProduct(right, forward));

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

          auto& curCamera = world.curCamera;
          Camera3D mainCamera;
          mainCamera.position = curCamera->position;
          mainCamera.target = Vector3Add(curCamera->position, Vector3Negate(Vector3RotateByQuaternion({ 0,0,1 }, curCamera->rotation)));
          mainCamera.up = Vector3RotateByQuaternion({ 0,1,0 }, curCamera->rotation);

          const Vector3& roomPos = world.curStage->rooms[group.roomId].origPosition;

          rlSetMatrixModelview(world.cameraView);
          rlSetMatrixProjection(world.cameraProjection);

          BeginBlendMode(BLEND_ALPHA);
          Vector3 pos = Vector3Add(roomPos, group.position);
          DrawBillboard(mainCamera, circleTexture, pos, 0.1f, RED);
          for (const auto& p : group.particles) {
              if (!p.active) continue;
              pos = Vector3Add(roomPos, p.position);
              DrawBillboard(mainCamera, circleTexture, pos, p.size, p.color);
          }
          EndBlendMode();

      }
  };
}