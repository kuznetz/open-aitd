#pragma once
#include <vector>
#include <string>
#include "./resources/resources.h"
#include "./world/world.h"
#include "./controllers/camera_renderer.h"
#include "./controllers/freelook_renderer.h"
#include "./controllers/player_controller.h"
#include "./controllers/physics_controller.h"
#include "./controllers/animation_controller.h"
#include "./controllers/life_controller.h"
#include "./controllers/tracks_controller.h"

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
    AppState state;

    int screenW = 1280;
    int screenH = 960;
    
    Resources resources;
    World world(&resources);
    CameraRenderer renderer(&resources, &world);
    FreelookRenderer flRenderer(&resources, &world);
    PlayerController playerContr(&resources, &world);
    PhysicsController physContr(&resources, &world);
    AnimationController animContr(&resources, &world);
    LifeController lifeContr(&resources, &world);
    TracksController tracksContr(&world);
    
    bool freeLook = false;
    bool pause = false;

    void startIntro() {
        world.gameOver = false;
        world.loadVars("data/vars.json");
        world.loadGObjects("data/objects.json");
        world.setCurRoom(7, 1);
        lifeContr.reloadVars();
        state = AppState::Intro;
    }

    void startGame() {
        world.gameOver = false;
        world.loadVars("data/vars.json");
        world.loadGObjects("data/objects.json");
        world.setCurRoom(0, 0);
        lifeContr.reloadVars();
        state = AppState::InWorld;
    }

    void processWorld(float timeDelta) {
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
            world.chrono += timeDelta;
            playerContr.process(timeDelta);
            lifeContr.process();
            animContr.process(timeDelta);
            tracksContr.process(timeDelta);
            physContr.process(timeDelta);
        }

        BeginDrawing();
        if (freeLook) {
            flRenderer.process();
        }
        else {
            renderer.process();
        }
        EndDrawing();
    }

    int main(void)
    {
        AITDExtractor::extractAllData();

        InitWindow(screenW, screenH, "Open-AITD");

        resources.stages.resize(8);
        for (int i = 0; i < 8; i++) {
            resources.stages[i].load(string("data/stages/")+to_string(i));
        }
        resources.loadTracks("data/tracks");

        renderer.screenW = screenW;
        renderer.screenH = screenH;
        DisableCursor();

        startIntro();

        float timeDelta = 0;        
        while (!WindowShouldClose()) {
            timeDelta = GetFrameTime();
            if (state == AppState::Intro || state == AppState::InWorld) {
                processWorld(timeDelta);
                if (state == AppState::Intro && IsKeyPressed(KEY_SPACE)) {
                    world.gameOver = true;
                }
            }
            if (world.gameOver) {
                if (state == AppState::Intro) {
                    startGame();
                }
                else {
                    break;
                }
            }
        }

        return 0;
    }
}

int main(void)
{
    return openAITD::main();
}