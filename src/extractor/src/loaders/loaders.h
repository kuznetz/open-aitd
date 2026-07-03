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

  typedef std::array<u8, 3> PalleteColor;
  typedef std::array<PalleteColor, 256> Pallete;

  class ResourceLoader {
  public:
    static Pallete loadPallete() {
        constexpr size_t PALETTE_SIZE = 256;
        PakFile pak("original/ITD_RESS.PAK");
        auto data = pak.readBlock(3);
        if (data.size() < PALETTE_SIZE) {
            throw std::runtime_error("Palette data too small (wrong ITD_RESS.PAK)");
        }

        Pallete palette;
        for (size_t i = 0; i < PALETTE_SIZE; i += 3) {
            palette[i][0] = data[i];
            palette[i][1] = data[i + 1];
            palette[i][2] = data[i + 2];
        }
        return palette;
    };

    static PakModel loadModel(vector<u8>& data);
    static Animation loadAnimation(vector<u8>& data);
    static floorStruct loadFloor(const string filename);
    static vector<LifeInstruction> loadLife(vector<u8>& data, bool floppy = false);
    static vector<gameObjectStruct> loadGameObjects(const string fileName);
  };
  
}