#pragma once
#include <vector>
#include "../../common/raylib_cpp.hpp"

using namespace std;
using namespace raylib;

namespace openAITD {

  class Particle {
  public:
    bool active;
    Vector3 position = {0,0,0};
    Vector3 velocity = {0,0,0};
    Color color = {255,255,255,255};
    float size = 0.1f;
    float lifetime = 0;
  };

  class ParticleGroup {
  public:
    bool active = false;
    float spawnTimer = 0.0f;
    int type;
    int roomId;
    Vector3 position = {0,0,0};
    vector<Particle> particles;
    Bounds bounds;
    float lifetime;

    ParticleGroup() :
      particles(25)
      {}

    void calcActive();
    void ParticleGroup::calcBounds();
    Particle& ParticleGroup::addParticle();
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

  inline void ParticleGroup::calcBounds() {
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
  }

  inline Particle& ParticleGroup::addParticle() {
      Particle* candidate = nullptr;
      float minLifetime = std::numeric_limits<float>::max();
      for (auto& p : particles) {
          if (!p.active) {
              p.active = true;
              return p;
          }
          if (p.lifetime < minLifetime) {
              minLifetime = p.lifetime;
              candidate = &p;
          }
      }
      return *candidate;
  }

}