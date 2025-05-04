#pragma once
#include <vector>
#include <string>
#include "../raylib.h"
#include "../world/world.h"
#include "../resources/resources.h"
#include "./base_menu.h"

using namespace std;
using namespace raylib;
namespace openAITD {

	enum class MenuScreenResult {
		none,
		resume,
		newGame,
		exit
	};

	class MenuScreen {
	public:
		World* world;
		Resources* resources;
		BaseMenu menu;
		bool firstFrame = true;

		vector<int> curActions;
		MenuScreenResult result = MenuScreenResult::none;

		MenuScreen(World* world) {
			this->world = world;
			this->resources = world->resources;
			menu.resources = this->resources;
		}

		~MenuScreen() {
		}

		void reload() {
			result = MenuScreenResult::none;
			firstFrame = true;
			menu.middle = 1;
			menu.rect = { 0, 0, (float)resources->config.screenW, (float)resources->config.screenH };
			menu.items.clear();
			menu.items.push_back({ 0, "New Game" });
			menu.items.push_back({ 1, "Continue Game" });
			//menu.items.push_back("Save Game");
			menu.items.push_back({ 4, "Options" });
			menu.items.push_back({ 5, "Quit" });

			menu.reset();
		}

		void submit() {
 			switch (menu.curItemId)
			{
			case 0:
				result = MenuScreenResult::newGame;
				break;
			case 5:
				result = MenuScreenResult::exit;
				break;
			}
		}

		void processKeys() {
			if (IsKeyPressed(KEY_ESCAPE)) {
				result = MenuScreenResult::resume;
			}
			if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
				submit();
			}
			menu.processKeys();
		}

		void process(float timeDelta) {
			if (!firstFrame) {
				processKeys();
			}

			BeginDrawing();
			ClearBackground(BLACK);
			menu.render();
			EndDrawing();

			firstFrame = false;
		}

	};

}