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
    float size = 0.05f;
    float lifetime = 0;
  };

  class ParticleGroup {
  public:
    bool active = false;
    float spawnTimer = 0.0f;
    int type;
    int roomId;
    int stageId = -1;
    Vector3 position = {0,0,0};
    vector<Particle> particles;
    Bounds bounds;
    bool initialized = false;
    unsigned int creationOrder = 0;

    ParticleGroup() :
      particles(25)
      {
        reset();
        active = false;
      }

    void reset() {
      spawnTimer = 0.0f;
      type = 0;
      roomId = -1;
      stageId = -1;
      position = {0,0,0};
      initialized = false;
      for (auto& p : particles) {
          p.active = false;
      }
    }      

    void calcActive();
    void ParticleGroup::calcBounds();
    Bounds ParticleGroup::getRenderBounds();
    Particle& ParticleGroup::addParticle();
  };

  class ParticleGroups {
  public:
      std::vector<ParticleGroup> groups;
      unsigned int nextOrder = 0;
 
      ParticleGroups(): groups(10) {
          clear();
      }

      ParticleGroup& add();

      void clear() {
          for (auto& g : groups) {
              g.active = false;
          }
      }
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

}