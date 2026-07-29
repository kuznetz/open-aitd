#pragma once
#include <vector>
#include <string>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../../raylib-cpp/raylib-cpp.h"

using namespace std;
using namespace raylib;
namespace openAITD {

	struct BookType {
		int pictureId;
	};

	inline const std::array<BookType, 3> bookTypes = {{
			{6},
			{7},
			{8}
	}};

	class BookScreen {
	public:
		World& world;
		Resources& resources;
		BookData& bookData;
		int lastBookText = -1;
		
		BookScreen(World& world) :
		  world(world),
			resources(*world.resources),
			bookData(world.bookData)
			{}

		~BookScreen() {
		}

		void reload() {
			string text = resources.loadBookText(bookData.readText);			
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
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
					exit();
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
		}

	};

}