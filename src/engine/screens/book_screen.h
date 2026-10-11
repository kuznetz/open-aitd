#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../../common/raylib_cpp.hpp"
#include "./widgets/text.hpp"

using namespace std;
using namespace raylib;
namespace openAITD {

	struct BookType {
		int pictureId;
		raylib::Rectangle bounds;
		raylib::Rectangle pages;
	};

	inline const std::array<BookType, 3> bookTypes = {{
			{
				6,
				{60/320.f, 10/200.f, (245-60)/320.f, (190-10)/200.f},
				{60/320.f, 185/200.f, (245-60)/320.f, 1.0f} 
			}, // READ_MESSAGE
			{
				7, 
				{48/320.f, 2/200.f, (260-48)/320.f, (197-2)/200.f},
				{60/320.f, 185/200.f, (245-60)/320.f, 1.0f}
			}, // READ_BOOK
			{
				8, 
				{50/320.f, 20/200.f, (250-50)/320.f, (199-20)/200.f},
				{60/320.f, 185/200.f, (245-60)/320.f, 1.0f}
			}  // READ_CARNET
	}};

	class BookScreen {
	public:
		World& world;
		Resources& resources;
		BookData& bookData;
		TextWidget text;
		int lastBookText = -1;		

		// Book animations: slide-in on open, slide-out before close, page flip wipe.
		enum class Anim { None, Opening, Closing, FlipNext, FlipPrev };

		Anim anim = Anim::None;
		float animTime = 0.0f;
		int flipFromPage = -1;   // page drawn on the left of the flip line (FlipNext)
		int flipToPage = -1;     // page drawn on the right of the flip line (FlipNext)

		// Durations (seconds)
		static constexpr float openTime = 0.35f;
		static constexpr float closeTime = 0.30f;
		static constexpr float flipTime = 0.30f;

		// Sound played when the book opens and on every page flip.
		static constexpr int pageSoundId = 49;

		// Color of the flip line separating the old and the new page.
		raylib::Color flipLineColor = { 0, 0, 0, 150 };

		BookScreen(World& world) :
		  world(world),
			resources(*world.resources),
			bookData(world.bookData),
			text(resources.texts.mainFont, {})
			{}

		~BookScreen() {
			// If the screen goes away while the book is still open, make sure the
			// voice-over stops and the background music resumes.
			resources.audio.StopAudioBook();
		}

		void reload() {
			auto& c = resources.config;
			auto& b = bookTypes[bookData.bookType].bounds;
			text.setBounds({ b.x * c.screenW, b.y * c.screenH, b.width * c.screenW, b.height * c.screenH });
			string textSrc = resources.texts.getBookText(bookData.readText);			
			text.setText(textSrc);
			lastBookText = bookData.readText;			
		}

		void process(float timeDelta) {
			if (lastBookText != bookData.readText) {
				// Opening a book (closed -> a valid text id) plays the slide-in.
				bool opening = (lastBookText == -1 && bookData.readText != -1);
				reload();
				anim = opening ? Anim::Opening : Anim::None;
				animTime = 0.0f;
				if (opening) {
					resources.audio.PlaySound(pageSoundId);
					// Start the audio book voice-over for the first page (if any).
					startAudioBook();
				}
			}
			processKeys();
			updateAnim(timeDelta);
		}

		// Plays the audio book voice-over for the currently shown page.
		// bookData.readAudio < 0 means the book has no narration.
		void startAudioBook() {
			if (bookData.readAudio < 0) return;
			resources.audio.PlayAudioBook(bookData.readAudio, text.getCurrentPage());
		}

		// Advances the current animation and finishes it once the duration is over.
		void updateAnim(float timeDelta) {
			if (anim == Anim::None) return;
			animTime += timeDelta;
			if (animTime < animDuration()) return;

			if (anim == Anim::Closing) {
				// The slide-out is done: release the book so the world resumes.
				// Stop the narration and resume the music that was playing before it.
				resources.audio.StopAudioBook();
				bookData.readText = -1;
				lastBookText = -1;
			}
			anim = Anim::None;
			animTime = 0.0f;
		}

		float animDuration() const {
			switch (anim) {
				case Anim::Opening: return openTime;
				case Anim::Closing: return closeTime;
				case Anim::FlipNext:
				case Anim::FlipPrev: return flipTime;
				default: return 1.0f;
			}
		}

		// Linear animation progress in 0..1 (1 when idle).
		float animProgress() const {
			if (anim == Anim::None) return 1.0f;
			float d = animDuration();
			return d > 0.0f ? Clamp(animTime / d, 0.0f, 1.0f) : 1.0f;
		}

