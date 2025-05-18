#pragma once
#include <vector>
#include <string>
#include "./resources/resources.h"
#include "./world/world.h"
#include "./controllers/camera_renderer.h"
#include "./controllers/freelook_renderer.h"
#include "./controllers/player_controller.h"
#include "./controllers/physics_controller.h"
#include "./controllers/objrotate_controller.h"
#include "./controllers/animation_controller.h"
#include "./controllers/hit_controller.h"
#include "./controllers/throw_controller.h"
#include "./controllers/life_controller.h"
#include "./controllers/tracks_controller.h"
#include "./controllers/save_controller.h"

#include "./screens/found_screen.h"
#include "./screens/inventory_screen.h"
#include "./screens/menu_screen.h"
#include "./screens/picture_screen.h"

#include "../extractor/extractor.h"

using namespace std;
namespace openAITD {

    enum class AppState {
        Loading,
        Intro,
        InWorld,
        MainMenu,
        Inventory,
        Book,
        SelectGame
    };
    AppState state = AppState::MainMenu;
    bool gameStarted = false;
    bool fastStart = false;
    
    Resources resources;
    World world(&resources);
    FoundScreen foundScreen(&world);
    CameraRenderer renderer(&resources, &world);
    FreelookRenderer flRenderer(&resources, &world);
    PhysicsController physContr(&resources, &world, &foundScreen);
    ObjRotateController objrotContr(&world);
    AnimationController animContr(&resources, &world);
    HitController hitContr(&world);
    ThrowController throwContr(&world);
    PlayerController playerContr(&world);
    TracksController tracksContr(&world);
    InventoryScreen inventoryScreen(&world);
    MenuScreen mainMenu(&world);
    PictureScreen pictureScr(&world);
    LifeController lifeContr(&world, &tracksContr, &playerContr, &hitContr, &throwContr, &foundScreen);
    SaveController saveContr(&world, &lifeContr);

    bool freeLook = false;
    bool pause = false;
    const float maxDelta = 1. / 30;

    void startIntro() {
        world.gameOver = false;
        world.loadVars("data/vars.json");
        world.loadGObjects("data/objects.json");
        world.setCurStage(7, 1);
        lifeContr.reloadVars();
        state = AppState::Intro;
    }

    void startGame() {
        world.gameOver = false;
        world.loadVars("data/vars.json");
        world.loadGObjects("data/objects.json");
        world.setCurStage(0, 0);
        world.followTarget = 0;
        lifeContr.reloadVars();
        state = AppState::InWorld;
    }

    void loadGame(int slot) {
        world.gameOver = false;
        world.loadGObjects("data/objects.json");
        saveContr.load(mainMenu.saveSlot);
        state = AppState::InWorld;
    }

    bool loadStage() {
        if (world.curStageId == world.nextStageId) return false;
        BeginDrawing();
        auto& f = resources.screen.mainFont;
        const char* m = "Loading...";
        auto mt = MeasureTextEx(f, m, f.baseSize, 0);
        Vector2 v = { (int)(resources.config.screenW - mt.x) / 2, resources.config.screenH - (f.baseSize * 2) };
        DrawTextEx(f, m, v, f.baseSize, 0, WHITE);
        EndDrawing();

        world.curStage = &resources.stages[world.nextStageId];
        world.curStageId = world.nextStageId;
        world.curCameraId = -1;

        //Preload Cameras
        resources.backgrounds.loadStage(world.curStageId);
                
        //Preload Objects
        for (int i = 0; i < world.gobjects.size(); i++) {
            auto& gobj = world.gobjects[i];
            if (gobj.modelId == -1) continue;
            if (gobj.location.stageId != world.curStageId) continue;
            resources.models.getModel(gobj.modelId);
        }

        //reset frame time
        BeginDrawing();
        EndDrawing();

        return true;
    }

