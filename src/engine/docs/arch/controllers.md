# Architectural Documentation: `src/engine/controllers`

## `src/engine/controllers/animation_controller.h`
**Role:**  
Drives skeletal animation playback for all active game objects, handling time progression, keyframe interpolation, pose blending, and root-motion extraction so that animated models move consistently each frame.

**Components:**
*   **`AnimationController`**: Iterates over all `GameObject`s in the current stage, advances their animation timers, resolves animation IDs to model animation indices, computes current poses (including transitions between animations), and extracts root-motion deltas for physics integration. It also tracks missed animation references via `MissedAnims`.

---

## `src/engine/controllers/camera_controller.h`
**Role:**  
Automatically selects and positions the camera based on a follow target, ensuring the view always aligns with the nearest predefined camera in the target's current stage.

**Components:**
*   **`CameraController`**: Reads the world's `followTarget`, switches the active stage/room to match it, and queries the stage for the closest camera position. It updates the world's active camera index accordingly.

---

## `src/engine/controllers/hit_controller.h`
**Role:**  
Manages melee-style hit actions triggered at specific animation keyframes, performing bounding-box overlap checks to determine which objects are struck and applying damage.

**Components:**
*   **`HitAction`**: A lightweight record linking a game object, an animation ID, and a keyframe index to a single hit event. It tracks whether the action has been triggered.
*   **`HitController`**: Maintains a fixed-size pool of `HitAction` entries. Each frame it advances keyframe state, activates hits when the target keyframe is reached, computes the hit position from the relevant bone, and performs AABB overlap tests against all other objects in the stage to assign damage and hit relationships.

---

## `src/engine/controllers/life_controller.h`
**Role:**  
Serves as the top-level facade for the game's scripted behavior system, combining the Lua execution core with a comprehensive set of game-specific commands exposed to scripts.

**Components:**
*   **`LifeController`**: Inherits from `LifeCore` and composes a `LifeCommands` instance. It wires together all other controllers (tracks, player, hit, throw, physics, shoot) and UI screens so that Lua scripts can manipulate every aspect of game logic through a unified command set.

---

## `src/engine/controllers/life_controller/life_core.hpp`
**Role:**  
Initializes and manages the Lua runtime, loads script files from multiple data directories, and dispatches per-object "life" scripts each frame.

**Components:**
*   **`LifeFunc`**: A small structure tracking whether a Lua function has been executed in the current frame and holding a pointer to the compiled `LuaFunction`.
*   **`LifeCore`**: Creates and owns a `LuaState`, registers core utility functions (`GET`, `SET`, `ADD`, `LOG`, `dofile`), loads `main.lua` from `data/scripts`, `newdata/scripts`, and `moddata/scripts`, and discovers up to 1000 `life_<N>` functions. Each frame it iterates all active game objects and invokes their assigned life script, passing the object ID as an argument.

---

## `src/engine/controllers/life_controller/life_commands.hpp`
**Role:**  
Registers the full catalog of Lua-exposed functions that scripts use to query state, modify objects, trigger animations, handle inventory, play sounds, and interact with other controllers.

**Components:**
*   **`LifeCommands`**: Holds references to `LifeCore` and all sibling controllers. In `initExpressions()` it registers read/query functions (position, animation state, collisions, inventory flags, etc.). In `initInstructions()` it registers write/action functions (set model, set animation, change room, throw, hit, fire, move on tracks, play sound/music, spawn particles, etc.). It also provides helper methods like `getPosRel` for relative positioning and `canCompleteAnimation` for collision-aware animation validation.

---

## `src/engine/controllers/objrotate_controller.h`
**Role:**  
Processes smooth rotation animations for objects, interpolating Euler angles along the shortest arc and performing collision checks on rotated bounding boxes.

**Components:**
*   **`LerpAngle` / `LerpEulerShortest`**: Free utility functions that interpolate angles taking the shortest rotational path, normalizing results to `[-PI, PI]`.
*   **`ObjRotateController`**: Iterates game objects with active rotation animations, advances their timers, applies shortest-path Euler interpolation, and for objects with rotated bounds performs pairwise collision detection against other objects in the same stage.

---

## `src/engine/controllers/particle_controller.h`
**Role:**  
Updates all active particle groups each frame, dispatching to type-specific simulators for effects like ricochet sparks, smoke, muzzle flash, cigar smoke, and fountains.

**Components:**
*   **`ParticleController`**: Iterates `world.partGroups` and routes each active group to one of five inline update methods (`processRicochet`, `processSmoke`, `processMuzzleFlash`, `processCigarSmoke`, `processFountain`). Each method handles spawning, velocity integration, gravity, turbulence, lifetime decay, and group deactivation.

---

## `src/engine/controllers/physics_controller.h`
**Role:**  
Implements the core physics simulation: static and dynamic collision detection, gravity/falling, zone triggers, object pushing, and surface-snapping utilities.

**Components:**
*   **`PhysicsController`**: Each frame it resets collision state, then for each active object computes movement vectors (including root-motion from animation), resolves collisions against static room colliders and dynamic game objects, applies gravity to fallable objects, processes trigger zones, and handles stage-change repositioning. It also exposes `pushObject`, `raiseStuckObject`, `placeOnSurface`, and `findGroundHeight` as utilities consumed by other controllers and Lua commands.

---

