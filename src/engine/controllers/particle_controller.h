#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

namespace openAITD {

  class ParticleController {
  public:
      World& world;
      Resources& resources;

      ParticleController(World& world)
          : world(world), resources(*world.resources) {}

      void process(const float timeDelta) {
        for (auto& partGrp: world.partGroups) {
          if (!partGrp.active) continue;
          processFountain(partGrp, timeDelta); 
        }      
      }

      void processFountain(ParticleGroup& partGrp, const float timeDelta);
  };


  inline void ParticleController::processFountain(ParticleGroup& partGrp, const float timeDelta) {
      // Update group lifetime
      // partGrp.lifetime -= timeDelta;
      // if (partGrp.lifetime <= 0.0f) {
      //     partGrp.active = false;
      //     return;
      // }

      // Fountain parameters
      const float gravity = -9.8f;          // gravitational acceleration
      const float baseSpeed = 5.0f;         // base ejection speed
      const float spread = 0.5f;            // horizontal spread
      const float particleLifetime = 1.5f;  // lifetime of each particle
      const float spawnInterval = 0.1f;  // spawn every 0.2 seconds

      // 1. Update existing particles
      for (auto& p : partGrp.particles) {
          if (!p.active) continue;
          // Movement
          p.position += p.velocity * timeDelta;
          // Gravity (affects only vertical velocity)
          p.velocity.y += gravity * timeDelta;
          // Decrease particle lifetime
          p.lifetime -= timeDelta;
          if (p.lifetime <= 0.0f) {
              p.active = false;
          }
      }

      // 2. Spawn one new particle per frame
      partGrp.spawnTimer -= timeDelta;
      if (partGrp.spawnTimer <= 0.0f) {  
        partGrp.spawnTimer = spawnInterval;
        auto& p = partGrp.addParticle();
        // Start position – center of the group
        p.position = partGrp.position;
        // Random velocity
        float angleX = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * spread * 2.0f;
        float angleZ = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * spread * 2.0f;
        float speedY = baseSpeed + (static_cast<float>(rand()) / RAND_MAX) * baseSpeed * 0.5f;
        p.velocity = {angleX * baseSpeed, speedY, angleZ * baseSpeed};
        // Particle parameters
        p.lifetime = particleLifetime;
        p.color = {255, 128, 0, 255}; // orange (example)
      }
  }

}