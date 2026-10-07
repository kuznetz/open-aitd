#pragma once
#include <map>
#include <string>
#include <stdexcept>
#include <random>
#include "../../common/raylib_cpp.hpp"
#include "data_path.h"

namespace openAITD {
  using namespace std;
  using namespace raylib;

  class Audio {
  public:
      bool initialized = false;
      map<int, Sound> sounds;

      int currentMusicId = -1;
      float musicVolume = 1.0f;

      raylib::Music musicTrack;

      Audio() {}
      ~Audio() {}
      void Init();
      void LoadSound(const int soundId);
      void PlaySound(const int soundId, const float rndFreq = 0);

      void LoadMusic(const int musicId);
      void StopMusic();
      void PlayMusic(const int musicId);
      void SetMusicVolume(float volume);
      void Process();

  private:
      std::mt19937 rng{ std::random_device{}() };

      // Равномерное случайное смещение в диапазоне [-amplitude, +amplitude]
      float RandomAmplitude(const float amplitude) {
        std::uniform_real_distribution<float> dist(-amplitude, amplitude);
        return dist(rng);
      }
  };

  inline void Audio::Init() {
    if (!initialized) {
      initialized = true;
      InitAudioDevice();
    }
  }

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

  inline void Audio::PlaySound(const int soundId, const float rndFreq) {
    Init();
    if (sounds.find(soundId) == sounds.end()) {
      Audio::LoadSound(soundId);
    }

    Sound& snd = sounds[soundId];

    // rndFreq задаёт относительный разброс частоты: 0 = без изменений,
    // 0.5 = +-50% (pitch в диапазоне [0.5, 1.5]).
    float pitch = 1.0f;
    if (rndFreq > 0.0f) {
      pitch = 1.0f + RandomAmplitude(rndFreq);
      if (pitch < 0.05f) pitch = 0.05f; // защита от нулевого/отрицательного pitch
    }

    // Sound кэшируется и переиспользуется, поэтому pitch нужно выставлять
    // всегда (в т.ч. 1.0), иначе предыдущая рандомизация "прилипнет".
    raylib::SetSoundPitch(snd, pitch);

    raylib::PlaySound(snd);
  }

  inline void Audio::StopMusic() {
      if (currentMusicId != -1) {
          raylib::StopMusicStream(musicTrack);
          raylib::UnloadMusicStream(musicTrack);
          currentMusicId = -1;
      }
  }

  inline void Audio::LoadMusic(const int musicId) {
      vector<string> extensions = {".flac", ".ogg", ".mp3", ".wav"};
      string filename;
      for (const auto& ext : extensions) {
          filename = DataPath::GetFile(string("music/") + to_string(musicId) + ext);
          if (!filename.empty()) {
              break;
          }
      }
      
      if (filename.empty()) {
          string e = "Music file not found for ID: " + to_string(musicId);
          throw runtime_error(e);
      }
      
      musicTrack = raylib::LoadMusicStream(filename.c_str());
      if (!raylib::IsMusicValid(musicTrack)) {
          string e = "Failed to load music: " + filename;
          throw runtime_error(e);
      }
      
      SetMusicVolume(musicVolume);
  }

  inline void Audio::PlayMusic(const int musicId) {      
      if (currentMusicId != -1 && currentMusicId != musicId) {
          StopMusic();
      }

      Init();
      try {
        LoadMusic(musicId);
        raylib::PlayMusicStream(musicTrack);
        currentMusicId = musicId;
      } catch (exception e) {
        //Failed to load music
      }
  }

  inline void Audio::SetMusicVolume(float volume) {
      musicVolume = volume;
      if (currentMusicId != -1) {
          raylib::SetMusicVolume(musicTrack, volume);
      }
  }  

  inline void Audio::Process() {
      if (currentMusicId == -1) return;
      raylib::UpdateMusicStream(musicTrack);
      if (!raylib::IsMusicStreamPlaying(musicTrack)) {
        //StopMusic();
      }
  }

}