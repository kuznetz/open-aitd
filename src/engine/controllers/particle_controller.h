#pragma once
#include <cmath>

#include "../resources/resources.h"
#include "../world/world.h"

namespace openAITD {

  // Universal, data-driven particle controller.
  //
  // All effects are described by an EmitterConfig (see ParticlePresets in
  // particles.h). The controller contains no per-effect logic: it integrates
  // physics, spawns particles and applies size/color curves purely from config.
  class ParticleController {
  public:
    World& world;
    Resources& resources;

    ParticleController(World& world)
        : world(world), resources(*world.resources) {}

    void process(const float timeDelta) {
      for (auto& g : world.partGroups.groups) {
        if (g.active && g.config) {
          step(g, timeDelta);
        }
      }
    }

  private:
    void step(ParticleGroup& g, float dt);
    void updateParticle(Particle& p, const ParticleGroup& g, float dt);
    void spawnOne(ParticleGroup& g);
    void spawnBurst(ParticleGroup& g);
    Vector3 sampleSpawnDirection(const ParticleGroup& g) const;
  };

  inline void ParticleController::step(ParticleGroup& g, const float dt) {
    const EmitterConfig& cfg = *g.config;

    // First-time initialization
    if (!g.initialized) {
      g.lifetime = cfg.groupLifetime;
      if (cfg.burst) spawnBurst(g);
      g.initialized = true;
    }

    // Update alive particles
    for (auto& p : g.particles) {
      if (p.active) updateParticle(p, g, dt);
    }

    // Timed spawning
    if (!cfg.burst && cfg.spawnInterval > 0.0f) {
      g.spawnTimer -= dt;
      bool canSpawn = (cfg.groupLifetime < 0.0f) || (g.lifetime > 0.0f);
      int safety = 128;   // guard against large dt
      while (canSpawn && g.spawnTimer <= 0.0f && safety-- > 0) {
        g.spawnTimer += cfg.spawnInterval;
        spawnOne(g);
      }
    }

    // Group deactivation
    if (cfg.groupLifetime >= 0.0f) {
      // Finite emitter: stop after its lifetime once everything has died.
      g.lifetime -= dt;
      if (g.lifetime <= 0.0f && g.allInactive()) {
        g.active = false;
      }
    } else if (cfg.burst) {
      // Infinite burst emitter: die as soon as the burst is exhausted.
      if (g.allInactive()) {
        g.active = false;
      }
    }
    // Infinite timed emitters (e.g. Fountain) never deactivate on their own.
  }

  inline void ParticleController::updateParticle(Particle& p, const ParticleGroup& g, const float dt) {
    const EmitterConfig& cfg = *g.config;

    // Movement
    p.position += p.velocity * dt;

    // Gravity
    if (cfg.useGravity && p.position.y > 0.0f) {
      p.velocity.y += cfg.gravity * dt;
    }

    // Drag
    if (cfg.useDrag) {
      p.velocity *= (1.0f - cfg.drag * dt);
    }

    // Turbulence
    if (cfg.useTurbulence) {
      p.velocity.x += randSym(cfg.turbulence * dt);
      p.velocity.z += randSym(cfg.turbulence * dt);
    }

    // Floor collision
    if (p.position.y <= 0.0f) {
      if (cfg.stickToGround) {
        p.velocity = {0.0f, 0.0f, 0.0f};
      } else if (cfg.bounce && std::fabs(p.velocity.y) > 0.01f) {
        p.velocity.y = -p.velocity.y * cfg.bounceDamping;
      } else {
        p.velocity.y = 0.0f;
      }
    }

    // Lifetime
    p.lifetime -= dt;
    if (p.lifetime <= 0.0f) {
      p.active = false;
      return;
    }

    // Appearance
    float t = 1.0f - p.lifetime / p.maxLifetime;   // 0 — birth, 1 — death

    if (cfg.sizeOverLife.count > 0)
      p.size = p.baseSize * cfg.sizeOverLife.sample(t) * 0.01f;  // curve is in % of baseSize
    else
      p.size = p.baseSize;

    if (cfg.colorOverLife.count > 0)
      p.color = cfg.colorOverLife.sample(t);
  }

  inline void ParticleController::spawnOne(ParticleGroup& g) {
    const EmitterConfig& cfg = *g.config;
    Particle& p = g.addParticle();

    // Position
    p.position = g.position;
    if (cfg.positionJitter > 0.0f) {
      p.position.x += randSym(cfg.positionJitter);
      p.position.z += randSym(cfg.positionJitter);
    }

    // Velocity
    Vector3 dir = sampleSpawnDirection(g);
    float speed = randRange(cfg.speedMin, cfg.speedMax);
    p.velocity = dir * speed;

    // Lifetime
    p.maxLifetime = randRange(cfg.lifetimeMin, cfg.lifetimeMax);
    p.lifetime = p.maxLifetime;

    // Size and color
    p.baseSize = randRange(cfg.sizeMin, cfg.sizeMax);
    p.size = p.baseSize;
    p.color = cfg.baseColor;

    p.active = true;
  }

  inline void ParticleController::spawnBurst(ParticleGroup& g) {
    for (int i = 0; i < g.config->burstCount; ++i) {
      spawnOne(g);
    }
  }

  inline Vector3 ParticleController::sampleSpawnDirection(const ParticleGroup& g) const {
    const EmitterConfig& cfg = *g.config;

    Vector3 base = {0.0f, 1.0f, 0.0f};
    if (cfg.useGroupDirection) {
      float len = Vector3Length(g.direction);
      if (len > 1e-4f) base = g.direction / len;
    }

    // Orthonormal basis around base
    Vector3 up = (std::fabs(base.y) < 0.99f)
        ? Vector3{0.0f, 1.0f, 0.0f}
        : Vector3{1.0f, 0.0f, 0.0f};

    Vector3 right = Vector3Normalize(Vector3CrossProduct(base, up));
    up = Vector3Normalize(Vector3CrossProduct(right, base));

    // Random direction in cone
    float theta = randSym(cfg.coneHalfAngle);
    float phi   = rand01() * 2.0f * PI;

    Vector3 d = base * std::cos(theta)
              + (right * std::cos(phi) + up * std::sin(phi)) * std::sin(theta);
    return Vector3Normalize(d);
  }

} // namespace openAITD
