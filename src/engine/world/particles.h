#pragma once
#include <vector>
#include <cstdlib>
#include <cmath>
#include <limits>

#include "../../common/raylib_cpp.hpp"
#include "../resources/bounds.h"

using namespace std;
using namespace raylib;

namespace openAITD {

  // =============================================================
  // Random helpers
  // =============================================================

  inline float rand01() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  }

  inline float randRange(float a, float b) {
    return a + rand01() * (b - a);
  }

  inline float randSym(float amp) {
    return randRange(-amp, amp);
  }

  // =============================================================
  // Curves and gradients
  // =============================================================

  struct FloatKey { float t; float value; };
  struct ColorKey { float t; Color color; };

  // Piecewise-linear curve of scalar values.
  struct FloatCurve {
    static constexpr int MAX_KEYS = 4;
    FloatKey keys[MAX_KEYS]{};
    int count = 0;

    float sample(float t) const {
      if (count == 0) return 0.0f;
      if (t <= keys[0].t) return keys[0].value;
      if (t >= keys[count - 1].t) return keys[count - 1].value;
      for (int i = 1; i < count; ++i) {
        if (t <= keys[i].t) {
          float a = (t - keys[i - 1].t) / (keys[i].t - keys[i - 1].t);
          return keys[i - 1].value + a * (keys[i].value - keys[i - 1].value);
        }
      }
      return keys[count - 1].value;
    }
  };

  inline unsigned char lerpChannel(unsigned char a, unsigned char b, float t) {
    return static_cast<unsigned char>(a + (b - a) * t);
  }

  inline Color lerpColor(Color a, Color b, float t) {
    return Color{
        lerpChannel(a.r, b.r, t),
        lerpChannel(a.g, b.g, t),
        lerpChannel(a.b, b.b, t),
        lerpChannel(a.a, b.a, t)
    };
  }

  // Piecewise-linear color gradient.
  struct ColorGradient {
    static constexpr int MAX_KEYS = 4;
    ColorKey keys[MAX_KEYS]{};
    int count = 0;

    Color sample(float t) const {
      if (count == 0) return WHITE;
      if (t <= keys[0].t) return keys[0].color;
      if (t >= keys[count - 1].t) return keys[count - 1].color;
      for (int i = 1; i < count; ++i) {
        if (t <= keys[i].t) {
          float a = (t - keys[i - 1].t) / (keys[i].t - keys[i - 1].t);
          return lerpColor(keys[i - 1].color, keys[i].color, a);
        }
      }
      return keys[count - 1].color;
    }
  };

  // =============================================================
  // Particle
  // =============================================================

  struct Particle {
    bool active = false;
    Vector3 position = {0, 0, 0};
    Vector3 velocity = {0, 0, 0};
    Color color = {255, 255, 255, 255};
    float size = 0.05f;
    float baseSize = 0.05f;
    float lifetime = 0.0f;
    float maxLifetime = 1.0f;
  };

  // =============================================================
  // Emitter configuration
  // =============================================================

  // Declarative description of a particle effect. All effects are
  // produced by the same universal ParticleController; presets below
  // describe the differences between them.
  struct EmitterConfig {
    // group
    float groupLifetime = -1.0f;       // <0 — infinite

    // spawn
    bool  burst = false;               // true — one-shot burst, false — by timer
    int   burstCount = 0;
    float spawnInterval = 0.1f;

    // emission shape
    bool  useGroupDirection = false;   // use group direction, otherwise "up"
    float coneHalfAngle = 0.4f;        // radians
    float positionJitter = 0.0f;

    // initial values
    float speedMin = 1.0f;
    float speedMax = 2.0f;
    float lifetimeMin = 1.0f;
    float lifetimeMax = 1.5f;
    float sizeMin = 0.1f;
    float sizeMax = 0.25f;

    // how it changes over lifetime (t = 0..1)
    FloatCurve    sizeOverLife;        // size in % of baseSize (100 = baseSize); count==0 -> baseSize
    ColorGradient colorOverLife;       // if count==0, baseColor is used

    // physics
    bool  useGravity = false;
    float gravity = -9.8f;
    bool  stickToGround = false;
    bool  bounce = false;
    float bounceDamping = 0.5f;
    bool  useDrag = false;
    float drag = 0.2f;
    bool  useTurbulence = false;
    float turbulence = 1.0f;

    // rendering
    Texture2D* texture = nullptr;
    Color baseColor = WHITE;
  };

  // =============================================================
  // Particle group (a live emitter instance)
  // =============================================================

  struct ParticleGroup {
    bool active = false;
    float spawnTimer = 0.0f;
    const EmitterConfig* config = nullptr;
    int roomId = -1;
    int stageId = -1;
    Vector3 position = {0, 0, 0};
    Vector3 direction = {0, 0, 0};
    vector<Particle> particles;
    Bounds bounds;
    bool initialized = false;
    unsigned int creationOrder = 0;
    float lifetime = 0.0f;

    ParticleGroup() {
      particles.reserve(32);
      reset();
      active = false;
    }

    void reset() {
      spawnTimer = 0.0f;
      config = nullptr;
      roomId = -1;
      stageId = -1;
      position = {0, 0, 0};
      direction = {0, 0, 0};
      initialized = false;
      lifetime = 0.0f;
      for (auto& p : particles) {
        p.active = false;
      }
    }

    // Reuse an inactive slot, otherwise grow the pool.
    Particle& addParticle() {
      for (auto& p : particles) {
        if (!p.active) return p;
      }
      particles.push_back(Particle{});
      return particles.back();
    }

    bool allInactive() const {
      for (const auto& p : particles) {
        if (p.active) return false;
      }
      return true;
    }

    void calcActive();
    void calcBounds();
    Bounds getRenderBounds();
  };

  inline void ParticleGroup::calcActive() {
    for (const auto& p : particles) {
      if (p.active) {
        active = true;
        return;
      }
    }
    active = false;
  }

  inline Bounds ParticleGroup::getRenderBounds() {
    Bounds bounds;
    bool first = true;
    for (const auto& p : particles) {
      if (!p.active) continue;
      if (first) {
        bounds.min.x = p.position.x - p.size;
        bounds.min.y = p.position.y - p.size;
        bounds.min.z = p.position.z - p.size;
        bounds.max.x = p.position.x + p.size;
        bounds.max.y = p.position.y + p.size;
        bounds.max.z = p.position.z + p.size;
        first = false;
      } else {
        if (p.position.x - p.size < bounds.min.x) bounds.min.x = p.position.x - p.size;
        if (p.position.y - p.size < bounds.min.y) bounds.min.y = p.position.y - p.size;
        if (p.position.z - p.size < bounds.min.z) bounds.min.z = p.position.z - p.size;
        if (p.position.x + p.size > bounds.max.x) bounds.max.x = p.position.x + p.size;
        if (p.position.y + p.size > bounds.max.y) bounds.max.y = p.position.y + p.size;
        if (p.position.z + p.size > bounds.max.z) bounds.max.z = p.position.z + p.size;
      }
    }
    if (first) {
      bounds.min = bounds.max = {0, 0, 0};
    }
    return bounds;
  }

  inline void ParticleGroup::calcBounds() {
    bounds.min = {position.x - 0.2f, position.y - 0.2f, position.z - 0.2f};
    bounds.max = {position.x + 0.2f, position.y + 0.2f, position.z + 0.2f};
  }

  // =============================================================
  // Pool of groups (bounded memory)
  // =============================================================

  class ParticleGroups {
  public:
    std::vector<ParticleGroup> groups;
    unsigned int nextOrder = 0;

    ParticleGroups() : groups(10) {
      clear();
    }

    ParticleGroup& add();

    void clear() {
      for (auto& g : groups) {
        g.active = false;
      }
    }
  };

  inline ParticleGroup& ParticleGroups::add() {
    for (auto& g : groups) {
      if (!g.active) {
        g.reset();
        g.active = true;
        g.creationOrder = nextOrder++;
        return g;
      }
    }
    ParticleGroup* oldest = &groups[0];
    for (auto& g : groups) {
      if (g.creationOrder < oldest->creationOrder) {
        oldest = &g;
      }
    }
    oldest->reset();
    oldest->active = true;
    oldest->creationOrder = nextOrder++;
    return *oldest;
  }

  // =============================================================
  // Presets (built-in effects)
  // =============================================================

  namespace ParticlePresets {

    inline const EmitterConfig Fountain = []{
      EmitterConfig c;
      c.spawnInterval = 0.1f;
      c.coneHalfAngle = 0.15f;
      c.positionJitter = 0.05f;
      c.speedMin = 4.0f;
      c.speedMax = 6.0f;
      c.lifetimeMin = 1.2f;
      c.lifetimeMax = 1.8f;
      c.sizeMin = 0.10f;
      c.sizeMax = 0.15f;
      c.useGravity = true;
      c.bounce = true;
      c.colorOverLife.count = 2;
      c.colorOverLife.keys[0] = {0.0f, {255, 200, 100, 255}};
      c.colorOverLife.keys[1] = {1.0f, {255, 100,   0,   0}};
      c.baseColor = {255, 128, 0, 255};
      return c;
    }();

    inline const EmitterConfig Ricochet = []{
      EmitterConfig c;
      c.burst = true;
      c.burstCount = 10;
      c.coneHalfAngle = 0.6f;
      c.speedMin = 2.0f;
      c.speedMax = 3.5f;
      c.lifetimeMin = 0.7f;
      c.lifetimeMax = 1.2f;
      c.sizeMin = 0.05f;
      c.sizeMax = 0.10f;
      c.useGravity = true;
      c.bounce = true;
      c.bounceDamping = 0.3f;
      c.sizeOverLife.count = 3;
      c.sizeOverLife.keys[0] = {0.0f, 100.0f};
      c.sizeOverLife.keys[1] = {0.7f, 100.0f};      
      c.sizeOverLife.keys[2] = {1.0f,   0.0f};      
      c.baseColor = WHITE;
      return c;
    }();

    inline const EmitterConfig Blood = []{
      EmitterConfig c;
      c.burst = true;
      c.burstCount = 10;
      c.coneHalfAngle = 0.6f;
      c.speedMin = 2.0f;
      c.speedMax = 3.5f;
      c.lifetimeMin = 1.2f;
      c.lifetimeMax = 1.5f;
      c.sizeMin = 0.05f;
      c.sizeMax = 0.10f;
      c.useGravity = true;
      c.stickToGround = true;
      c.sizeOverLife.count = 3;
      c.sizeOverLife.keys[0] = {0.0f, 100.0f};
      c.sizeOverLife.keys[1] = {0.9f, 150.0f};      
      c.sizeOverLife.keys[2] = {1.0f,   0.0f};
      c.baseColor = RED;
      return c;
    }();    

    inline const EmitterConfig Smoke = []{
      EmitterConfig c;
      c.groupLifetime = 2.0f;
      c.spawnInterval = 0.05f;
      c.coneHalfAngle = 0.8f;
      c.positionJitter = 0.25f;
      c.speedMin = 1.0f;
      c.speedMax = 2.0f;
      c.lifetimeMin = 1.0f;
      c.lifetimeMax = 2.0f;
      c.sizeMin = 0.25f;
      c.sizeMax = 0.25f;
      c.useDrag = true;
      c.drag = 0.2f;
      c.useTurbulence = true;
      c.turbulence = 1.2f;
      c.sizeOverLife.count = 2;
      c.sizeOverLife.keys[0] = {0.0f, 100.0f};
      c.sizeOverLife.keys[1] = {1.0f,   0.0f};
      c.baseColor = {200, 200, 200, 255};
      return c;
    }();

    inline const EmitterConfig CigarSmoke = []{
      EmitterConfig c;
      c.groupLifetime = 1.0f;
      c.spawnInterval = 0.2f;
      c.coneHalfAngle = 1.2f;
      c.positionJitter = 0.0f;
      c.speedMin = 0.3f;
      c.speedMax = 0.7f;
      c.lifetimeMin = 3.0f;
      c.lifetimeMax = 5.0f;
      c.sizeMin = 0.05f;
      c.sizeMax = 0.05f;
      c.useDrag = true;
      c.drag = 0.2f;
      c.useTurbulence = true;
      c.turbulence = 1.2f;
      c.sizeOverLife.count = 2;
      c.sizeOverLife.keys[0] = {0.0f,  100.0f};
      c.sizeOverLife.keys[1] = {1.0f, 1200.0f};
      c.baseColor = {200, 200, 200, 255};
      return c;
    }();

    inline const EmitterConfig MuzzleFlash = []{
      EmitterConfig c;
      c.burst = true;
      c.burstCount = 5;
      c.useGroupDirection = true;
      c.coneHalfAngle = 0.4f;
      c.speedMin = 2.5f;
      c.speedMax = 3.5f;
      c.lifetimeMin = 0.15f;
      c.lifetimeMax = 0.25f;
      c.sizeMin = 0.25f;
      c.sizeMax = 0.25f;
      c.sizeOverLife.count = 2;
      c.sizeOverLife.keys[0] = {0.0f, 100.0f};
      c.sizeOverLife.keys[1] = {1.0f,   0.0f};
      c.baseColor = {255, 200, 100, 255};
      return c;
    }();

  } // namespace ParticlePresets

} // namespace openAITD
