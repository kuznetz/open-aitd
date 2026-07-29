#pragma once

#include <vector>
#include <string>
#include <memory>
#include "../world/world.h"
#include "../resources/resources.h"
#include "../../raylib-cpp/raylib-cpp.h"
#include "./widgets/vertical_menu.hpp"

using namespace std;
using namespace raylib;

namespace openAITD {

/**
 * @class InventoryScreen
 * @brief Handles the inventory UI: displays items, allows selection,
 *        shows possible actions for the selected item, and renders a 3D preview.
 */
class InventoryScreen {
public:
    // --- Public members ---
    World* world;               ///< Pointer to the game world (contains inventory, variables, etc.)
    Resources* resources;       ///< Pointer to resource manager (fonts, models, texts, config)
    bool exit = false;          ///< Flag to signal that the screen should close

    std::unique_ptr<VerticalMenuWidget> itemsMenu;
    std::unique_ptr<VerticalMenuWidget> actionsMenu;

    bool selAction = false;     ///< True when the user has selected an item and is choosing an action

    vector<int> curActions;     ///< List of action text IDs for the currently selected item

    // --- 3D model preview camera ---
    Camera3D modelCamera = {
        { 0, 2.5f, -5.f },   // Position
        { 0, 0, 0 },         // Target (look-at)
        { 0, 1, 0 },         // Up vector
        50.0f,               // Field of view (Y)
        CAMERA_PERSPECTIVE   // Projection type
    };
    float modelRotate = 0;      ///< Rotation angle for the 3D model preview
    bool firstFrame = true;     ///< Used to skip rotation on the first frame

    /**
     * @brief Constructor. Initialises the two menus based on screen dimensions.
     * @param world Pointer to the game world.
     */
    InventoryScreen(World* world)
        : world(world)
        , resources(world->resources)
    {
        float W = static_cast<float>(resources->config.screenW);
        float H = static_cast<float>(resources->config.screenH);

        // Items menu occupies the top half, full width
        itemsMenu = std::make_unique<VerticalMenuWidget>(
            resources->screen.mainFont,
            raylib::Rectangle{ 0, 0.05f * H, W, 0.45f * H },
            5
        );

        // Actions menu occupies the bottom-right quadrant (right half of bottom half)
        actionsMenu = std::make_unique<VerticalMenuWidget>(
            resources->screen.mainFont,
            raylib::Rectangle{ W / 2, H / 2, W / 2, H / 2 },
            5
        );

        reload(); // Fill the items list
    }

    ~InventoryScreen() = default;

    /**
     * @brief Reloads the inventory item list from the world and resets the action selection state.
     */
    void reload() {
        // Collect names of all inventory items using their nameId

        vector<string> itemNames;
        itemNames.reserve(world->inventory.size());
        for (auto& gobjPtr : world->inventory) {
            if (gobjPtr) {
                int nameId = gobjPtr->invItem.nameId;
                itemNames.push_back(resources->texts[nameId]);
            }
        }
        itemsMenu->setItems(itemNames);
        selAction = false;      // Start with item selection mode
        itemsMenu->active = true;
        actionsMenu->active = false;

        exit = false;
        firstFrame = true;      // Reset rotation for next time

        float W = static_cast<float>(resources->config.screenW);
        float H = static_cast<float>(resources->config.screenH);
        itemsMenu->bounds = raylib::Rectangle{ 0, 0.05f * H, W, 0.45f * H };
        actionsMenu->bounds = raylib::Rectangle{ W / 2, H / 2, W / 2, H / 2 };

        reloadActions();        // Update actions for the currently selected item
    }

    /**
     * @brief Refreshes the actions menu based on the currently selected inventory item.
     *        It checks the item's flags and maps each set bit to a text ID (starting at 23).
     */
    void reloadActions() {
        curActions.clear();
        int idx = itemsMenu->getSelectedIndex();
        if (idx >= 0 && idx < static_cast<int>(world->inventory.size())) {
            auto& gobj = *world->inventory[idx];
            // Check flags 0..10 (only these are considered valid action bits)
            for (int i = 0; i < 11; ++i) {
                if (gobj.invItem.flags & (1 << i)) {
                    curActions.push_back(i + 23);  // Map bit index to text resource ID (23 + i)
                }
            }
        }

        // Build the action menu from the collected text IDs
        vector<string> actionNames;
        actionNames.reserve(curActions.size());
        for (int actionId : curActions) {
            actionNames.push_back(resources->texts[actionId]);
        }
        actionsMenu->setItems(actionNames);
    }

    /**
     * @brief Handles keyboard input (navigation, selection, exit).
     */
    void processKeys() {
        // ESC: go back one step or close the screen
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (selAction) {
                selAction = false;          // Return to item selection
                itemsMenu->active = true;
                actionsMenu->active = false;
            } else {
                exit = true;                // Close the inventory screen
            }
        }

