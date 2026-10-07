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

      // Repeating (cyclic) sound. Plays while REP_SOUND is called every frame.
      // If the command is not issued during a frame, it stops in Process().
      int repeatSoundId = -1;
      bool repeatSoundRequested = false;

      Audio() {}
      ~Audio() {}
      void Init();
      void LoadSound(const int soundId);
      void PlaySound(const int soundId, const float rndFreq = 0);
      void PlayRepeatSound(const int soundId, const float rndFreq = 0);
      void StopRepeatSound();

      void LoadMusic(const int musicId);
      void StopMusic();
      void PlayMusic(const int musicId);
      void SetMusicVolume(float volume);
      void Process();

  private:
      std::mt19937 rng{ std::random_device{}() };

      // Uniform random offset in the range [-amplitude, +amplitude]
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

    // rndFreq sets the relative frequency spread: 0 = no change,
    // 0.5 = +-50% (pitch in the range [0.5, 1.5]).
    float pitch = 1.0f;
    if (rndFreq > 0.0f) {
      pitch = 1.0f + RandomAmplitude(rndFreq);
      if (pitch < 0.05f) pitch = 0.05f; // guard against zero/negative pitch
    }

    // Sound is cached and reused, so pitch must always be set
    // (including 1.0), otherwise the previous randomization would "stick".
    raylib::SetSoundPitch(snd, pitch);

    raylib::PlaySound(snd);
  }

  // Starts a cyclic sound. While the method is called every frame, the sound
  // continues to play (restarts upon completion). As soon as the calls
  // stop, the sound stops in Audio::Process().
  inline void Audio::PlayRepeatSound(const int soundId, const float rndFreq) {
    Init();

    // The repeating sound changed — stop the previous one.
    if (repeatSoundId != -1 && repeatSoundId != soundId) {
      auto it = sounds.find(repeatSoundId);
      if (it != sounds.end()) {
        raylib::StopSound(it->second);
      }
      repeatSoundId = -1;
    }

    if (repeatSoundId == -1) {
      // First frame of playback — start it.
      repeatSoundId = soundId;
      PlaySound(soundId, rndFreq);
    } else {
      // Sound already selected: restart only once it has finished (loop).
      auto it = sounds.find(repeatSoundId);
      if (it != sounds.end() && !raylib::IsSoundPlaying(it->second)) {
        PlaySound(repeatSoundId, rndFreq);
      }
    }

    repeatSoundRequested = true;
  }

  inline void Audio::StopRepeatSound() {
    if (repeatSoundId != -1) {
      auto it = sounds.find(repeatSoundId);
      if (it != sounds.end()) {
        raylib::StopSound(it->second);
      }
      repeatSoundId = -1;
    }
    repeatSoundRequested = false;
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
      // The cyclic sound plays only while REP_SOUND is called every frame.
      if (repeatSoundId != -1 && !repeatSoundRequested) {
          StopRepeatSound();
      }
      repeatSoundRequested = false;

      if (currentMusicId == -1) return;
      raylib::UpdateMusicStream(musicTrack);
      if (!raylib::IsMusicStreamPlaying(musicTrack)) {
        //StopMusic();
      }
  }

}