    void processWorld(float timeDelta) {
        if (world.picture.id != -1) {
            pictureScr.process(timeDelta);
            return;
        }

        loadStage();
        world.setCurRoom(world.nextRoomId);
        
        if (IsKeyPressed(KEY_O)) {
            freeLook = !freeLook;
        }
        if (IsKeyPressed(KEY_P)) {
            pause = !pause;
        }
        if (IsKeyDown(KEY_I)) {
            timeDelta *= 8;
        }
        if (!pause) {
            while (true) {

                float partDelta = min(timeDelta, maxDelta);
                world.chrono += partDelta;
                lifeContr.process(partDelta);
                animContr.process(partDelta);
                hitContr.process(partDelta);
                throwContr.process(partDelta);
                physContr.process(partDelta);
                objrotContr.process(partDelta);
                if (world.messageTime > 0) {
                    world.messageTime -= partDelta;
                }

                timeDelta -= maxDelta;
                if (timeDelta < 0) break;
            }
        }
        if (freeLook) {
            flRenderer.freeLook = pause;
            flRenderer.process();
        }
        else {
            //renderer.process();
        }
    }

    void renderWorld() {
        if (world.picture.id != -1) {
            pictureScr.render();
            return;
        }
        if (freeLook) {
            flRenderer.render();
        }
        else {
            renderer.render();
        }
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
            startIntro();
            break;
        case MenuScreenResult::resume:
            state = AppState::InWorld;
            break;
        case MenuScreenResult::saveGame:
            saveContr.save(mainMenu.saveSlot);
            world.messageText = "Game saved...";
            world.messageTime = 2;
            state = AppState::InWorld;
            break;
        case MenuScreenResult::loadGame:
            loadGame(mainMenu.saveSlot);
            break;
        }
        return true;
    }

    bool process(float timeDelta) {
        if (state == AppState::MainMenu) {
            if (!processMenu(timeDelta)) return false;
        }
        else if (state == AppState::Intro) {
            processWorld(timeDelta);
            if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ESCAPE)) {
                world.gameOver = true;
            }
            if (world.gameOver) {
                startGame();
            }
        }
        else if (state == AppState::InWorld) {
            if (world.player.allowInventory && IsKeyPressed(KEY_ENTER)) {
                inventoryScreen.reload();
                state = AppState::Inventory;
            }
            else if (world.player.allowInventory && IsKeyPressed(KEY_ESCAPE)) {
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
            inventoryScreen.process(timeDelta);
            if (inventoryScreen.exit) {
                state = AppState::InWorld;
            }
        }
        return true;
    }

    void render() {
        resources.screen.begin();

        if (state == AppState::MainMenu) {
            mainMenu.render();
        }
        else if (state == AppState::Intro) {
            renderWorld();
        }
        else if (state == AppState::InWorld) {
            renderWorld();
        }
        else if (state == AppState::Inventory) {
            inventoryScreen.render();
        }
            
        resources.screen.end();
    }

    int main(void)
    {
        resources.config = loadConfig();
        AITDExtractor::extractAllData();

        int m;
        if (resources.config.fulllscreen) {
            //SetConfigFlags(FLAG_BORDERLESS_WINDOWED_MODE);
            InitWindow(0, 0, "Open-AITD");
            m = GetCurrentMonitor();
            int w = GetMonitorWidth(m);
            int h = GetMonitorHeight(m);
            resources.config.screenW = h * 4 / 3;
            resources.config.screenH = h;
            resources.config.screenX = (w - resources.config.screenW) / 2;
            resources.config.screenY = 0;
            ToggleBorderlessWindowed();
            SetWindowSize(w, h);
            //SetWindowPosition(0, 0);
        }
        else {
            InitWindow(resources.config.screenW, resources.config.screenH, "Open-AITD");
            m = GetCurrentMonitor();
            //SetWindowPosition((w - resources.config.screenW) / 2 , (h - resources.config.screenH) / 2);
            //SetWindowSize(resources.config.screenW, resources.config.screenH);
        }
        resources.config.targetFps = GetMonitorRefreshRate(m);

        SetWindowState(FLAG_VSYNC_HINT);
        SetTargetFPS(resources.config.targetFps);
        SetExitKey(KEY_F10);

        resources.loadTexts("data/texts/english.txt");
        resources.screen.init();

        resources.stages.resize(8);
        for (int i = 0; i < 8; i++) {
            resources.stages[i].load(string("data/stages/")+to_string(i));
        }
        resources.loadTracks("data/tracks", "newdata/tracks");

        DisableCursor();

        if (fastStart) {
          startGame();
        }
        else {
          startMenu();
        }

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