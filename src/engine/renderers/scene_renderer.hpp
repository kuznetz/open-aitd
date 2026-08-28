#pragma once
#include <vector>
#include "common/raylib_cpp.hpp"
#include "engine/resources/resources.h"
#include "engine/world/world.h"

using namespace std;
using namespace raylib;

namespace openAITD {

class SceneRenderer {
private:
    World& world;
    Resources& resources;
    Texture2D softCircleTex = { 0 };
    Shader brightnessShader = { 0 };

public:
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
        if (softCircleTex.id) {
            UnloadTexture(softCircleTex);
        }
    }

    Texture2D GenerateSoftCircleTexture(int size = 128, float falloff = 2.0f) {
        // Create a black image
        Image img = GenImageColor(size, size, BLACK);
        float half = size / 2.0f;

        // Fill pixels with gradient
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                float dx = (float)x - half;
                float dy = (float)y - half;
                float dist = sqrtf(dx * dx + dy * dy);
                // Normalized distance from center (0 at center, 1 at edges)
                float t = dist / half;
                if (t > 1.0f) t = 1.0f;
                // Intensity: 1 - t^falloff
                float intensity = 1.0f - powf(t, falloff);
                // Convert to byte
                unsigned char val = (unsigned char)(intensity * 255.0f);
                ImageDrawPixel(&img, x, y, { val, val, val, 255 });
            }
        }

        // Load texture from image and unload the image
        Texture2D tex = LoadTextureFromImage(img);
        UnloadImage(img);
        return tex;
    }

    void renderMask() {
        if (!softCircleTex.id) {
            softCircleTex = GenerateSoftCircleTexture(128, 2.0f);            
        }

        auto& c = resources.config;
        float bright = world.brightnessCur;
        
        Color bgColor = {
            (unsigned char)(bright * 255.0f),
            (unsigned char)(bright * 255.0f),
            (unsigned char)(bright * 255.0f),
            255
        };

        BeginTextureMode(resources.screen.maskTex);
        ClearBackground(bgColor);

        auto& obj = world.gobjects[world.lightSpotObjId];
        if (obj.getStageId() == world.curStageId) {
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
            
            float size = radius * 3.0f;
            raylib::Rectangle srcRect = { 0, 0, (float)softCircleTex.width, (float)softCircleTex.height };
            raylib::Rectangle dstRect = { screenPos.x - size/2, screenPos.y - size/2, size, size };
            Vector2 origin = { 0, 0 };
            DrawTexturePro(softCircleTex, srcRect, dstRect, origin, 0.0f, WHITE);
        }

        EndTextureMode(); 
    }
    

    void render() {
        auto& c = resources.config;
        float offX = world.shake.offsetX;
        float offY = world.shake.offsetY;

        if (world.lightSpotObjId != -1) {
            useMask = true;
            renderMask();
        } else {
            useMask = false;
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