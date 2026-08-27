#pragma once
#include <vector>
#include "../../common/raylib_cpp.hpp"
#include "../resources/resources.h"
#include "../world/world.h"

using namespace std;
using namespace raylib;

namespace openAITD {

class SceneRenderer {
private:
    World& world;
    Resources& resources;

public:
    
    Shader brightnessShader = { 0 };
    
    // Локации uniform
    int shUniformLoc = 0;
    int modeUniformLoc = 0;
    int sceneTextureLoc = 0;
    int maskTextureLoc = 0;
    
    Vector3 brightnessFactor = { 1.0f, 1.0f, 1.0f };
    bool useMask = false; //External controlled

    SceneRenderer(World& world) : world(world), resources(*world.resources) {}

    void init() {
        brightnessShader = LoadShader(
            "newdata/shaders/glsl330/brightness.vs",
            "newdata/shaders/glsl330/brightness.fs"
        );
        shUniformLoc = GetShaderLocation(brightnessShader, "brightness");
        modeUniformLoc = GetShaderLocation(brightnessShader, "mode");
        sceneTextureLoc = GetShaderLocation(brightnessShader, "sceneTexture");
        maskTextureLoc = GetShaderLocation(brightnessShader, "maskTexture");
        
        int mode = 0;
        SetShaderValue(brightnessShader, modeUniformLoc, &mode, SHADER_UNIFORM_INT);
        SetShaderValue(brightnessShader, shUniformLoc, &brightnessFactor, SHADER_UNIFORM_VEC3);
    }

    ~SceneRenderer() {
        if (brightnessShader.id) {
            UnloadShader(brightnessShader);
        }
    }

    void renderMask() {
        auto& c = resources.config;
        float bright = world.brightnessCur;
        
        Color bgColor = {
            (unsigned char)(bright * 255.0f),
            (unsigned char)(bright * 255.0f),
            (unsigned char)(bright * 255.0f),
            255
        };
        
        auto& obj = world.gobjects[world.lightSpotObjId];
        auto& b = obj.getBounds();
        Vector3 pos = obj.getAbsPosition();
        pos.y += (b.max.y - b.min.y) / 2.f;
        
        // Screen coordinates for the center of the circle
        Vector3 screenPos = world.WorldToScreenZ(pos);
        
        // Distance from the camera to the light source (Euclidean)
        Vector3 camPos = world.curCamera->position; // or from the matrix
        float dist = Vector3Distance(camPos, pos);
        if (dist < 0.001f) dist = 0.001f; // protection against division by zero
        
        // Radius parameters (adjustable)
        const float baseRadius = 150.0f;
        const float referenceDist = 10.0f;
        float radius = baseRadius * (referenceDist / dist);
        radius = Clamp(radius, 10.0f, 800.0f);
        
        BeginTextureMode(resources.screen.maskTex);
        ClearBackground(bgColor);
        DrawCircleV({screenPos.x, screenPos.y}, radius, WHITE);
        EndTextureMode(); 
    }
    

    void render() {
        auto& c = resources.config;
        float offX = world.shake.offsetX;
        float offY = world.shake.offsetY;

        if (world.lightSpotObjId != -1) {
            useMask = true;
            renderMask();
        }

        BeginShaderMode(brightnessShader);

        if (useMask) {
            int mode = 1;
            SetShaderValue(brightnessShader, modeUniformLoc, &mode, SHADER_UNIFORM_INT);
            SetShaderValueTexture(brightnessShader, maskTextureLoc, resources.screen.maskTex.texture);
            SetShaderValueTexture(brightnessShader, sceneTextureLoc, resources.screen.sceneTex.texture);
        } else {
            int mode = 0;
            auto& bright = world.brightnessCur;
            Vector3 brightnessFactor = { bright, bright, bright };            
            SetShaderValue(brightnessShader, modeUniformLoc, &mode, SHADER_UNIFORM_INT);
            SetShaderValue(brightnessShader, shUniformLoc, &brightnessFactor, SHADER_UNIFORM_VEC3);
            SetShaderValueTexture(brightnessShader, sceneTextureLoc, resources.screen.sceneTex.texture);
        }

        DrawTextureRec(resources.screen.sceneTex.texture,
                       { 0, 0, (float)c.screenW, (float)-c.screenH },
                       { offX, offY },
                       WHITE);        

        EndShaderMode();
    }
};

} // namespace openAITD