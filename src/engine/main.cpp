#pragma once
#include <vector>
#include <string>
#include "../platform/platform.h"
#include "./resources/resources.h"
#include "./world/world.h"
#include "./world/save_helper.h"

#include "./renderers/camera_renderer/camera_renderer.hpp"
#include "./renderers/freelook_renderer/freelook_renderer.hpp"
#include "./renderers/scene_renderer.hpp"

#include "./controllers/player_controller.h"
#include "./controllers/physics_controller.h"
#include "./controllers/camera_controller.h"
#include "./controllers/objrotate_controller.h"
#include "./controllers/animation_controller.h"
#include "./controllers/hit_controller.h"
#include "./controllers/shoot_controller.h"
#include "./controllers/throw_controller.h"
#include "./controllers/life_controller.h"
#include "./controllers/tracks_controller.h"
#include "./controllers/particle_controller.h"
#include "./controllers/shake_controller.hpp"

#include "./screens/found_screen.h"
#include "./screens/inventory_screen.h"
#include "./screens/menu_screen.h"
#include "./screens/picture_screen.h"
#include "./screens/intro_screen.h"
#include "./screens/book_screen.h"
#include "./screens/console_screen.h"
#include "./screens/char_select_screen.h"
#include "./screens/cutscene_screen.h"

#include "../extractor/include/extractor.h"

using namespace std;
namespace openAITD {

    const float inDarkBrightness = 0.02f;

    enum class AppState {
        StartIntro,
        Loading,
        CharSelect,
        GameIntro,
        InWorld,
        MainMenu,
        Inventory,
        Book,
        SelectGame,
        Cutscene
    };
    AppState state = AppState::StartIntro;
    bool gameStarted = false;
    
    Resources resources;
    World world(&resources);
    FoundScreen foundScreen(&world);
    CameraRenderer renderer(&world);
    FreelookRenderer flRenderer(&world);
    SceneRenderer sceneRend(world);
    
    ThrowController throwContr(&world);
    PhysicsController physContr(&resources, &world, &foundScreen);
    CameraController camContr(world);
    ObjRotateController objrotContr(&world);
    AnimationController animContr(&resources, &world);
    HitController hitContr(&world);
    ShootController shootContr(&world);
    PlayerController playerContr(&world);
    TracksController tracksContr(&world);
    ParticleController particleContr(world);
    ShakeController shakeContr(world);

    InventoryScreen inventoryScreen(&world);
    BookScreen bookScreen(world);
    PictureScreen pictureScr(&world);
    IntroScreen introScreen(&world);
    LifeController lifeContr(&world, &tracksContr, &playerContr, &hitContr, &throwContr, &physContr, &foundScreen, &shootContr);
    SaveHelper saveHelper(world);
    ConsoleScreen consoleScreen(&world);
    MenuScreen mainMenu(world, saveHelper);
    CharSelectScreen charSelectScreen(world);
    VideoScreen videoScreen(&world);

    bool freeLook = false;
    bool pause = false;
    const float maxDelta = 1.f / 30;

    void startGame() {
        world.gameOver = false;
        world.player.dead = false;
        world.loadVars("data/vars.json");
        world.loadGObjects("data/objects.json");
        world.setCurStage(0, 0);
        world.followTarget = 0;
        state = AppState::InWorld;
    }

    void startGameIntro() {
        startGame();
        world.setCurStage(7, 1);
        state = AppState::GameIntro;
    }

    void startStartIntro() {
        world.brightnessTrg = 1;
        introScreen.start();
        state = AppState::StartIntro;
    }

    // Starts a full-screen video clip from the given file (see World::Video).
    void startCutscene(const string& path) {
        if (path.empty()) return;
        if (!videoScreen.start(path)) {
            world.cutscene.active = false;
            world.cutscene.finished = true;
            return;
        }
        world.cutscene.active = true;
        world.cutscene.finished = false;
        world.cutscene.path = path;
        state = AppState::Cutscene;
    }

    void loadGame(int slot) {
        startGame();
        try {
            saveHelper.load(mainMenu.saveSlot);
        } catch (exception e) {
            cout << e.what() << endl;
        }
    }

    bool loadStage() {
        if (world.curStageId == world.nextStageId) return false;
        resources.screen.begin();
        auto& txt = resources.texts;
        string loadingText = txt.getEngineText("main.loading");
        int fontH = txt.mainFont.baseSize;
        raylib::Rectangle loadingRect = {
            0,
            (float)(resources.config.screenH - (fontH * 2)),
            (float)resources.config.screenW,
            (float)fontH
        };
        txt.drawCentered(loadingText.c_str(), loadingRect, WHITE);
        resources.screen.end();

        world.curStage = &resources.stages[world.nextStageId];
        world.curStageId = world.nextStageId;
        world.setCamera(-1);
        world.preload();

        //reset animation
        for (int i = 0; i < world.gobjects.size(); i++) {
            auto& gobj = world.gobjects[i];
            if (gobj.animation.id == -1) continue;
            gobj.animation.animTime = 0;
        }

        //reset frame time
        BeginDrawing();
        EndDrawing();

        return true;
    }

