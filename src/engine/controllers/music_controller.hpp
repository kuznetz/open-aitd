#pragma once
#include "../resources/resources.h"
#include "../world/world.h"

namespace openAITD {

  // Music state machine.
  //
  // The Audio resource only knows how to load/play/stop a single stream and to
  // set its volume, so all playlist logic lives here:
  //   targetMusicId  - the track that should be playing  (PlayMusic)
  //   nextMusicId    - the track queued after the current (PlayNextMusic)
  //   currentMusicId - what actually plays right now (Audio::currentMusicId)
  //
  // When the current track ends and nextMusicId is set, playback jumps to it
  // without any fade; otherwise the target track is repeated seamlessly.
  // Switching to a different target while another track is playing goes through
  // a fade (fade out -> swap -> fade in) implemented with Audio::SetMusicVolume.
  //
  // process() is also the only place that applies config.musicVolume, so the
  // volume options are respected even in the middle of a fade, and it silences
  // the music while a book is being read (World::bookData).
  class MusicController {
  public:
    World& world;
    Resources& resources;

    // The track that should be playing; -1 means "no music".
    int targetMusicId = -1;
    // The track scheduled to start right after the current one; -1 = none.
    int nextMusicId = -1;

    // Fade durations in seconds (0 disables the corresponding fade).
    float fadeOutTime = 1.0f;
    float fadeInTime = 0.5f;

    MusicController(World& world)
      : world(world), resources(*world.resources) {}

    // Sets the track that should be playing. The actual change happens in
    // process(): the current track is faded out first unless nothing plays.
    void PlayMusic(const int musicId) {
      if (musicId == targetMusicId) return;
      targetMusicId = musicId;
      failedMusicId = -1;
    }

    // Sets the track to start once the current one ends (no fade).
    void PlayNextMusic(const int musicId) {
      nextMusicId = musicId;
    }

    // Stops the music immediately and drops the whole playlist state.
    void StopMusic() {
      resources.audio.StopMusic();
      targetMusicId = -1;
      nextMusicId = -1;
      failedMusicId = -1;
      fadeFactor = 1.0f;
      trackStarted = false;
      appliedVolume = -1.0f;
      phase = Phase::Idle;
    }

    void process(const float timeDelta);

  private:
    enum class Phase { Idle, Playing, FadeOut, FadeIn };

    Phase phase = Phase::Idle;
    // Volume multiplier in [0, 1] applied on top of config.musicVolume.
    float fadeFactor = 1.0f;
    // Last volume pushed to the audio resource (avoids redundant calls).
    float appliedVolume = -1.0f;
    // True once the current track has been loaded and started.
    bool trackStarted = false;
    // Set while the music is silenced because a book is being read.
    bool pausedForBook = false;
    // Track that failed to load: prevents retrying it on every frame.
    int failedMusicId = -1;

    // Starts the given track right away; false if it could not be loaded.
    bool startTrack(const int musicId);
    // Restarts the current track from the beginning (manual looping).
    void repeatTrack();
    // True when the current stream has reached its end.
    bool isTrackFinished() const;
    // Pushes config.musicVolume * fadeFactor into the audio resource.
    void applyVolume();
  };

  inline bool MusicController::startTrack(const int musicId) {
    resources.audio.PlayMusic(musicId);
    if (resources.audio.currentMusicId != musicId) {
      // Audio::PlayMusic swallows load errors: remember the id and give up.
      failedMusicId = musicId;
      trackStarted = false;
      return false;
    }
    trackStarted = true;
    // The end of the track is handled by this controller, not by raylib.
    resources.audio.musicTrack.looping = false;
    // Audio::PlayMusic applies the volume while the stream id is still unset,
    // so the new stream may still have the default volume: force a volume push.
    appliedVolume = -1.0f;
    return true;
  }

  inline void MusicController::repeatTrack() {
    raylib::SeekMusicStream(resources.audio.musicTrack, 0.0f);
    resources.audio.musicTrack.looping = false;
    raylib::PlayMusicStream(resources.audio.musicTrack);
    trackStarted = true;
    appliedVolume = -1.0f;
  }

