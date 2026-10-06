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

  // Loads a 256-color palette (768 bytes) located at `offset` inside `data`.
  // Mirrors PAKExtract Palette.LoadPalette: when every component is a 6-bit
  // VGA value (<= 63), it is rescaled to 8-bit (AITD2 / AITD3 / TIME GATE).
  inline Pallete loadPalleteFromData(const u8* data, size_t offset) {
      bool vgaMap = true;
      for (size_t i = 0; i < PALETTE_SIZE * 3; i++) {
          if (data[offset + i] > 63) {
              vgaMap = false;
              break;
          }
      }

      Pallete palette;
      for (size_t i = 0; i < PALETTE_SIZE; i++) {
          int r = data[offset + i * 3 + 0];
          int g = data[offset + i * 3 + 1];
          int b = data[offset + i * 3 + 2];

          if (vgaMap) { // AITD2, AITD3, TIME GATE
              r = (r << 2) | (r >> 4);
              g = (g << 2) | (g >> 4);
              b = (b << 2) | (b >> 4);
          }

          palette[i][0] = (u8)r;
          palette[i][1] = (u8)g;
          palette[i][2] = (u8)b;
      }
      return palette;
  }

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

        return loadPalleteFromData(data.data(), 0);
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