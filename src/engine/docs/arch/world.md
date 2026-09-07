# Architectural Documentation: `src/engine/world`

## `src/engine/world/game_objects.h`
**Role:**  
Defines the core game object entity and its associated substructures, encapsulating animation, physics, inventory, tracking, and collision state for every interactive entity in the game world.

**Components:**
*   **`GOFlags`**: A bitfield structure encoding binary properties of a game object such as animatability, drawability, movability, trigger status, and findability.
*   **`InventoryFlags`**: A bitfield structure encoding inventory-related capabilities and states, including whether an item can be used, eaten, read, thrown, dropped, or is currently held in inventory.
*   **`AnimationFlags`**: A bitfield structure encoding animation playback behavior, including repeat mode, interruptibility, and reset-on-change semantics.
*   **`BoundsType`**: An enumeration defining collision bounds computation modes: simple AABB, cubified AABB, and rotation-transformed AABB.
*   **`GOAnimation`**: Holds per-object animation state including current and next animation IDs, root motion vectors, pose interpolation data, frame timing, and animation flags.
*   **`GORotateAnim`**: Stores interpolation data for smooth Euler angle rotation transitions, including start/end angles and timing.
*   **`GOInvItem`**: Represents the inventory aspect of an object, storing its display name ID, visual model ID, life script ID, and inventory capability flags.
*   **`GOPhysics`**: Encapsulates physics state including falling status, collidability, movement vectors, collision references, zone triggers, and optional bounds overload data.
*   **`GOLifeMode`**: An enumeration defining object lifecycle scoping: no lifecycle, stage-wide, room-scoped, or room-visible-only-from-specific-camera.
*   **`GOTrackMode`**: An enumeration defining movement track behavior: none, manual control, follow-target, or cinematic track playback.
*   **`GOTrack`**: Stores track-following state including track ID, current position index, mode, start/target positions, direction, and debug visualization data.
*   **`GODamage`**: Represents a damage event with damage amount, source object reference, and impact point.
*   **`GOHit`**: Describes an active hitbox with range, bone index, damage value, bounds, and target object reference.
*   **`GOThrowing`**: Stores throwing state including active flag, throw direction, thrower reference, and impact damage.
*   **`GameObject`**: The central entity class representing any interactive object in the world. It aggregates animation, physics, inventory, and track substructures, manages position and rotation with cached bounds and rotation matrices, and provides spatial queries for absolute positioning, room-relative bounds, and render-space bounds. It holds a reference to `Resources` for model and stage lookups.

---

## `src/engine/world/particles.h`
**Role:**  
Implements a lightweight, fixed-capacity particle system for visual effects, managing individual particles and groups with automatic recycling and bounds computation for rendering culling.

**Components:**
*   **`Particle`**: A minimal particle descriptor storing active state, 3D position, velocity, color, size, and remaining lifetime.
*   **`ParticleGroup`**: Represents a single particle emitter with a fixed pool of 25 particles. It manages spawn timing, emitter position and direction, stage/room placement, and provides methods for particle recycling, active-state checking, and bounds computation for both logical and render-space culling.
*   **`ParticleGroups`**: A fixed-capacity pool of 10 `ParticleGroup` instances. It provides an `add()` method that reuses inactive groups or recycles the oldest active group by creation order, ensuring bounded memory usage for particle effects.

---

## `src/engine/world/save_helper.h`
**Role:**  
Provides save slot management with full serialization and deserialization of the `World` state to and from JSON files, including screenshot capture and slot metadata listing.

**Components:**
*   **`SaveSlot`**: A lightweight structure holding a save slot's numeric ID, timestamp string, and human-readable location description.
*   **`SaveHelper`**: Manages save/load/delete/list operations for game state. It serializes the entire `World`—including all game objects' positions, rotations, animations, tracks, inventory, variables, and global flags—into a `data.json` file per slot. It also generates a `slot.json` metadata file and a `screen.png` screenshot. On load, it reconstructs the world state, restores object references (inventory, hand-held, follow target), and triggers resource preloading.

---

## `src/engine/world/world.h`
**Role:**  
Serves as the central game state manager, owning all game objects, particles, inventory, player state, and camera configuration. It orchestrates stage/room transitions, object lifecycle queries, animation control, inventory operations, and world-to-screen coordinate projection.

**Components:**
*   **`PlayerAnimations`**: A structure holding default animation IDs for player character actions including idle, walk, run, and turn variants.
*   **`Player`**: Represents the player entity with a reference object ID, control state, animation mappings, inventory access flag, and input state.
*   **`Picture`**: Stores state for a displayed image sequence, including ID, current time, and frame delay.
*   **`BookData`**: Holds data for an in-progress book-reading interaction, including book type, text ID, and audio ID.
*   **`Shake`**: Manages screen shake effects with configurable intensity, fade-in/out timing, and per-frame offset computation.
*   **`World`**: The top-level game state class. It owns the `gobjects` vector, `ParticleGroups`, inventory, game variables, and player state. It tracks the current stage, room, and camera, managing transitions between them. It provides object lifecycle queries (`isObjectActive`), animation setters, inventory operations (take/drop/put), model assignment, and world-to-screen coordinate projection. It holds a pointer to `Resources` for asset and stage access.

---

## Module Relationships & Dependencies

### Internal Dependencies (`src/engine/world`)
*   **`game_objects.h`** is the foundational entity definition consumed by all other modules in this directory. `world.h` owns a `vector<GameObject>`, and `save_helper.h` serializes and deserializes every `GameObject` field.
*   **`particles.h`** is consumed by `world.h`, which owns a `ParticleGroups` instance for managing active particle effects in the current scene.
*   **`world.h`** is the central state hub depended on by `save_helper.h`, which holds a `World&` reference to serialize and restore the full game state.
*   **`save_helper.h`** depends on both `world.h` (for state access) and `game_objects.h` (for object field iteration during serialization).

### External Dependencies
*   **Resources Layer**: `world.h` holds a `Resources*` pointer and depends on `src/engine/resources/resources.h` for access to stages, models, backgrounds, screens, and configuration. `game_objects.h` also holds a `Resources&` reference for model bounds lookups and stage/room coordinate transforms.
*   **Math & Coordinate Utilities**: `game_objects.h` depends on `src/common/raylib_cpp.hpp` for `Vector3`, `Matrix`, `Transform`, and `BoundingBox` types, on `src/common/euler_angles.hpp` for rotation normalization, and on `src/common/metrics.hpp` for integer rotation encoding used by scripts.
*   **Stage Geometry**: `game_objects.h` and `world.h` both depend on types from `src/engine/resources/stages.h` (`Stage`, `Room`, `RoomCollider`, `WCamera`) for spatial queries, room transitions, and camera view/projection matrix construction.
*   **Model Data**: `game_objects.h` depends on `src/engine/resources/models.h` (`RModel`) to resolve per-model collision bounds during `getBounds()` and `getRenderBounds()` calls.
*   **JSON Serialization**: `save_helper.h` and `world.h` both depend on `nlohmann/json` for save file I/O and game object loading from `data/objects.json` and `data/vars.json`.
*   **Filesystem**: `save_helper.h` uses `std::filesystem` for slot directory creation, deletion, and enumeration.