  inline bool MusicController::isTrackFinished() const {
    if (!trackStarted) return false;
    if (resources.audio.currentMusicId == -1) return false;

    const raylib::Music& track = resources.audio.musicTrack;
    // raylib streams the last frames and then stops the stream itself
    // (its `playing` flag is cleared), which is the end of the track.
    if (!raylib::IsMusicStreamPlaying(track)) return true;

    // Fallback: the play head has reached the end of the file.
    const float length = raylib::GetMusicTimeLength(track);
    if (length <= 0.0f) return false;
    const float played = raylib::GetMusicTimePlayed(track);
    return played >= length - 0.05f;
  }

  inline void MusicController::applyVolume() {
    float volume = resources.config.musicVolume * fadeFactor;
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    if (volume == appliedVolume) return;
    appliedVolume = volume;
    resources.audio.SetMusicVolume(volume);
  }

  inline void MusicController::process(const float timeDelta) {
    Audio& audio = resources.audio;

    // ---- A book being read silences the music (the target is kept) ----
    if (world.bookData.readText != -1) {
      if (!pausedForBook) {
        pausedForBook = true;
        audio.StopMusic();
        phase = Phase::Idle;
        fadeFactor = 1.0f;
        trackStarted = false;
      }
      return;
    }
    if (pausedForBook) {
      // Reading is over: the state machine below restarts the pending target.
      pausedForBook = false;
      trackStarted = false;
    }

    // Guard against huge frame times (window drag, breakpoint, ...).
    const float dt = (timeDelta > 0.25f) ? 0.25f : (timeDelta > 0.0f ? timeDelta : 0.0f);

    switch (phase) {
    case Phase::Idle: {
      if (targetMusicId == -1 || targetMusicId == failedMusicId) break;
      if (audio.currentMusicId == targetMusicId) {
        // Something else (e.g. the audio book) already resumed this track.
        trackStarted = true;
        audio.musicTrack.looping = false;
        appliedVolume = -1.0f; // its volume may not be the expected one yet
      } else if (!startTrack(targetMusicId)) {
        break; // failed to load, the target stays but is not retried
      }
      if (fadeInTime > 0.0f) {
        fadeFactor = 0.0f;
        phase = Phase::FadeIn;
      } else {
        fadeFactor = 1.0f;
        phase = Phase::Playing;
      }
      break;
    }
    case Phase::Playing: {
      // The requested target changed: the playing track has to go.
      if (targetMusicId != audio.currentMusicId) {
        phase = Phase::FadeOut;
        break;
      }
      if (!isTrackFinished()) break;

      const int next = nextMusicId;
      nextMusicId = -1;
      if (next != -1) {
        // Jump to the scheduled track without any fade.
        targetMusicId = next;
        if (!startTrack(next)) break;
        fadeFactor = 1.0f;
        phase = Phase::Playing;
      } else {
        // Nothing scheduled: repeat the current track (seamless loop).
        repeatTrack();
      }
      break;
    }
    case Phase::FadeOut: {
      // The target went back to what is already playing: fade in again.
      if (targetMusicId != -1 && targetMusicId == audio.currentMusicId) {
        phase = Phase::FadeIn;
        break;
      }
      if (fadeOutTime > 0.0f) {
        fadeFactor -= dt / fadeOutTime;
      } else {
        fadeFactor = 0.0f;
      }
      if (fadeFactor > 0.0f) break;

      // Fully faded out: stop the old track and start the new one.
      fadeFactor = 0.0f;
      audio.StopMusic();
      trackStarted = false;
      if (targetMusicId == -1 || targetMusicId == failedMusicId) {
        phase = Phase::Idle;
        break;
      }
      if (!startTrack(targetMusicId)) break;
      if (fadeInTime > 0.0f) {
        phase = Phase::FadeIn;
      } else {
        fadeFactor = 1.0f;
        phase = Phase::Playing;
      }
      break;
    }
    case Phase::FadeIn: {
      if (targetMusicId != audio.currentMusicId) {
        phase = Phase::FadeOut;
        break;
      }
      if (fadeInTime > 0.0f) {
        fadeFactor += dt / fadeInTime;
      } else {
        fadeFactor = 1.0f;
      }
      if (fadeFactor >= 1.0f) {
        fadeFactor = 1.0f;
        phase = Phase::Playing;
      }
      break;
    }
    }

    applyVolume();
  }

}
