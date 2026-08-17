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
        for (auto& partGrp: world.partGroups.groups) {
          if (!partGrp.active) continue;
          if (partGrp.type == 1) {
            processRicochet(partGrp, timeDelta);
          } else if (partGrp.type == 2) {
            processSmoke(partGrp, timeDelta);
          } else {
            processFountain(partGrp, timeDelta); 
          }
        }      
      }

      void processFountain(ParticleGroup& partGrp, const float timeDelta);
      void processRicochet(ParticleGroup& partGrp, const float timeDelta);
      void processSmoke(ParticleGroup& partGrp, const float timeDelta);
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
      const float baseSpeed = 4.0f;         // base ejection speed
      const float spread = 0.3f;            // horizontal spread
      const float particleLifetime = 1.5f;  // lifetime of each particle
      const float spawnInterval = 0.1f;  // spawn every 0.2 seconds

      // 1. Update existing particles
      for (auto& p : partGrp.particles) {
          if (!p.active) continue;
          // Movement
          p.position += p.velocity * timeDelta;

          if (p.position.y > 0) {
            // Gravity (affects only vertical velocity)
            p.velocity.y += gravity * timeDelta;
          } else {
            p.velocity.y = 0;
            p.position.y = 0;
          }

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

  inline void ParticleController::processRicochet(ParticleGroup& partGrp, const float timeDelta) {
      // Fountain parameters
      const float gravity = -9.8f;          // gravitational acceleration
      const float baseSpeed = 2.5f;         // base ejection speed
      const float spread = 0.5f;            // horizontal spread
      const float particleLifetime = 1.0f;  // lifetime of each particle
      const float spawnInterval = 0.1f;  // spawn every 0.2 seconds

      if (!partGrp.initialized) {
        for (int i=0; i<10; i++) {
          auto& p = partGrp.addParticle();
          p.position = partGrp.position;
          float angleX = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * spread * 2.0f;
          float angleZ = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * spread * 2.0f;
          float speedY = baseSpeed + (static_cast<float>(rand()) / RAND_MAX) * baseSpeed * 0.5f;
          p.velocity = {angleX * baseSpeed, speedY, angleZ * baseSpeed};
          p.lifetime = particleLifetime + ((static_cast<float>(rand()) / RAND_MAX - 0.5f) * (0.5f));
          p.color = {255, 255, 255, 255};
        }
        partGrp.initialized = true;
      }

      bool allInactive = true;
      for (auto& p : partGrp.particles) {
          if (!p.active) continue;
          allInactive = false;
          
          // Movement
          p.position += p.velocity * timeDelta;
          if (p.position.y > 0) {
            // Gravity (affects only vertical velocity)
            p.velocity.y += gravity * timeDelta;
          } else {
            p.velocity.y = 0;
            p.position.y = 0;
          }

          // Decrease particle lifetime
          p.lifetime -= timeDelta;
          if (p.lifetime <= 0.0f) {
              p.active = false;
          }
      }

      if (allInactive) {
          partGrp.active = false;
      }
  }

  inline void ParticleController::processSmoke(ParticleGroup& partGrp, const float timeDelta) {
      // Smoke parameters
      const float riseSpeed = 1.5f;            // base rise speed
      const float spread = 0.8f;               // horizontal spread
      const float turbulence = 1.2f;           // amplitude of random deviations
      const float particleLifetime = 1.5f;     // lifetime of each particle
      const float spawnInterval = 0.05f;       // interval for spawning new particles

      if (!partGrp.initialized) {
          partGrp.lifetime = 2.f;
          partGrp.initialized = true;
      }

      // 1. Update existing particles
      for (auto& p : partGrp.particles) {
          if (!p.active) continue;

          // Movement: upward with slight random horizontal drift
          p.position += p.velocity * timeDelta;

          // Turbulence: random velocity change (vortex effect)
          float turbX = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * turbulence * timeDelta;
          float turbZ = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * turbulence * timeDelta;
          p.velocity.x += turbX;
          p.velocity.z += turbZ;

          // Gradual slowdown of the ascent (can add weak drag)
          p.velocity.y *= (1.0f - 0.2f * timeDelta); // slight damping

          // Decrease lifetime
          p.lifetime -= timeDelta;
          if (p.lifetime <= 0.0f) {
              p.active = false;
          } else {
              float lifeRatio = p.lifetime / particleLifetime; // from 1 to 0
              p.size = 0.25f * lifeRatio;
          }
      }

      // 2. Spawn new particles
      partGrp.spawnTimer -= timeDelta;
      partGrp.lifetime -= timeDelta;
      if (partGrp.lifetime > 0 && partGrp.spawnTimer <= 0.0f) {
          partGrp.spawnTimer = spawnInterval;
          auto& p = partGrp.addParticle();
          p.size = 0.25f;

          // Starting position – center of the group with a slight random offset
          p.position = partGrp.position;
          p.position.x += (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.5f;
          p.position.z += (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.5f;

          // Initial velocity: upward with horizontal spread
          float angleX = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * spread * 2.0f;
          float angleZ = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * spread * 2.0f;
          float speedY = riseSpeed + (static_cast<float>(rand()) / RAND_MAX) * riseSpeed * 0.5f;
          p.velocity = { angleX * riseSpeed * 0.5f, speedY, angleZ * riseSpeed * 0.5f };

          // Particle parameters
          p.lifetime = particleLifetime + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.0f;
          // Smoke color – light gray, with maximum opacity
          p.color = { 200, 200, 200, 255 };
      }

      bool allInactive = true;
      for (auto& p : partGrp.particles) {
          if (!p.active) continue;
          allInactive = false;
      }

      if (partGrp.lifetime < 0 && allInactive) {
          partGrp.active = false;
      }     
  }

}