#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../../common/raylib_cpp.hpp"
#include "./widgets/text.hpp"
#include "./book_screen.h"

using namespace std;
using namespace raylib;
namespace openAITD {

	class CharSelectScreen {
	public:
		World& world;
		Resources& resources;
		Texture2D pictureTex = { 0 };
		Texture2D bookTex = { 0 };
		TextWidget textWid;
		BookScreen book;
		bool selected = false;
		int selectedChar = -1;

		// Start animation: the screen fades in from black (same idea as IntroScreen).
		static constexpr float fadeInTime = 0.5f;
		float fadeTimer = 0.0f;

		// Smooth brightness change of each pictureTex half when selectedChar flips.
		// dimBrightness matches the old GRAY tint, activeBrightness the old WHITE one.
		static constexpr float dimBrightness = 0.4f;
		static constexpr float activeBrightness = 1.0f;
		static constexpr float brightnessSpeed = 2.0f; // brightness units per second
		float brightness[2] = { dimBrightness, dimBrightness };

		// Blinking of the active pictureTex (selectedChar), like ButtonWidget.
		static constexpr float blinkPeriod = 2.0f;
		static constexpr float blinkMin = 0.6f;
		float blinkTimer = 0.0f;

		bool exiting = false;
		bool exited = false;
		
		CharSelectScreen(World& world) :
		  world(world),
			resources(*world.resources),
			textWid(resources.texts.mainFont,{0}),
			book(world)
			{}

		~CharSelectScreen() {
		}

		Texture2D loadPicture() {
			string path = DataPath::GetFile("characters/0.png");
			if (path == "") {
				throw std::runtime_error("Picture characters/0.png not exists");
			}
			pictureTex = resources.backgrounds.loadImageResized(path);
		}

		void end() {
			if (pictureTex.id != 0) {
				UnloadTexture(pictureTex);
			}
		}

		void start() {
			loadPicture();
			auto& c = resources.config;
			exited = false;
			exiting = false;
			selected = false;
			selectedChar = -1;
			book.lastBookText = -1;
			// Fade in from black and show both previews dimmed until one is chosen.
			fadeTimer = 0.0f;
			blinkTimer = 0.0f;
			brightness[0] = dimBrightness;
			brightness[1] = dimBrightness;
		}

		// Advances the start fade-in, the smooth brightness and the blink timer.
		void updateAnim(float timeDelta) {
			if (fadeTimer < fadeInTime) {
				fadeTimer = std::min(fadeTimer + timeDelta, fadeInTime);
			}
			blinkTimer += timeDelta;
			if (blinkTimer > blinkPeriod) blinkTimer -= blinkPeriod;
			for (int i = 0; i < 2; i++) {
				float target = (selectedChar == i) ? activeBrightness : dimBrightness;
				brightness[i] = moveTowards(brightness[i], target, brightnessSpeed * timeDelta);
			}
		}

		static float moveTowards(float cur, float target, float maxDelta) {
			if (cur < target) return std::min(cur + maxDelta, target);
			return std::max(cur - maxDelta, target);
		}

		// Overall opacity of the start fade-in (0..1).
		float fadeAlpha() const {
			return (fadeInTime > 0.0f) ? Clamp(fadeTimer / fadeInTime, 0.0f, 1.0f) : 1.0f;
		}

		// Final tint of one pictureTex half, combining brightness, blink and fade-in.
		raylib::Color pictureTint(int i) const {
			float b = brightness[i];
			if (selectedChar == i) {
				float t = (1.0f - cosf(2.0f * PI * blinkTimer / blinkPeriod)) * 0.5f;
				b *= (blinkMin + (1.0f - blinkMin) * t);
			}
			return Fade(WHITE, Clamp(b * fadeAlpha(), 0.0f, 1.0f));
		}

		void process(float timeDelta) {
			updateAnim(timeDelta);
			if (exiting) {
				UnloadTexture(pictureTex);
				exited = true;
			} else if (selected) {
				// While a character is selected the nested BookScreen owns input/render.
				book.process(timeDelta);
				// The book has been read through (readText reset by BookScreen::exit) – leave.
				if (world.bookData.readText == -1) {
					exiting = true;
				}
			} else {
				processKeys();
			}
		}

    void processKeys() {
			// ESC: close the screen
			if (IsKeyPressed(KEY_ESCAPE)) {
				selectedChar = -1;
				exiting = true;
			}
			if (IsKeyPressed(KEY_LEFT)) {
				selectedChar = 0;
			}				
			if (IsKeyPressed(KEY_RIGHT)) {
				selectedChar = 1;
			}
			if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
				if (selectedChar != -1) {
					world.altModels = (selectedChar != 1);
					world.bookData = { 
						0,
						(world.altModels)?20:19, 
						(world.altModels)?1:0
				  };
					GameObject::altModels = world.altModels;
					selected = true;
				}
			}
		}

		// The character previews are treated as a scene: they are drawn into
		// sceneTex and then blitted through the brightness shader from main
		// (AppState::CharSelect), exactly like the in-world renderers.
		void renderScene() {
			BeginTextureMode(resources.screen.sceneTex);
			ClearBackground(BLACK);
			drawBackground();
			EndTextureMode();
		}

		// Draws both pictureTex halves with the current brightness/blink tint.
		void drawBackground() {
			float screenW = resources.config.screenW;
			float screenW2 = screenW / 2;
			float screenH = resources.config.screenH;
			DrawTexturePro(
				pictureTex,
				{ 0, 0, screenW2, screenH },
				{ 0, 0, screenW2, screenH },
				{ 0, 0 }, 0, pictureTint(0)
			);
			DrawTexturePro(
				pictureTex,
				{ screenW2, 0, screenW, screenH },
				{ screenW2, 0, screenW, screenH },
				{ 0, 0 }, 0, pictureTint(1)
			);
		}

	};

}