        // ENTER / SPACE: confirm selection
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            if (!selAction) {
                selAction = true;           // Switch to action selection
                itemsMenu->active = false;
                actionsMenu->active = true;
                reloadActions();            // Refresh actions (in case the selected item changed)
            } else {
                submit();                   // Execute the chosen action
                exit = true;                // Close the screen after submission
            }
        }

        // UP / DOWN: navigate the active menu
        if (IsKeyPressed(KEY_UP)) {
            if (!selAction) {
                itemsMenu->moveUp();
                reloadActions();            // Update actions for new item
            } else {
                actionsMenu->moveUp();
            }
        }

        if (IsKeyPressed(KEY_DOWN)) {
            if (!selAction) {
                itemsMenu->moveDown();
                reloadActions();
            } else {
                actionsMenu->moveDown();
            }
        }
    }

    /**
     * @brief Commits the selected action on the selected item.
     *        Stores the action flag in world->curInvAction and the item in world->curInvGObject
     *        for later processing by the game logic.
     */
    void submit() {
        int itemIdx = itemsMenu->getSelectedIndex();
        int actionIdx = actionsMenu->getSelectedIndex();
        if (itemIdx < 0 || itemIdx >= static_cast<int>(world->inventory.size())) return;
        if (actionIdx < 0 || actionIdx >= static_cast<int>(curActions.size())) return;

        world->curInvGObject = world->inventory[itemIdx];
        // Convert the action text ID back to a flag bit (1 << (id - 23))
        world->curInvAction = 1 << (curActions[actionIdx] - 23);
    }

    /**
     * @brief Main render function: draws the menus, separator lines, and the 3D preview.
     */
    void render() {
        drawLines();       // Draw the dividing lines between quadrants
        itemsMenu->draw();
        actionsMenu->draw();
        drawModel();       // Draw the 3D preview of the selected item
        resources->screen.resetViewport();
        drawVariable();   // Draw additional info (e.g., a variable value) below the model
    }

    /**
     * @brief Updates the screen state each frame.
     * @param timeDelta Time elapsed since the last frame (used for rotation).
     */
    void process(float timeDelta) {
        if (!firstFrame) {
            modelRotate += timeDelta * 180.f;   // Rotate model at a constant speed
            processKeys();                      // Handle input
        }
        firstFrame = false;
				itemsMenu->process(timeDelta);
				actionsMenu->process(timeDelta);
    }

    // --- Private drawing helpers (though they are public in this class) ---

    /**
     * @brief Draws the two separator lines that divide the screen into four quadrants.
     */
    void drawLines() {
        auto& w = resources->config.screenW;
        auto& h = resources->config.screenH;
        DrawLineEx({ 0, (float)(h / 2) }, { (float)w, (float)(h / 2) }, 2, GRAY);
        DrawLineEx({ (float)(w / 2), (float)(h / 2) }, { (float)(w / 2), (float)h }, 2, GRAY);
    }

    /**
     * @brief Renders the 3D model of the selected item in the top‑right quadrant.
     *        The model rotates around the Y axis.
     */
    void drawModel() {
        if (world->inventory.empty()) return;
        int idx = itemsMenu->getSelectedIndex();
        if (idx < 0 || idx >= static_cast<int>(world->inventory.size())) return;

        GameObject& gobj = *world->inventory[idx];
        RModel* rmodel = resources->models.getModel(gobj.invItem.modelId);
        if (!rmodel) return;

        BeginMode3D(modelCamera);
        
        auto& c = resources->config;
        rlViewport(c.screenX, c.screenY, c.screenW / 2, c.screenH / 2);
        float aspect = (float) c.screenW / c.screenH;

        Matrix proj = MatrixPerspective(modelCamera.fovy * DEG2RAD, aspect, 0.01, 100.0);
        rlMatrixMode(RL_PROJECTION);
        rlSetMatrixProjection(proj);

        rlMatrixMode(RL_MODELVIEW);
        Matrix view = MatrixLookAt(modelCamera.position, modelCamera.target, modelCamera.up);
        rlSetMatrixModelview(view);
        rlRotatef(modelRotate, 0, 1, 0);

        rmodel->model.Render();

        EndMode3D();   
    }

    /**
     * @brief Displays a variable value (if any) associated with the selected item.
     *        The variable ID is stored in gobj.stageLifeId.
     */
    void drawVariable() {
        int idx = itemsMenu->getSelectedIndex();
        if (idx < 0 || idx >= static_cast<int>(world->inventory.size())) return;
        auto& gobj = *world->inventory[idx];
        if (gobj.stageLifeId == -1) return;   // No variable to show

        string valStr = std::to_string(world->vars[gobj.stageLifeId]);
        raylib::Rectangle r = {
            resources->config.screenW * 0.01f,
            (float)(resources->config.screenH / 2),
            (float)(resources->config.screenW / 2),
            (float)resources->config.screenH
        };
        resources->screen.drawLeft(valStr.c_str(), r, raylib::WHITE);
    }
};

} // namespace openAITD