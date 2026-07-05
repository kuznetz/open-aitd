#pragma once
#include "../pak/pak.h"
#include "../structs/model.h"
#include "../structs/int_types.h"
#include "../structs/game_objects.h"
#include "../structs/floor.h"
#include "../structs/animation.h"
#include "../structs/life.h"
#include <vector>
#include <array>
#include <vector>
#include <stdexcept>

namespace AITDExtractor {
  using namespace std;

  constexpr size_t PALETTE_SIZE = 256;
  typedef std::array<u8, 3> PalleteColor;
  typedef std::array<PalleteColor, 256> Pallete;

  class ResourceLoader {
  public:

    ResourceLoader(const string dataPath = "./original") {
      this->dataPath = dataPath;
    }

    Pallete loadPallete() {        
        PakFile pak(dataPath+"/ITD_RESS.PAK");
        auto data = pak.readBlock(3);
        if (data.size() < PALETTE_SIZE*3) {
            throw std::runtime_error("Palette data too small (wrong ITD_RESS.PAK)");
        }

        Pallete palette;
        for (size_t i = 0; i < PALETTE_SIZE; i++) {
            palette[i][0] = data[i*3];
            palette[i][1] = data[i*3 + 1];
            palette[i][2] = data[i*3 + 2];
        }
        return palette;
    };

    PakModel loadModel(vector<u8>& data);
    Animation loadAnimation(vector<u8>& data);
    floorStruct loadFloor(const int floorNum);
    vector<LifeInstruction> loadLife(vector<u8>& data, bool floppy = false);
    vector<gameObjectStruct> loadGameObjects();

  private:
    string dataPath;
  };
  
}