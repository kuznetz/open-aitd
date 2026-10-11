#pragma once
#include <vector>
#include <string>
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
		const raylib::Rectangle textRect = {60/320.f, 10/200.f, (245-60)/320.f, (190-10)/200.f};

		World& world;
		Resources& resources;
		Texture2D pictureTex = { 0 };
		Texture2D bookTex = { 0 };
		TextWidget textWid;
		BookScreen book;
		bool selected = false;
		int selectedChar = -1;

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
			auto& b = textRect;
			auto& c = resources.config;
			textWid.setBounds({ b.x * c.screenW, b.y * c.screenH, b.width * c.screenW, b.height * c.screenH });
			exited = false;
			exiting = false;
			selected = false;
			selectedChar = -1;
			book.lastBookText = -1;
		}

		void process(float timeDelta) {
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
					world.bookData = { 0, (world.altModels)?20:19, (world.altModels)?19:18};
					GameObject::altModels = world.altModels;
					selected = true;
				}
			}
		}

		void render() {
			if (selected) {
				book.render();
			} else {
				float screenW = resources.config.screenW;
				float screenW2 = screenW / 2;
				float screenH = resources.config.screenH;
				DrawTexturePro(
					pictureTex,
					{ 0, 0, screenW2, screenH },
					{ 0, 0, screenW2, screenH },
					{ 0, 0 }, 0, selectedChar == 0? WHITE: GRAY
				);
				DrawTexturePro(
					pictureTex,
					{ screenW2, 0, screenW, screenH },
					{ screenW2, 0, screenW, screenH },
					{ 0, 0 }, 0, selectedChar == 1? WHITE: GRAY
				);				
			}
		}

	};

}