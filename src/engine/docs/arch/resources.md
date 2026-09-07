# Architectural Documentation: `src/engine/resources`

## `src/engine/resources/audio.h`
**Role:**  
Manages all audio playback within the engine, including sound effects and background music, by wrapping Raylib's audio streaming API with an ID-based loading and caching system.

**Components:**
*   **`Audio`**: A singleton-style manager that initializes the audio device, loads and caches `Sound` objects by integer ID, and handles streaming music playback with volume control. It lazily loads sounds on first use and manages the lifecycle of the currently playing music track.

---

## `src/engine/resources/backgrounds.h`
**Role:**  
Loads, resizes, and manages stage background textures and their overlay masks, providing camera-specific backgrounds per stage with support for alternate background variants.

**Components:**
*   **`BackgroundOverlay`**: A structure representing a masked overlay region with bounds and an associated texture, used to render selective transparency or masking on top of background images.
*   **`Background`**: Represents a single background image with its texture path, loaded texture, and a 2D array of overlay masks organized by room and overlay index.
*   **`IntRect`**: A lightweight integer-based rectangle structure used internally for computing bounding regions of overlay masks during image processing.
*   **`Backgrounds`**: Manages the loading and caching of background textures per stage and camera. It resizes images to match the configured screen resolution, generates overlay masks from PNG mask files, and supports switching between default and alternate background variants.

---

## `src/engine/resources/bounds.h`
**Role:**  
Provides axis-aligned bounding box (AABB) representation and collision utilities, including overlap detection, minimum translation vector calculation, rotation transforms, and ray intersection tests.

**Components:**
*   **`Bounds`**: Represents an AABB defined by minimum and maximum `Vector3` corners. It provides static methods for computing minimum translation vectors (MTV) between two boxes along XYZ or XZ axes, as well as instance methods for bounds correction, expansion, rotation, corner extraction, collision testing, and ray-AABB intersection using the slab method.

---

## `src/engine/resources/config.h`
**Role:**  
Defines the engine's runtime configuration schema and provides serialization/deserialization functions to persist user settings to and from a JSON file.

**Components:**
*   **`Config`**: A structure holding global engine settings such as screen dimensions, fullscreen mode, target FPS, antialiasing level, and debug flags.
*   **`loadConfig()`**: A free function that reads `data/config.json` and populates a `Config` instance with defaults where values are missing.
*   **`saveConfig()`**: A free function that serializes a `Config` instance back to `data/config.json` in formatted JSON.

---

## `src/engine/resources/data_path.h`
**Role:**  
Resolves asset file paths with a priority-based fallback chain across mod, new, and default data directories, enabling mod override behavior.

**Components:**
*   **`DataPath`**: A utility class providing static methods to check file existence and resolve a relative asset path by searching `moddata/`, `newdata/`, and `data/` directories in order of priority.

---

## `src/engine/resources/missed_anims.h`
**Role:**  
Tracks and persists animations that are referenced but missing from model data, logging them to a JSON file for debugging and content completeness verification.

**Components:**
*   **`MissedAnim`**: A structure pairing a model ID with a list of animation IDs that were not found during loading.
*   **`MissedAnims`**: Manages a persistent list of `MissedAnim` entries, loading from and saving to `data/missed_anims.json`. It deduplicates entries and prints console warnings when a new missing animation is recorded.

---

## `src/engine/resources/model.h`
**Role:**  
Loads glTF models with skeletal animation support, baking animation keyframes at a target FPS, and providing pose interpolation, skinning, and bounding box computation.

**Components:**
*   **`BoneChannels`**: Holds pointers to the translation, rotation, and scale animation channels for a single bone, along with the interpolation type used by the glTF sampler.
*   **`Animation`**: Represents a single animation clip, storing the raw cgltf animation pointer, per-bone channel references, keyframe timestamps, root motion data, baked pose arrays, and baked bounding boxes.
*   **`Bone`**: A minimal bone descriptor storing the index of its parent bone in the skeleton hierarchy.
*   **`Model`**: The core model class that loads a glTF file via `cgltf`, parses skins and bones, extracts animations, computes AABB bounds, bakes poses at a given FPS, and applies skeletal transforms for rendering. It supports pose interpolation, root motion extraction, and direct mesh rendering through Raylib.

---

## `src/engine/resources/models.h`
**Role:**  
Acts as a resource cache for all game models, loading them on demand with animation ID mapping and bounds data from companion JSON files.

**Components:**
*   **`RModel`**: Wraps a `Model` instance with a mapping from game animation IDs to internal animation indices, along with a `Bounds` structure loaded from external JSON metadata.
*   **`RModels`**: Manages two maps of `RModel` instances (standard and alternate variants). It loads models lazily by ID, resolves filenames through `NameDecoders`, bakes animation poses at the configured target FPS, and parses per-model `data.json` for collision bounds.

---

## `src/engine/resources/resources.h`
**Role:**  
Serves as the central resource manager, aggregating all subsystem managers (config, stages, tracks, models, backgrounds, screen, audio, texts, and name decoders) into a single owning object.

**Components:**
*   **`Resources`**: The top-level resource container that initializes and wires together all engine subsystems. It manages the lifecycle of configuration, name decoding, stage and track loading, model and background caching, screen setup, audio, and text localization. It also provides utility methods like room connectivity checks and language switching.

---

## `src/engine/resources/screen.h`
**Role:**  
Manages the Raylib window, viewport, and render textures, handling fullscreen/windowed transitions and providing drawing begin/end framing.

**Components:**
*   **`Screen`**: Encapsulates the Raylib window and two render textures (scene and mask). It initializes the window with configured dimensions, handles fullscreen toggling with aspect-ratio-preserving sizing, manages viewport orthographic projection, and provides screenshot export functionality.