    void processBrightness(float timeDelta) {
        if (world.brightnessTrg != world.brightnessCur) {
            float diff = world.brightnessTrg - world.brightnessCur;
            float step = timeDelta * 4;
            if (fabs(diff) <= step) {
                world.brightnessCur = world.brightnessTrg;
            } else {
                world.brightnessCur += (diff > 0.0f ? step : -step);
            }
        }        
    }

    void renderMessage() {
        if (world.messageTime > 0) {
            auto& txt = resources.texts;
            const char* m = world.messageText.c_str();
            int& fontH = resources.texts.mainFont.baseSize;
            raylib::Rectangle messageRect = { 
                0,
                (float)(txt.config.screenH - (fontH * 2)),
                (float)txt.config.screenW,
                (float)fontH
            };
            messageRect.x += 2;
            messageRect.y += 2;
            txt.drawCentered(m, messageRect, BLACK);
            messageRect.x -= 2;
            messageRect.y -= 2;
            txt.drawCentered(m, messageRect, WHITE);

        }
    }

    void processWorld(float timeDelta) {
        if (world.picture.id != -1) {
            world.brightnessTrg = 1;
            pictureScr.process(timeDelta);
            return;
        }
        if (world.curStageId != world.nextStageId) {
            if (world.brightnessCur > 0) {
                world.brightnessTrg = 0;
                return;
            } else {
                loadStage();
            }
        }
        world.setCurRoom(world.nextRoomId);
        
        if (IsKeyPressed(KEY_O)) {
            freeLook = !freeLook;
        }
        if (IsKeyPressed(KEY_GRAVE)) {
            consoleScreen.start();
        }
        if (IsKeyPressed(KEY_P)) {
            pause = !pause;
        }
        if (IsKeyDown(KEY_I)) {
            timeDelta *= 8;
        }
        if (!pause) {
            if (world.inDark) {
                world.brightnessTrg = inDarkBrightness;
            } else {
                world.brightnessTrg = 1;
            }

            while (true) {

                float partDelta = min(timeDelta, maxDelta);
                world.chrono += partDelta;
                lifeContr.process(partDelta);
                animContr.process(partDelta);
                hitContr.process(partDelta);
                shootContr.process();
                throwContr.process(partDelta);
                physContr.process(partDelta);
                particleContr.process(partDelta);
                shakeContr.process(partDelta);
                camContr.process();
                objrotContr.process(partDelta);
                if (world.messageTime > 0) {
                    world.messageTime -= partDelta;
                }

                timeDelta -= maxDelta;
                if (timeDelta < 0) break;
            }
        } else if (!freeLook) {
            world.brightnessTrg = 0.5;
        }
        if (freeLook) {
            world.brightnessTrg = 1;
            flRenderer.freeLook = pause;
            flRenderer.process();
        }
    }

    void renderWorld() {
        if (freeLook) {
            flRenderer.render();
        } else if (world.picture.id != -1) {
            pictureScr.render();
        } else {
            renderer.render();
        }
        resources.screen.begin();
        sceneRend.render();
        renderMessage();
        resources.screen.end();
    }

    void startMenu() {
        mainMenu.reload();
        state = AppState::MainMenu;
    }

    bool processMenu(const float timeDelta) {
        mainMenu.process(timeDelta);
        switch (mainMenu.result)
        {
        case MenuScreenResult::exit:
            return false;
            break;
        case MenuScreenResult::newGame:
            charSelectScreen.start();
            state = AppState::CharSelect;
            break;
        case MenuScreenResult::resume:
            state = AppState::InWorld;
            break;
        case MenuScreenResult::saveGame:
            saveHelper.save(mainMenu.saveSlot);
            world.messageText = "Game saved...";
            world.messageTime = 2;
            state = AppState::InWorld;
            break;
        case MenuScreenResult::loadGame:
            loadGame(mainMenu.saveSlot);
            break;
        }
        mainMenu.result = MenuScreenResult::none;
        return true;
    }