		void processKeys() {
			// No input while the book is sliding in or out.
			if (anim == Anim::Opening || anim == Anim::Closing) return;

			// ESC: close the screen (plays the slide-out animation first)
			if (IsKeyPressed(KEY_ESCAPE)) {
				exit();
				return;
			}
			// ENTER / SPACE / RIGHT: next page
			if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_RIGHT)) {
				if (text.hasNext()) {
					startFlip(1);
				} else {
					exit();
				}
			}
			// LEFT: previous page
			if (IsKeyPressed(KEY_LEFT)) {
				if (text.hasPrev()) startFlip(-1);
			}
		}

		// Starts the page-flip wipe. dir > 0 – forward, dir < 0 – backward.
		void startFlip(int dir) {
			if (anim == Anim::FlipNext || anim == Anim::FlipPrev) return; // already flipping
			int from = text.getCurrentPage();
			if (dir > 0) text.nextPage(); else text.prevPage();
			int to = text.getCurrentPage();
			if (to == from) return;
			flipFromPage = from;
			flipToPage = to;
			anim = (dir > 0) ? Anim::FlipNext : Anim::FlipPrev;
			animTime = 0.0f;
			resources.audio.PlaySound(pageSoundId, 0.1);
			// Voice-over for the page we flipped to.
			startAudioBook();
		}

		// Starts the slide-out animation. The book is actually released
		// (bookData.readText = -1) once the animation finishes in updateAnim().
		void exit() {
			if (anim == Anim::Closing) return;
			anim = Anim::Closing;
			animTime = 0.0f;
		}

		void render() {
			auto& texture = this->resources.backgrounds.loadPicture(bookTypes[bookData.bookType].pictureId);
			auto& c = resources.config;
			float screenW = (float)c.screenW;
			float screenH = (float)c.screenH;

			// Vertical offset of the whole book during slide-in / slide-out.
			float offsetY = 0.0f;
			if (anim == Anim::Opening) {
				offsetY = (1.0f - animProgress()) * screenH; // slides up from below
			} else if (anim == Anim::Closing) {
				offsetY = animProgress() * screenH;          // slides down out of view
			}

			DrawTexturePro(
				texture,
				{ 0, 0, screenW, screenH },
				{ 0, offsetY, screenW, screenH },
				{ 0, 0 }, 0, WHITE
			);

			if (anim == Anim::FlipNext || anim == Anim::FlipPrev) {
				drawFlip(offsetY);
			} else {
				text.draw(0.0f, offsetY);
			}
			drawPageNumber(offsetY);
		}

		// Page-flip wipe: a vertical line sweeps across the pages. The old page
		// stays on one side of the line while the new one is revealed on the other.
		void drawFlip(float offsetY) {
			auto& c = resources.config;
			auto& b = bookTypes[bookData.bookType].bounds;
			float left = b.x * c.screenW;
			float top = b.y * c.screenH + offsetY;
			float width = b.width * c.screenW;
			float height = b.height * c.screenH;
			float p = animProgress();

			// Forward: the line sweeps right -> left, the previous page stays on the
			// left and the new one appears on the right. Backward: mirrored.
			float lineX;
			int leftPage, rightPage;
			if (anim == Anim::FlipNext) {
				lineX = left + (1.0f - p) * width;
				leftPage = flipFromPage;   // previous page
				rightPage = flipToPage;    // current (new) page
			} else {
				lineX = left + p * width;
				leftPage = flipToPage;     // new page comes from the left
				rightPage = flipFromPage;  // previous page
			}

			// Left of the line
			float leftW = lineX - left;
			if (leftW > 0.0f) {
				BeginScissorMode((int)left, (int)top, (int)leftW, (int)height);
				text.drawPage(leftPage, 0.0f, offsetY);
				EndScissorMode();
			}

			// Right of the line
			float rightW = left + width - lineX;
			if (rightW > 0.0f) {
				BeginScissorMode((int)lineX, (int)top, (int)rightW, (int)height);
				text.drawPage(rightPage, 0.0f, offsetY);
				EndScissorMode();
			}

			// The line itself
			DrawRectangle((int)lineX - 1, (int)top, 2, (int)height, flipLineColor);
		}

		// Draws the page indicator "{n}/{total}" centered in the BookType.pages area.
		// Nothing is drawn when the text fits on a single page.
		void drawPageNumber(float offsetY = 0.0f) {
			int total = text.getPageCount();
			if (total <= 1) return;
			auto& c = resources.config;
			auto& p = bookTypes[bookData.bookType].pages;
			raylib::Rectangle area = {
				p.x * c.screenW,
				p.y * c.screenH + offsetY,
				p.width * c.screenW,
				p.height * c.screenH
			};
			int pageIndex = (anim == Anim::FlipNext || anim == Anim::FlipPrev)
				? flipToPage : text.getCurrentPage();
			string label = to_string(pageIndex + 1) + "/" + to_string(total);
			resources.texts.drawCentered(label.c_str(), area, BLACK);
		}

	};

}
