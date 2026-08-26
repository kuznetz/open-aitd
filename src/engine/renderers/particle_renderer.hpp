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

      void render(const ParticleGroup& group) {
          if (circleTexture.id == 0) {
              createCircleTexture();
          }

          auto& curCamera = world.curCamera;
          Camera3D mainCamera;
          mainCamera.position = curCamera->position;
          mainCamera.target = Vector3Add(curCamera->position, Vector3Negate(Vector3RotateByQuaternion({ 0,0,1 }, curCamera->rotation)));
          mainCamera.up = Vector3RotateByQuaternion({ 0,1,0 }, curCamera->rotation);   

          const Vector3& pos = group.position;
          const Vector3& roomPos = world.curStage->rooms[group.roomId].origPosition;

          Matrix matr = MatrixIdentity();
          matr = MatrixMultiply(MatrixTranslate(roomPos.x, roomPos.y, roomPos.z), matr);
          auto m = world.cameraView;
          rlSetMatrixModelview(MatrixMultiply(m, matr));
  				rlSetMatrixProjection(world.cameraProjection);
				          
          BeginBlendMode(BLEND_ALPHA);
          //DrawBillboard(camera, circleTexture, group.position, 0.1f, {255,255,255,255}); //center
          for (const auto& p : group.particles) {
              if (!p.active) continue;
              DrawBillboard(mainCamera, circleTexture, p.position, p.size, p.color);
          }
          EndBlendMode();

    			rlSetMatrixModelview(m);
      }

  };

}