    bool process(float timeDelta) {
        // The video source is a file: an .ogv dropped onto the window starts playback.
        if (IsFileDropped()) {
            FilePathList dropped = LoadDroppedFiles();
            if (dropped.count > 0 && dropped.paths[0] != nullptr) {
                world.cutscene.path = dropped.paths[0];
                world.cutscene.request = true;
            }
            UnloadDroppedFiles(dropped);
        }

        // Video can also be requested from anywhere by setting World::Video::request.
        if (world.cutscene.request && state != AppState::Cutscene) {
            world.cutscene.request = false;
            startCutscene(world.cutscene.path);
        }

        if (state == AppState::Cutscene) {
            world.brightnessTrg = 1;
            videoScreen.process(timeDelta);
            if (IsKeyPressed(KEY_ESCAPE) || videoScreen.isFinished()) {
                videoScreen.stop();
                world.cutscene.active = false;
                world.cutscene.finished = true;
                state = AppState::InWorld;
            }
        }
        else if (state == AppState::MainMenu) {
            world.brightnessTrg = world.inDark ? inDarkBrightness : 0.1f;
            if (!processMenu(timeDelta)) return false;
        }
        else if (state == AppState::CharSelect) {
            charSelectScreen.process(timeDelta);
            if (charSelectScreen.exited) {
                if (charSelectScreen.selectedChar != -1) {
                    startGameIntro();                    
                } else {
                    state = AppState::MainMenu;
                }
            }
        }
        else if (state == AppState::StartIntro) {
            world.brightnessTrg = 1;
            // ESC is handled inside IntroScreen: it fades the current frame out
            // first and only then sets finished.
            introScreen.process(timeDelta);
            if (introScreen.finished) {
                introScreen.end();
                if (resources.config.fastStart) {
                    startGame();
                } else {
                    startMenu();
                }
            }
        }
        else if (state == AppState::GameIntro) {
            processWorld(timeDelta);
            if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ESCAPE)) {
                world.gameOver = true;
            }
            if (world.gameOver) {
                startGame();
            }
        }
        else if (state == AppState::InWorld) {
            if (world.bookData.readText != -1) {
                world.brightnessTrg = 1;
                bookScreen.process(timeDelta);
            }            
            else if (world.player.allowInventory && IsKeyPressed(KEY_ENTER)) {
                inventoryScreen.reload();
                state = AppState::Inventory;
            }
            else if (IsKeyPressed(KEY_ESCAPE)) {
                startMenu();
            }
            else if (world.picture.id != -1) {
                pictureScr.process(timeDelta);
            }
            else if (world.gameOver) {
                mainMenu.reload();
                state = AppState::MainMenu;
            }
            else {
                world.player.space = IsKeyDown(KEY_SPACE);
                if (IsKeyDown(KEY_UP)) {
                    world.player.keyboard = 1;
                }
                else if (IsKeyDown(KEY_DOWN)) {
                    world.player.keyboard = 2;
                }
                else if (IsKeyDown(KEY_LEFT)) {
                    world.player.keyboard = 4;
                }
                else if (IsKeyDown(KEY_RIGHT)) {
                    world.player.keyboard = 8;
                }
                else {
                    world.player.keyboard = 0;
                }
                world.player.space = IsKeyDown(KEY_SPACE);
                processWorld(timeDelta);
            }
        }
        else if (state == AppState::Inventory) {
            if (!inventoryScreen.exitting) {
                world.brightnessTrg = world.inDark ? inDarkBrightness : 0.2f;
            } else {
                world.brightnessTrg = world.inDark ? inDarkBrightness : 1.0f;
            }
            inventoryScreen.process(timeDelta);
            if (inventoryScreen.exit) {
                state = AppState::InWorld;
            }
        }
        processBrightness(timeDelta);        
        return true;
    }

    void render() {
        if (state == AppState::MainMenu) {
            resources.screen.begin();
            sceneRend.render();
            mainMenu.render();
            resources.screen.end();
        }
        else if (state != AppState::CharSelect && world.bookData.readText != -1) {
            resources.screen.begin();
            bookScreen.render();
            resources.screen.end();
        } 
        else if (state == AppState::CharSelect) {
            resources.screen.begin();
            charSelectScreen.render();
            resources.screen.end();
        }
        else if (state == AppState::StartIntro) {
            // Intro draws straight to the screen
            resources.screen.begin();
            introScreen.render();
            resources.screen.end();
        }
        else if (state == AppState::Cutscene) {
            // Video draws straight to the screen: no sceneTex -> brightness
            resources.screen.begin();
            videoScreen.render();
            resources.screen.end();
        }
        else if (state == AppState::GameIntro) {
            renderWorld();
        }
        else if (state == AppState::InWorld) {
            renderWorld();
        }
        else if (state == AppState::Inventory) {
            resources.screen.begin();
            sceneRend.render();
            inventoryScreen.render();
            resources.screen.end();
        }
        resources.audio.Process();
    }

    int main(void)
    {
        resources.config = loadConfig();

        resources.screen.init();
        sceneRend.init();
        SetExitKey(KEY_F10);

        auto extractor =  AITDExtractor::createAITDExtractor();
        extractor->extractAllData(false);

        resources.texts.load();

        resources.stages.resize(8);
        for (int i = 0; i < 8; i++) {
            resources.stages[i].load(string("data/stages/")+to_string(i));
        }
        resources.loadTracks("data/tracks", "newdata/tracks");

        DisableCursor();

        startStartIntro();

        float timeDelta = 0;        
        while (!WindowShouldClose()) {
            timeDelta = GetFrameTime();
            if (!process(timeDelta)) break;
            render();
        }

        return 0;
    }
}

int main(void)
{
    return openAITD::main();
}