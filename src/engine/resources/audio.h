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
      // Gain applied to one-shot/cyclic sound effects (Sound volume option).
      float soundVolume = 1.0f;
      // Gain applied to audio book voice-over (Voice volume option).
      float voiceVolume = 1.0f;

      raylib::Music musicTrack;

      // Audio book (voice-over) currently playing: audiobooks/{num}.{page}.ogg
      int currentAudioBookNum = -1;
      int currentAudioBookPage = -1;
      raylib::Music audioBookTrack;

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
      // Current playback position of the music stream, in seconds (0 if none).
      float GetMusicTime() const;
      // Plays the track. startTime > 0 starts playback at that offset (seconds),
      // e.g. to resume a track from the position where it was paused.
      void PlayMusic(const int musicId, const float startTime = 0.0f);
      void SetMusicVolume(float volume);
      void SetSoundVolume(float volume);
      void SetVoiceVolume(float volume);

      // Plays the audio book page: audiobooks/{audioBookNum}.{pageNum}.ogg.
      // The background music is left to MusicController, which fades it out
      // while a book is open. A missing audio book file is not an error.
      void PlayAudioBook(const int audioBookNum, const int pageNum);
      // Stops the audio book voice-over (the music is handled by MusicController).
      void StopAudioBook();

      void Process();

  private:
      std::mt19937 rng{ std::random_device{}() };

      // Uniform random offset in the range [-amplitude, +amplitude]
      float RandomAmplitude(const float amplitude) {
        std::uniform_real_distribution<float> dist(-amplitude, amplitude);
        return dist(rng);
      }

      // Stops and unloads the current audio book without resuming the music.
      void UnloadAudioBook() {
        if (currentAudioBookNum != -1) {
          raylib::StopMusicStream(audioBookTrack);
          raylib::UnloadMusicStream(audioBookTrack);
          currentAudioBookNum = -1;
          currentAudioBookPage = -1;
        }
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

    // The Sound volume option also applies to cached sounds, so set it
    // on every play (the cached Sound keeps the last volume otherwise).
    raylib::SetSoundVolume(snd, soundVolume);

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

  inline float Audio::GetMusicTime() const {
      if (currentMusicId == -1) return 0.0f;
      return raylib::GetMusicTimePlayed(musicTrack);
  }

  inline void Audio::PlayMusic(const int musicId, const float startTime) {
      if (currentMusicId != -1 && currentMusicId != musicId) {
          StopMusic();
      }

      Init();
      try {
        LoadMusic(musicId);
        raylib::PlayMusicStream(musicTrack);
        // Optional start offset: resume from the position where it was paused.
        if (startTime > 0.0f) {
          raylib::SeekMusicStream(musicTrack, startTime);
        }
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

  inline void Audio::SetSoundVolume(float volume) {
      soundVolume = volume;
      // Update sounds already loaded into the cache.
      for (auto& entry : sounds) {
          raylib::SetSoundVolume(entry.second, volume);
      }
  }

  inline void Audio::SetVoiceVolume(float volume) {
      voiceVolume = volume;
      if (currentAudioBookNum != -1) {
          raylib::SetMusicVolume(audioBookTrack, volume);
      }
  }

  inline void Audio::PlayAudioBook(const int audioBookNum, const int pageNum) {
      if (audioBookNum < 0 || pageNum < 0) return;

      // The same page of the same book is already playing – nothing to do.
      if (currentAudioBookNum == audioBookNum && currentAudioBookPage == pageNum) return;

      // File name: audiobooks/{audioBookNum}.{pageNum}.{ext}.
      // .ogg is the canonical extension, the others are accepted as a fallback.
      vector<string> extensions = { ".ogg", ".wav", ".mp3", ".flac" };
      string filename;
      for (const auto& ext : extensions) {
          filename = DataPath::GetFile(string("audiobooks/") + to_string(audioBookNum)
                                       + "." + to_string(pageNum) + ext);
          if (!filename.empty()) break;
      }

      Init();

      // A missing audio book file is normal: just drop the previous audio book.
      if (filename.empty()) {
          UnloadAudioBook();
          return;
      }

      raylib::Music track = raylib::LoadMusicStream(filename.c_str());
      if (!raylib::IsMusicValid(track)) {
          UnloadAudioBook();
          return;
      }

      // An audio book must play through once: raylib enables music looping by
      // default (LoadMusicStream sets `looping = true`), so turn it off here.
      track.looping = false;

      // The background music is not touched here: while a book is open,
      // MusicController fades it out and resumes it once the book is closed.
      UnloadAudioBook();
      audioBookTrack = track;
      raylib::SetMusicVolume(audioBookTrack, voiceVolume);
      raylib::PlayMusicStream(audioBookTrack);
      currentAudioBookNum = audioBookNum;
      currentAudioBookPage = pageNum;
  }

  inline void Audio::StopAudioBook() {
      // Just drop the voice-over; the background music is resumed (from the
      // position where it was interrupted) by MusicController on book close.
      UnloadAudioBook();
  }

  inline void Audio::Process() {
      // The cyclic sound plays only while REP_SOUND is called every frame.
      if (repeatSoundId != -1 && !repeatSoundRequested) {
          StopRepeatSound();
      }
      repeatSoundRequested = false;

      // The audio book must be advanced every frame to keep playing.
      if (currentAudioBookNum != -1) {
          raylib::UpdateMusicStream(audioBookTrack);
      }

      if (currentMusicId == -1) return;
      raylib::UpdateMusicStream(musicTrack);
      if (!raylib::IsMusicStreamPlaying(musicTrack)) {
        //StopMusic();
      }
  }

}