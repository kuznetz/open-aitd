#pragma once
#include <map>
#include <string>
#include <stdexcept>
#include "../raylib-cpp.h"
#include "data_path.h"

namespace openAITD {
  using namespace std;
  using namespace raylib;

  class Audio {
  public:
      bool initialized = false;
      map<int, Sound> sounds;
      Audio() {}
      ~Audio() {}
      void LoadSound(const int soundId);
      void PlaySound(const int soundId);
  };

  inline void Audio::LoadSound(const int soundId) {
    if (sounds.find(soundId) != sounds.end()) return;
    string filename = DataPath::GetFile(string("sounds/") + to_string(soundId) + ".wav");
    if (filename.empty()) {
      string e = "Sound file not found: " + to_string(soundId);
      throw exception(e.c_str());
    }
    
    auto wave = raylib::LoadWave(filename.c_str());
    if (!IsWaveValid(wave)) {
      string e = "Invalid Sound: " + to_string(soundId);
      throw exception(e.c_str());
    }
    sounds[soundId] = raylib::LoadSoundFromWave(wave);
  }

  inline void Audio::PlaySound(const int soundId) {
    if (!initialized) {
      initialized = true;
      InitAudioDevice();
    }
    if (sounds.find(soundId) == sounds.end()) {
      Audio::LoadSound(soundId);
    }
    raylib::PlaySound(sounds[soundId]);

  }

}