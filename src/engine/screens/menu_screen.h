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
		saveGame,
		loadGame,
		exit
	};

	enum class MenuScreenState {
		main,
		save,
		load,
		options
	};

	class MenuScreen {
	public:
		World* world;
		Resources* resources;
		BaseMenu menu;
		BaseMenu savesMenu;
		bool firstFrame = true;

		MenuScreenState state = MenuScreenState::main;
		MenuScreenResult result = MenuScreenResult::none;
		int saveSlot = -1;

		MenuScreen(World* world) {
			this->world = world;
			this->resources = world->resources;
			menu.resources = this->resources;
			savesMenu.resources = this->resources;

			menu.maxRenderItems = 8;
			savesMenu.maxRenderItems = 8;
			for (int i = 0; i < 6; i++) {
				savesMenu.items.push_back({ i+1, string("Slot ") + to_string(i+1) });
			}
		}

		~MenuScreen() {
		}

		void reload() {
			state = MenuScreenState::main;
			result = MenuScreenResult::none;
			firstFrame = true;

			menu.middle = 1;
			menu.rect = { 0, 0, (float)resources->config.screenW, (float)resources->config.screenH };
			menu.items.clear();
			if (!world->gameOver) {
				menu.items.push_back({ 0, "Resume to Game" });
			}
			menu.items.push_back({ 1, "New Game" });
			if (!world->gameOver) {
				menu.items.push_back({ 2, "Save Game" });
			}
			menu.items.push_back({ 3, "Load Game" });
			menu.items.push_back({ 4, "Options" });
			menu.items.push_back({ 5, "Quit" });
			menu.reset();

			savesMenu.middle = 1;
			savesMenu.rect = { 0, resources->config.screenH * 0.1f, (float)resources->config.screenW, (float)resources->config.screenH };
			savesMenu.reset();
		}

		void submitMain() {
 			switch (menu.curItemId)
			{
			case 0:
				result = MenuScreenResult::resume;
				break;
			case 1:
				result = MenuScreenResult::newGame;
				break;
			case 2:
				state = MenuScreenState::save;
				break;
			case 3:
				state = MenuScreenState::load;
				break;
			case 5:
				result = MenuScreenResult::exit;
				break;
			}
		}

		void processKeys() {
			if (IsKeyPressed(KEY_ESCAPE)) {
				if (state != MenuScreenState::main) {
					state = MenuScreenState::main;
				}
				else if (!world->gameOver) {
					result = MenuScreenResult::resume;
				}
			}

			if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
				switch (state) {
				case MenuScreenState::main:
					submitMain();
					break;
				case MenuScreenState::save:
					saveSlot = savesMenu.curItemId;
					result = MenuScreenResult::saveGame;
					break;
				case MenuScreenState::load:
					saveSlot = savesMenu.curItemId;
					result = MenuScreenResult::loadGame;
					break;
				}
				
			}

			switch (state) {
			case MenuScreenState::main:
				menu.processKeys();
				break;
			case MenuScreenState::save:
				savesMenu.processKeys();
				break;
			case MenuScreenState::load:
				savesMenu.processKeys();
				break;
			}
		}

		void process(float timeDelta) {
			if (!firstFrame) {
				processKeys();
			}

			BeginDrawing();
			ClearBackground(BLACK);
			
			switch (state) {
			case MenuScreenState::main:
				menu.render();
				break;
			case MenuScreenState::save:
				resources->drawCentered("Save Game", {
					0, resources->config.screenH * 0.05f,
					(float)resources->config.screenW, 0
				}, WHITE);
				savesMenu.render();
				break;
			case MenuScreenState::load:
				resources->drawCentered("Load Game", {
					0, resources->config.screenH * 0.05f,
					(float)resources->config.screenW, 0
				}, WHITE);
				savesMenu.render();
				break;
			}

			EndDrawing();

			firstFrame = false;
		}

	};

}