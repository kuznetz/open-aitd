#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../../raylib-cpp/raylib-cpp.h"
#include "./widgets/text.hpp"

using namespace std;
using namespace raylib;
namespace openAITD {

	struct BookType {
		int pictureId;
		raylib::Rectangle bounds;
	};

	inline const std::array<BookType, 3> bookTypes = {{
			{6, {60/320.f, 10/200.f, (245-60)/320.f, (190-10)/200.f} }, // READ_MESSAGE
			{7, {48/320.f, 2/200.f, (260-48)/320.f, (197-2)/200.f} }, // READ_BOOK
			{8, {50/320.f, 20/200.f, (250-50)/320.f, (199-20)/200.f} }  // READ_CARNET
	}};

	class BookScreen {
	public:
		World& world;
		Resources& resources;
		BookData& bookData;
		TextWidget text;
		int lastBookText = -1;		
		
		BookScreen(World& world) :
		  world(world),
			resources(*world.resources),
			bookData(world.bookData),
			text(resources.screen.mainFont, {})
			{}

		~BookScreen() {
		}

		void reload() {
			auto& c = resources.config;
			auto& b = bookTypes[bookData.bookType].bounds;
			text.setBounds({ b.x * c.screenW, b.y * c.screenH, b.width * c.screenW, b.height * c.screenH });
			string textSrc = resources.loadBookText(bookData.readText);			
			text.setText(textSrc);
			lastBookText = bookData.readText;			
		}

		void process(float timeDelta) {
			if (lastBookText != bookData.readText) {
				reload();
			}
			processKeys();
		}

    void processKeys() {
        // ESC: close the screen
        if (IsKeyPressed(KEY_ESCAPE)) {
					exit();
        }
        // ENTER / SPACE: next page
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_RIGHT)) {
					if (text.hasNext()) {
						text.nextPage();
					} else {
						exit();
					}
        }
				// Prev Page
        if (IsKeyPressed(KEY_LEFT)) {
					if (text.hasPrev()) text.prevPage();
        }
    }

		void exit() {
			bookData.readText = -1;
		}

		void render() {
			auto& texture = this->resources.backgrounds.loadPicture(bookTypes[bookData.bookType].pictureId);
			auto& c = resources.config;
			DrawTexturePro(
				texture,
				{ 0, 0, (float)c.screenW, (float)c.screenH },
				{ 0, 0, (float)c.screenW, (float)c.screenH },
				{ 0, 0 }, 0, WHITE
			);
			text.draw();
		}

	};

}