#pragma once
#include <vector>
#include "../../common/raylib_cpp.hpp"

using namespace std;
using namespace raylib;

namespace openAITD {

  class Particle {
  public:
    bool active;
    Vector3 position;
    Color color;
    float size;
    float lifetime;
  };

  class ParticleGroup {
  public:
    bool active;
    int type;
    int roomId;
    Vector3 position;
    vector<Particle> particles;
    float lifetime;
  };

}