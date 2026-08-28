#pragma once

#include <functional>
#include "common/raylib_cpp.hpp"
#include "engine/resources/resources.h"
#include "engine/world/world.h"

namespace openAITD {

    /**
     * @brief Class for rendering object masks.
     * Contains the shader and mask texture, as well as methods for rendering
     * the mask and applying it to the color texture.
     */
    class MaskRenderer {
    public:
        // --- Masking resources ---
        Shader maskShader{ 0 };               ///< Shader for combining color and mask
        int shTextureColorLoc = 0;            ///< Location of the color texture in the shader
        int shTextureMaskLoc  = 0;            ///< Location of the mask texture in the shader

        RenderTexture2D objMaskTex;           ///< Texture into which the mask is rendered

        /**
         * @brief Initializes the shader and mask texture.
         * @param cfg Resource configuration (uses screen size).
         */
        void init(const Config& cfg) {
            // Create a mask texture with screen dimensions
            objMaskTex = LoadRenderTexture(cfg.screenW, cfg.screenH);

            // Load the shader
            maskShader = LoadShader(
                "newdata/shaders/glsl330/mask.vs",
                "newdata/shaders/glsl330/mask.fs"
            );
            shTextureColorLoc = GetShaderLocation(maskShader, "texture0");
            shTextureMaskLoc  = GetShaderLocation(maskShader, "texture1");
        }

        /**
         * @brief Renders the color texture with mask applied in the specified rectangle.
         * @param colorTex The color texture (source).
         * @param r        The rectangle on the screen for output.
         */
        void renderMasked(const Texture2D colorTex, const raylib::Rectangle& r) {
            BeginShaderMode(maskShader);
            SetShaderValueTexture(maskShader, shTextureColorLoc, colorTex);
            SetShaderValueTexture(maskShader, shTextureMaskLoc, objMaskTex.texture);

            float width  = static_cast<float>(objMaskTex.texture.width);
            float height = static_cast<float>(objMaskTex.texture.height);

            // Texture coordinates based on the screen rectangle
            Vector2 topLeft = {
                r.x / width,
                (height - r.y) / height
            };
            Vector2 botRight = {
                (r.x + r.width) / width,
                (height - (r.y + r.height)) / height
            };

            rlSetTexture(colorTex.id);
            rlBegin(RL_QUADS);
            rlTexCoord2f(topLeft.x, topLeft.y);
            rlVertex2f(r.x, r.y);
            rlTexCoord2f(topLeft.x, botRight.y);
            rlVertex2f(r.x, r.y + r.height);
            rlTexCoord2f(botRight.x, botRight.y);
            rlVertex2f(r.x + r.width, r.y + r.height);
            rlTexCoord2f(botRight.x, topLeft.y);
            rlVertex2f(r.x + r.width, r.y);
            rlEnd();

            EndShaderMode();
        }

        /**
         * @brief Renders the mask for a single object.
         * @param roomId       The identifier of the room the object is in.
         * @param position     The object's position (in local room coordinates).
         * @param curCamera    The current camera (for accessing rooms and overlays).
         * @param curBackground The current background (for obtaining overlay textures).
         * @param checkOverlay Function that checks whether the object's position falls within the overlay.
         */
        void renderMask(
            const int& roomId,
            const Vector3& position,
            const WCamera& curCamera,
            const Background& curBackground
        ) {
            BeginTextureMode(objMaskTex);
            ClearBackground(BLACK);
            BeginBlendMode(BLEND_ADDITIVE);

            // Iterate through camera rooms
            for (int camRoomIdx = 0; camRoomIdx < curCamera.rooms.size(); ++camRoomIdx) {
                if (roomId != curCamera.rooms[camRoomIdx].roomId) continue;

                // Iterate through overlays of the current room
                for (int ovlIdx = 0; ovlIdx < curCamera.rooms[camRoomIdx].overlays.size(); ++ovlIdx) {
                    const auto& ovl = curCamera.rooms[camRoomIdx].overlays[ovlIdx];
                    if (checkOverlay(ovl, position)) {
                        // Render the overlay texture into the mask buffer
                        renderOverlay(curBackground.overlays[camRoomIdx][ovlIdx]);
                    }
                }
            }

            EndBlendMode();
            EndTextureMode();
        }

    private:

        bool checkOverlay(const GCameraOverlay& ovl, const Vector3& pos) {
            for (int i = 0; i < ovl.bounds.size(); i++) {
                auto& b = ovl.bounds[i].getExpanded(-0.01);
                if (pos.x >= b.min.x && pos.x <= b.max.x &&
                    pos.z >= b.min.z && pos.z <= b.max.z) {
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief Renders a single overlay (texture) into the current mask buffer.
         */
        void renderOverlay(const BackgroundOverlay& ovl) {
            const auto& b = ovl.bounds;
            rlSetTexture(ovl.texture.id);
            rlBegin(RL_QUADS);
            rlTexCoord2f(0, 0);
            rlVertex2f(b.x, b.y);
            rlTexCoord2f(0, 1);
            rlVertex2f(b.x, b.y + b.height);
            rlTexCoord2f(1, 1);
            rlVertex2f(b.x + b.width, b.y + b.height);
            rlTexCoord2f(1, 0);
            rlVertex2f(b.x + b.width, b.y);
            rlEnd();
        }
    };

} // namespace openAITD