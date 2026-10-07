#pragma once

#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../../common/raylib_cpp.hpp"

using namespace std;
using namespace raylib;

namespace openAITD {

	// Intro screen: shows data/intro/[0-9].png in order with fade-in/fade-out.
	class IntroScreen {
	public:
		// Per-frame phases: fade in -> fully visible -> fade out -> next frame.
		// Exiting is the ESC-triggered fade-out: twice as fast and finishing the intro.
		enum class Phase { FadeIn, Hold, FadeOut, Exiting };

		World* world;
		Resources* resources;

		// Loaded frame texture (id == 0 means nothing loaded)
		Texture2D texture = { 0 };
		int curFrame = -1;

		Phase phase = Phase::FadeIn;
		float curTime = 0;

		// Durations (seconds)
		float fadeTime = 0.75f;   // fade-in and fade-out duration
		float holdTime = 3.0f;    // fully visible duration

		// Highest frame index probed (data/intro/[0-9].png)
		int maxFrames = 10;

		bool started = false;
		bool finished = false;
		// ESC fade-out runs twice as fast as the regular one.
		static constexpr float exitFadeScale = 0.5f;

		IntroScreen(World* world)
			: world(world)
			, resources(world->resources)
		{}

		~IntroScreen() {
			end();
		}

		void unloadFrame() {
			if (texture.id != 0) {
				UnloadTexture(texture);
				texture = { 0 };
			}
		}

		string framePath(int id) {
			return DataPath::GetFile("intro/" + to_string(id) + ".png");
		}

		bool loadFrame(int id) {
			string path = framePath(id);
			if (path == "") return false;
			unloadFrame();
			texture = resources->backgrounds.loadImageResized(path);
			curFrame = id;
			return true;
		}

		// Start playing from the first available frame.
		void start() {
			curFrame = -1;
			curTime = 0;
			finished = false;
			started = true;
			phase = Phase::FadeIn; // fade in from black on start
			if (!loadFrame(0)) {
				finished = true;
			}
		}

		void end() {
			unloadFrame();
			curFrame = -1;
			curTime = 0;
			started = false;
			finished = false;
			phase = Phase::FadeIn;
		}

		// Advance to the next existing frame, or finish if none left.
		void advance() {
			int next = curFrame + 1;
			if (next >= maxFrames || !loadFrame(next)) {
				finished = true;
				return;
			}
			phase = Phase::FadeIn;
			curTime = 0;
		}

		void process(float timeDelta) {
			if (!started || finished) return;

			processKeys();
			if (finished) return;

			curTime += timeDelta;

			if (phase == Phase::FadeIn) {
				if (curTime >= fadeTime) {
					curTime -= fadeTime;
					phase = Phase::Hold;
				}
			}
			else if (phase == Phase::Hold) {
				if (curTime >= holdTime) {
					curTime -= holdTime;
					phase = Phase::FadeOut;
				}
			}
			else { // Phase::FadeOut / Phase::Exiting
				if (curTime >= fadeOutDuration()) {
					curTime -= fadeOutDuration();
					if (phase == Phase::Exiting) {
						// The ESC fade-out is done: the intro finishes here.
						finished = true;
						return;
					}
					advance();
				}
			}
		}

		void processKeys() {
			// ESC: fast fade-out, the intro finishes only after it completes.
			if (phase != Phase::Exiting && IsKeyPressed(KEY_ESCAPE)) {
				phase = Phase::Exiting;
				curTime = 0;
				return;
			}

			// SPACE / ENTER / RIGHT: fade out the current frame and go to the next
			if (phase != Phase::Exiting &&
				(IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_RIGHT))) {
				phase = Phase::FadeOut;
				curTime = 0;
			}
		}

		// Duration of the current fade-out (Exiting is twice as fast).
		float fadeOutDuration() const {
			return (phase == Phase::Exiting) ? fadeTime * exitFadeScale : fadeTime;
		}

		// Current frame opacity: 0..1
		float getAlpha() {
			if (phase == Phase::FadeIn) {
				return (fadeTime > 0) ? Clamp(curTime / fadeTime, 0.0f, 1.0f) : 1.0f;
			}
			if (phase == Phase::Hold) {
				return 1.0f;
			}
			// Phase::FadeOut / Phase::Exiting
			float dur = fadeOutDuration();
			return (dur > 0) ? Clamp(1.0f - curTime / dur, 0.0f, 1.0f) : 0.0f;
		}

		// Draws directly into the current target (the final screen), skipping
		// the sceneTex -> brightness shader pass, which intro does not need.
		void render() {
			if (texture.id == 0) return;
			float screenW = this->resources->config.screenW;
			float screenH = this->resources->config.screenH;
			ClearBackground(BLACK);
			DrawTexturePro(
				texture,
				{ 0, 0, screenW, screenH },
				{ 0, 0, screenW, screenH },
				{ 0, 0 }, 0, Fade(WHITE, getAlpha())
			);
		}

	};

}