## `src/engine/controllers/player_controller.h`
**Role:**  
Translates keyboard input into player character movement and rotation, selecting appropriate walk/run/turn/idle animations based on current input state.

**Components:**
*   **`PlayerController`**: Reads key states (arrows, shift) to compute rotation and forward/backward movement deltas. It updates the object's orientation, sets the movement vector, and switches the active animation to match the current action (walk forward/backward, run, turn clockwise/counterclockwise, or idle).

---

## `src/engine/controllers/shake_controller.hpp`
**Role:**  
Computes screen-shake offsets over time using combined sine/cosine waves, with configurable intensity fade-in and fade-out phases.

**Components:**
*   **`ShakeController`**: Reads the world's shake state, ramps intensity up or down based on active/inactive phase, and computes pixel-space `offsetX`/`offsetY` using multi-frequency sinusoidal functions scaled by intensity and screen height.

---

## `src/engine/controllers/shoot_controller.h`
**Role:**  
Handles projectile-style shooting triggered at animation keyframes, performing raycast intersection against static colliders and dynamic objects to determine hits and apply damage.

**Components:**
*   **`ShootAction`**: Records the shooter object, triggering animation/keyframe, emit bone index, damage value, and range for a single shot event.
*   **`ShootController`**: Maintains a fixed pool of `ShootAction` entries. Each frame it checks keyframe progress, and when triggered performs a raycast from the bone position along the object's forward direction. It tests against all dynamic objects and static room colliders, applies damage to the closest hit target, and spawns muzzle-flash and ricochet particle effects.

---

## `src/engine/controllers/throw_controller.h`
**Role:**  
Manages the throw action lifecycle: spawning the thrown item at the correct bone position when a keyframe is reached, then simulating its flight with collision detection until it lands or strikes an object.

**Components:**
*   **`ThrowAction`**: Stores the thrower, thrown item, triggering animation/keyframe, active bone, damage, and rotation flag for a single throw event.
*   **`ThrowController`**: Processes two phases each frame. `processAnim` waits for the throw keyframe, then spawns the item at the bone position with throwing state active. `processThrowables` advances all active thrown objects along their forward velocity, checks for static/dynamic collisions, applies damage on hit, and places the object on the ground surface. It also duplicates `findGroundHeight` and `placeOnSurface` utilities for landing resolution.

---

## `src/engine/controllers/tracks_controller.h`
**Role:**  
Drives NPC and object pathfinding along predefined track waypoints, supporting movement to positions, stair climbing, 3D positioning, rotation alignment, warping, and follow behaviors.

**Components:**
*   **`TracksController`**: Processes track-based movement by iterating track items for each object. It supports multiple track item types: `GOTO_POS` (move and rotate toward a waypoint), `GOTO_3D` (direct 3D placement), `STAIRS_X`/`STAIRS_Z` (elevation-aware stair climbing), `ROTATE_XYZ` (set rotation), `MARK`/`REWIND`/`END` (track state control), `WARP` (room teleport), and collision/trigger enable/disable flags. It also provides `processObjFollow` for objects that track another object's position.

---

## Module Relationships & Dependencies

### Internal Dependencies (`src/engine/controllers`)
*   **`LifeController`** is the central orchestrator. It composes `LifeCore` (Lua runtime) and `LifeCommands` (script API), and holds pointers to nearly every other controller (`TracksController`, `PlayerController`, `HitController`, `ThrowController`, `PhysicsController`, `ShootController`) so that Lua scripts can delegate actions to the appropriate subsystem.
*   **`LifeCommands`** depends on all sibling controllers, acting as the bridge between the Lua scripting layer and the C++ engine. It calls into `HitController::addAction`, `ShootController::addAction`, `ThrowController::throw_`, `TracksController::processObjTrack`/`processObjFollow`, `PlayerController::processObj`/`processRotate`, and `PhysicsController::raiseStuckObject`.
*   **`AnimationController`** and **`PhysicsController`** are loosely coupled but interact indirectly: animation root-motion feeds into the physics movement vector, and physics collision results feed back into script logic.
*   **`HitController`**, **`ShootController`**, and **`ThrowController`** all independently read bone positions from models and write damage/hit relationships to `GameObject` fields. They are coordinated through `LifeCommands` which schedules their actions at the correct script level.
*   **`ParticleController`** and **`ShakeController`** are leaf modules with no internal controller dependencies; they only read from `World` and `Resources`.
*   **`ObjRotateController`** and **`TracksController`** both modify object rotations and positions, but operate on different triggers (rotation animations vs. track waypoints).

### External Dependencies
*   **`src/engine/world`**: Every controller depends on `World` (and often `Resources`) as the primary source of game state, object lists, stage/room data, and configuration.
*   **`src/engine/resources`**: Controllers access models, animations, tracks, audio, texts, and backgrounds through the `Resources` subsystem.
*   **`src/engine/screens/found_screen.h`**: `PhysicsController` and `LifeCommands` depend on `FoundScreen` to display item-discovery UI when collisions or script commands trigger it.
*   **`luacpp`**: `LifeCore` and `LifeCommands` depend on the `luacpp` library for Lua state management, function registration, and script execution.
*   **`raylib`**: All controllers use raylib math types (`Vector3`, `Matrix`, `Quaternion`) and input functions (`IsKeyDown`) through the common `raylib_cpp.hpp` wrapper.