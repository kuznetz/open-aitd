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

    // Spot parameters (filled in render(), used as uniforms)
    Vector2 spotCenter = { 0.0f, 0.0f }; // spot center in UV space [0..1]
    Vector2 spotSizeUV = { 1.0f, 1.0f }; // spot quad size / screen size (per-axis, accounts for aspect ratio)

public:
    // Uniform locations
    int shUniformLoc = 0;
    int modeUniformLoc = 0;
    int sceneTextureLoc = 0;
    int maskTextureLoc = 0;
    int bgBrightnessLoc = 0;
    int spotBrightnessLoc = 0;
    int spotCenterLoc = 0;
    int spotSizeUVLoc = 0;
    
    Vector3 brightnessFactor = { 1.0f, 1.0f, 1.0f };
    bool useMask = false; //Externally controlled

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
        bgBrightnessLoc = GetShaderLocation(brightnessShader, "bgBrightness");
        spotBrightnessLoc = GetShaderLocation(brightnessShader, "spotBrightness");
        spotCenterLoc = GetShaderLocation(brightnessShader, "spotCenter");
        spotSizeUVLoc = GetShaderLocation(brightnessShader, "spotSizeUV");
        
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
        // Border texels are 0, so clamping gives a black mask outside the spot
        // (default REPEAT would wrap out-of-range UVs back into the circle)
        SetTextureWrap(tex, TEXTURE_WRAP_CLAMP);
        return tex;
    }

    // Computes the spot center and size in UV space. Returns true if the spot is active.
    bool computeSpotParams() {
        useMask = false;
        spotCenter = { 0.0f, 0.0f };
        spotSizeUV = { 1.0f, 1.0f };

        if (world.lightSpotObjId == -1) return false;

        auto& obj = world.gobjects[world.lightSpotObjId];
        if (obj.getStageId() != world.curStageId) return false;

        auto& c = resources.config;
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

        // The spot texture quad has size radius * 3.0 (as in the previous DrawTexturePro)
        float size = radius * 3.0f;

        spotCenter = { screenPos.x / (float)c.screenW, screenPos.y / (float)c.screenH };
        spotSizeUV = { size / (float)c.screenW, size / (float)c.screenH };
        useMask = true;
        return true;
    }

    void render() {
        auto& c = resources.config;
        float offX = world.shake.offsetX;
        float offY = world.shake.offsetY;

        // The soft circle texture is created once and sampled directly from the shader
        if (!softCircleTex.id) {
            softCircleTex = GenerateSoftCircleTexture(128, 0.8f);
        }

        computeSpotParams();

        BeginShaderMode(brightnessShader);

        if (useMask) {
            int mode = 1;
            float bgBrightness = world.brightnessCur;
            float spotBrightness = world.lightSpotBrightCur;
            SetShaderValue(brightnessShader, modeUniformLoc, &mode, SHADER_UNIFORM_INT);
            SetShaderValue(brightnessShader, bgBrightnessLoc, &bgBrightness, SHADER_UNIFORM_FLOAT);
            SetShaderValue(brightnessShader, spotBrightnessLoc, &spotBrightness, SHADER_UNIFORM_FLOAT);
            SetShaderValue(brightnessShader, spotCenterLoc, &spotCenter, SHADER_UNIFORM_VEC2);
            SetShaderValue(brightnessShader, spotSizeUVLoc, &spotSizeUV, SHADER_UNIFORM_VEC2);
            SetShaderValueTexture(brightnessShader, maskTextureLoc, softCircleTex);
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