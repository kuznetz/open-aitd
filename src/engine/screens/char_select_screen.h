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

	class CharSelectScreen {
	public:
		const raylib::Rectangle textRect = {60/320.f, 10/200.f, (245-60)/320.f, (190-10)/200.f};

		World& world;
		Resources& resources;
		Texture2D pictureTex = { 0 };
		Texture2D bookTex = { 0 };
		TextWidget textWid;
		int selected = 0;

		bool exiting = false;
		bool exited = false;
		
		CharSelectScreen(World& world) :
		  world(world),
			resources(*world.resources),
			textWid(resources.screen.mainFont,{0})
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
			selected = -1;
		}

		void process(float timeDelta) {
			if (exiting) {
				UnloadTexture(pictureTex);
				exited = true;
			} else {
				processKeys();
			}
		}

    void processKeys() {
        // ESC: close the screen
        if (IsKeyPressed(KEY_ESCAPE)) {
					selected = -1;
					exiting = true;
        }
        if (IsKeyPressed(KEY_LEFT)) {
					selected = 0;
        }				
        if (IsKeyPressed(KEY_RIGHT)) {
					selected = 1;
        }
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
					if (selected != -1) {
						world.altModels = (selected != 1);
						GameObject::altModels = world.altModels;
						exiting = true;
					}
        }
    }

		void render() {
			float screenW = resources.config.screenW;
			float screenW2 = screenW / 2;
			float screenH = resources.config.screenH;
			DrawTexturePro(
				pictureTex,
				{ 0, 0, screenW2, screenH },
				{ 0, 0, screenW2, screenH },
				{ 0, 0 }, 0, selected == 0? WHITE: GRAY
			);
			DrawTexturePro(
				pictureTex,
				{ screenW2, 0, screenW, screenH },
				{ screenW2, 0, screenW, screenH },
				{ 0, 0 }, 0, selected == 1? WHITE: GRAY
			);
		}

	};

}