---

## `src/engine/resources/stages.h`
**Role:**  
Loads and represents stage geometry, including rooms, colliders, zones, and cameras, from glTF and JSON data files, providing spatial queries for camera selection and room transitions.

**Components:**
*   **`RoomCollider`**: Represents a collision volume within a room, storing its bounds, type (simple, climbing, or linked), and an associated parameter or linked object ID.
*   **`RoomZoneType`**: An enumeration defining zone behavior types: room change, trigger activation, and stage change.
*   **`RoomZone`**: A spatial zone within a room with bounds, a `RoomZoneType`, and a parameter value used for gameplay triggers.
*   **`Room`**: Represents a single room with its original position in world space, a list of colliders, and a list of interactive zones.
*   **`GCameraOverlay`**: Holds a list of bounding boxes defining overlay regions visible from a specific camera.
*   **`GCameraRoom`**: Associates a room ID with its camera-specific overlays.
*   **`WCamera`**: Represents a world camera with position, rotation, perspective parameters, and coverage polygons. It provides point-in-camera testing and world-to-screen projection utilities.
*   **`Stage`**: The main stage container holding rooms and cameras. It loads stage data from `stage.gltf` and `stage.json`, providing methods for camera proximity queries, room-to-room coordinate transformation, and point-in-camera tests.
*   **`isPointInPoly()`**: A free function implementing a ray-casting algorithm to test whether a 2D point lies within a polygon.
*   **`findNode()`**: A free function that searches a glTF model's nodes by name.
*   **`NodeToBounds()`**: A free function converting a glTF node's translation and scale into a `Bounds` AABB.
*   **`loadLineAcc2d()`**: A free function extracting 2D line vertex data from a glTF accessor for camera coverage zone polygons.

---

## `src/engine/resources/texts.h`
**Role:**  
Manages localized text strings and font loading, providing drawing utilities for UI text rendering with support for multiple languages and character encodings.

**Components:**
*   **`Texts`**: Loads localized text maps from `data/texts/<lang>/main.txt` and loads a TTF font with a predefined set of Unicode codepoints (including Latin extended and Cyrillic). It provides methods to retrieve text by ID, load book texts from separate files, and draw left-aligned or centered text using the loaded font.

---

## `src/engine/resources/tracks.h`
**Role:**  
Defines the data structures for cinematic camera tracks, enumerating all possible track item types used to drive cutscene playback.

**Components:**
*   **`TrackItemType`**: An enumeration listing all cinematic track commands, including warps, position jumps, speed changes, rotations, collision toggles, trigger toggles, marks, and stair movements.
*   **`TrackItem`**: A single entry in a cinematic track, storing the item type, target room, mark label, timing, and optional position/rotation vectors.
*   **`Track`**: A type alias for `vector<TrackItem>`, representing a complete cinematic sequence as an ordered list of track items.

---

## Module Relationships & Dependencies

### Internal Dependencies (`src/engine/resources`)
*   **`data_path.h`** is the foundational dependency for all asset-loading modules. `audio.h`, `backgrounds.h`, `models.h`, and `texts.h` all use `DataPath::GetFile()` to resolve file paths with mod-priority fallback.
*   **`config.h`** is consumed by `screen.h`, `backgrounds.h`, `models.h`, `texts.h`, and `resources.h` to access screen dimensions, FPS targets, and fullscreen settings.
*   **`bounds.h`** is a core geometry utility used by `model.h` (for AABB computation), `stages.h` (for colliders and zones), and `models.h` (for per-model bounds metadata).
*   **`model.h`** is the low-level model loader consumed by `models.h`, which adds caching, animation ID mapping, and JSON metadata parsing on top of it.
*   **`stages.h`** is depended on by `backgrounds.h` (to iterate stage cameras for background loading) and `resources.h` (to store the stage list).
*   **`tracks.h`** defines the `Track` type used by `resources.h` to store cinematic sequences.
*   **`resources.h`** is the aggregate owner of all other modules in this directory. It instantiates `Config`, `NameDecoders`, `RModels`, `Backgrounds`, `Screen`, `Audio`, `Texts`, and manages `Stage` and `Track` vectors, wiring their cross-references (e.g., `backgrounds.stages = &stages`, `models.nameDecoders = &nameDecoders`).

### External Dependencies
*   **Graphics & Math Stack**: `screen.h`, `audio.h`, `backgrounds.h`, `model.h`, `models.h`, `bounds.h`, and `texts.h` all depend on `src/common/raylib_cpp.hpp` for Raylib types (`Texture2D`, `Sound`, `Model`, `Vector3`, `Matrix`, `Font`, etc.). `bounds.h` additionally depends on `src/common/euler_angles.hpp` for rotation-based bounds transformation.
*   **Name Resolution**: `models.h` depends on `src/common/name_decoders.hpp` to translate integer model IDs into human-readable directory names for asset path construction.
*   **Stage Metrics**: `stages.h` depends on `src/common/metrics.hpp` for coordinate system conversions when parsing stage geometry.
*   **glTF Parsing**: `model.h` uses the external `cgltf` library for glTF model and animation loading. `stages.h` uses `tinygltf` for parsing stage geometry from glTF files.
*   **JSON Serialization**: `config.h`, `backgrounds.h`, `missed_anims.h`, `models.h`, `resources.h`, and `stages.h` all depend on `nlohmann/json` for parsing configuration, metadata, and stage data files.
*   **Image Resizing**: `backgrounds.h` uses `stbir` (via `stbir_resize_uint8_linear`) for high-quality background image resizing to match the target screen resolution.