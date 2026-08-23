#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

namespace openAITD {

  class ShakeController {
  public:
    World& world;
    Resources& resources;

    ShakeController(World& world)
        : world(world), resources(*world.resources) {}

    void process(const float timeDelta) {
        auto& shake = world.shake;
        const float screenH = (float)world.resources->config.screenH;

        shake.time += timeDelta;

        // Relative
        if (shake.active && !shake.fadingOut) {
            shake.intensity += (shake.maxIntensity / shake.fadeInTime) * timeDelta;
            if (shake.intensity > shake.maxIntensity)
                shake.intensity = shake.maxIntensity;
        } else if (shake.fadingOut) {
            shake.intensity -= (shake.maxIntensity / shake.fadeOutTime) * timeDelta;
            if (shake.intensity <= 0.0f) {
                shake.intensity = 0.0f;
                shake.fadingOut = false;
                shake.active = false;
            }
        }

        // In pixels
        if (shake.intensity > 0.001f) {
            float t = shake.time;
            float ampPixels = shake.intensity * screenH;  // переводим в пиксели
            shake.offsetX = (sinf(t * 43.7f) * 0.6f + sinf(t * 71.3f + 1.2f) * 0.4f) * ampPixels;
            shake.offsetY = (cosf(t * 57.1f) * 0.6f + cosf(t * 83.9f + 2.3f) * 0.4f) * ampPixels;
        } else {
            shake.offsetX = 0.0f;
            shake.offsetY = 0.0f;
        }
    }

  };

}