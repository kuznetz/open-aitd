#pragma once
#include <vector>
#include <string>
#include "../../raylib-cpp/raylib-cpp.h"
#include "../world/world.h"
#include "../resources/resources.h"
#include "./options_screen.h"
#include "./widgets/vertical_menu.hpp"

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
        World& world;
        Resources& resources;
        VerticalMenuWidget mainMenu;
        VerticalMenuWidget savesMenu;
        OptionsScreen options;
        bool firstFrame = true;

        MenuScreenState state = MenuScreenState::main;
        MenuScreenResult result = MenuScreenResult::none;
        int saveSlot = -1;

        MenuScreen(World& world) :
					world(world),
					resources(*world.resources),
					options(world),
					mainMenu(resources.screen.mainFont, raylib::Rectangle{0,0,1,1}, 10),
					savesMenu(resources.screen.mainFont, raylib::Rectangle{0,0,1,1}, 10)
        {}

        ~MenuScreen() = default;

        void reload() {
            state = MenuScreenState::main;
            result = MenuScreenResult::none;
            firstFrame = true;

            mainMenu.bounds = { 0, 0, (float)resources.config.screenW, (float)resources.config.screenH };
            savesMenu.bounds = { 0, resources.config.screenH * 0.1f,
                                 (float)resources.config.screenW, (float)resources.config.screenH };

            vector<string> mainItems;
            if (!world.gameOver) {
                mainItems.push_back("Resume to Game");
            }
            mainItems.push_back("New Game");
            if (!world.gameOver) {
                mainItems.push_back("Save Game");
            }
            mainItems.push_back("Load Game");
            mainItems.push_back("Options");
            mainItems.push_back("Quit");
            mainMenu.setItems(mainItems);

            vector<string> saveItems;
            for (int i = 0; i < 6; ++i) {
                saveItems.push_back("Slot " + to_string(i + 1));
            }
            savesMenu.setItems(saveItems);

            mainMenu.setSelectedIndex(0);
            savesMenu.setSelectedIndex(0);
            world.brightnessTrg = 0.1;
        }

        void submitMain() {
            int idx = mainMenu.getSelectedIndex();
            if (!world.gameOver) {
                switch (idx) {
                    case 0: result = MenuScreenResult::resume; break;
                    case 1: result = MenuScreenResult::newGame; break;
                    case 2: state = MenuScreenState::save; break;
                    case 3: state = MenuScreenState::load; break;
                    case 4: state = MenuScreenState::options; options.reload(); break;
                    case 5: result = MenuScreenResult::exit; break;
                }
            } else {
                switch (idx) {
                    case 0: result = MenuScreenResult::newGame; break;
                    case 1: state = MenuScreenState::load; break;
                    case 2: state = MenuScreenState::options; options.reload(); break;
                    case 3: result = MenuScreenResult::exit; break;
                }
            }
        }

        void processKeys() {
            if (IsKeyPressed(KEY_ESCAPE)) {
                if (state != MenuScreenState::main) {
                    state = MenuScreenState::main;
                } else if (!world.gameOver) {
                    result = MenuScreenResult::resume;
                }
            }

            if (state == MenuScreenState::main) {
                if (IsKeyPressed(KEY_UP)) mainMenu.moveUp();
                else if (IsKeyPressed(KEY_DOWN)) mainMenu.moveDown();

                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                    submitMain();
                }
            }
            else if (state == MenuScreenState::save || state == MenuScreenState::load) {
                if (IsKeyPressed(KEY_UP)) savesMenu.moveUp();
                else if (IsKeyPressed(KEY_DOWN)) savesMenu.moveDown();

                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                    saveSlot = savesMenu.getSelectedIndex() + 1;   // слоты нумеруются с 1
                    result = (state == MenuScreenState::save)
                                ? MenuScreenResult::saveGame
                                : MenuScreenResult::loadGame;
                }
            }
            else if (state == MenuScreenState::options) {
                options.processKeys();
            }
        }

        void process(float timeDelta) {
            if (!firstFrame) {
                processKeys();
            }
            firstFrame = false;

            mainMenu.process(timeDelta);
            savesMenu.process(timeDelta);
            if (state == MenuScreenState::options) {
                options.process(timeDelta);
            }
        }

        void render() {
            switch (state) {
                case MenuScreenState::main:
                    mainMenu.draw();
                    break;
                case MenuScreenState::save:
                    resources.screen.drawCentered("Save Game", {
                        0, resources.config.screenH * 0.05f,
                        (float)resources.config.screenW, 0
                    }, WHITE);
                    savesMenu.draw();
                    break;
                case MenuScreenState::load:
                    resources.screen.drawCentered("Load Game", {
                        0, resources.config.screenH * 0.05f,
                        (float)resources.config.screenW, 0
                    }, WHITE);
                    savesMenu.draw();
                    break;
                case MenuScreenState::options:
                    options.render();
                    break;
            }
        }
    };

} // namespace openAITD