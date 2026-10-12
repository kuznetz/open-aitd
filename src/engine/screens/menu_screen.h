#pragma once
#include <vector>
#include <string>
#include "../../common/raylib_cpp.hpp"
#include "../world/world.h"
#include "../resources/resources.h"
#include "./options_screen.h"
#include "./saves_screen.h"
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
        SavesScreen savesScreen;
        OptionsScreen options;
        bool firstFrame = true;

        MenuScreenState state = MenuScreenState::main;
        MenuScreenResult result = MenuScreenResult::none;

        MenuScreen(World& world, SaveHelper& saveHelper)
            : world(world),
            resources(*world.resources),
            options(world),
            mainMenu(resources.texts.mainFont, raylib::Rectangle{0,0,1,1}, 10),
            savesScreen(resources, saveHelper)
        {}

        ~MenuScreen() = default;

        void reload() {
            state = MenuScreenState::main;
            result = MenuScreenResult::none;
            firstFrame = true;

            mainMenu.bounds = { 0, 0, (float)resources.config.screenW, (float)resources.config.screenH };

            vector<VerticalMenuWidget::Item> mainItems;
            if (!world.gameOver) {
                mainItems.push_back({ MenuResume, _T("menu.resume") });
            }
            mainItems.push_back({ MenuNewGame, _T("menu.newGame") });
            //Saving is not allowed while the player is dead
            if (!world.gameOver && !world.player.dead) {
                mainItems.push_back({ MenuSaveGame, _T("menu.saveGame") });
            }
            mainItems.push_back({ MenuLoadGame, _T("menu.loadGame") });
            mainItems.push_back({ MenuOptions, _T("menu.options") });
            mainItems.push_back({ MenuQuit, _T("menu.quit") });
            mainMenu.setItemsExt(mainItems);
            mainMenu.setSelectedIndex(0);
        }

        void submitMain() {
            switch (static_cast<MainItemId>(mainMenu.getSelectedId())) {
                case MenuResume:   result = MenuScreenResult::resume; break;
                case MenuNewGame:  result = MenuScreenResult::newGame; break;
                case MenuSaveGame: state = MenuScreenState::save; savesScreen.reload(SavesScreen::Mode::Save); break;
                case MenuLoadGame: state = MenuScreenState::load; savesScreen.reload(SavesScreen::Mode::Load); break;
                case MenuOptions:  state = MenuScreenState::options; options.reload(); break;
                case MenuQuit:     result = MenuScreenResult::exit; break;
                default: break;
            }
        }

        void processKeys() {
            if (IsKeyPressed(KEY_ESCAPE)) {
                if (state == MenuScreenState::save || state == MenuScreenState::load) {
                    state = MenuScreenState::main;
                } else if (state == MenuScreenState::options) {
                    state = MenuScreenState::main;
                    reload();
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
                savesScreen.processKeys();
                if (savesScreen.isComplete()) {
                    int slot = savesScreen.getSelectedSlot();
                    if (slot != -1) {
                        if (state == MenuScreenState::save)
                            result = MenuScreenResult::saveGame;
                        else
                            result = MenuScreenResult::loadGame;
                        saveSlot = slot;
                    }
                    state = MenuScreenState::main;
                    savesScreen.resetComplete();
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
            if (state == MenuScreenState::save || state == MenuScreenState::load) {
                savesScreen.process(timeDelta);
            } else if (state == MenuScreenState::options) {
                options.process(timeDelta);
            }
        }

        void render() {
            switch (state) {
                case MenuScreenState::main:
                    mainMenu.draw();
                    break;
                case MenuScreenState::save:
                case MenuScreenState::load:
                    savesScreen.render();
                    break;
                case MenuScreenState::options:
                    options.render();
                    break;
            }
        }

        int saveSlot = -1;

    private:

        enum MainItemId {
            MenuResume = 1,
            MenuNewGame,
            MenuSaveGame,
            MenuLoadGame,
            MenuOptions,
            MenuQuit,
        };

        string _T(const string& key) {
            return this->resources.texts.getEngineText(key);
        }
    };

} // namespace openAITD