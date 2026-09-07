# Architectural Documentation: `src/engine/renderers`

## `src/engine/renderers/base_renderer.hpp`
**Role:**  
Provides the foundational base class for all renderer implementations, holding shared world/resource references, camera state, and common 3D-to-screen projection utilities.

**Components:**
*   **`BaseRenderer`**: Abstract base class for renderers. It stores references to `World` and `Resources`, tracks the current stage and camera IDs, and provides reusable utilities for loading/clearing cameras, projecting 3D bounding boxes to 2D screen rectangles with depth, drawing 3D wireframe bounds, and applying skeletal animation poses to models.

---

## `src/engine/renderers/scene_renderer.hpp`
**Role:**  
Handles post-processing of the final scene texture, applying brightness adjustments and dynamic light-spot masking via GLSL shaders to produce the final composited frame.

**Components:**
*   **`SceneRenderer`**: A post-processing renderer that loads a brightness/mask shader and manages a soft-circle texture for light-spot effects. It renders a mask texture centered on the light source object (scaled by camera distance), then composites the scene texture with the mask and brightness factor applied, outputting the final screen image with optional camera-shake offsets.

---

## `src/engine/renderers/camera_renderer/camera_renderer.hpp`
**Role:**  
Orchestrates the main camera-driven rendering pipeline, collecting all visible game objects and particle groups into a depth-sorted render queue, then rendering each with per-object masking and compositing.

**Components:**
*   **`RenderOrder`**: A linked-list node storing a polymorphic renderable (either a `GameObject*` or `ParticleGroup*`), its depth position, 3D bounds, and projected screen rectangle. Used to maintain a Z-sorted rendering order.
*   **`CameraRenderer`**: The primary renderer for normal gameplay cameras. It inherits from `BaseRenderer` and owns `ParticleRenderer`, `ObjectRenderer`, and `MaskRenderer` sub-renderers. It collects all objects and particle groups visible from the current camera's rooms, projects their bounds to screen space, sorts them by depth into a linked list, and renders each one individually: first generating a room-overlay mask, then rendering the object into a color texture, and finally compositing the masked color onto the scene texture. It also handles background rendering and camera/stage transitions.

---

## `src/engine/renderers/camera_renderer/object_renderer.hpp`
**Role:**  
Renders individual 3D game objects using a custom GLSL shader that supports texture mapping, diffuse color tinting, and Y-axis water-level clipping.

**Components:**
*   **`ObjectRenderer`**: Specialized renderer for `GameObject` instances. It lazily loads an inline GLSL 330 vertex/fragment shader pair that passes texture coordinates, normals, and world positions to the fragment stage. The fragment shader optionally discards fragments below a configurable Y-cut threshold (for water submersion effects) and blends texture color with diffuse tint. It computes the full model-view matrix chain (room offset, object position, rotation, Y-flip) and draws each mesh of the resolved `RModel`.

---

## `src/engine/renderers/camera_renderer/mask_renderer.hpp`
**Role:**  
Generates per-object visibility masks by rendering room overlay textures into a mask buffer, then composites a color texture through that mask using a custom shader.

**Components:**
*   **`MaskRenderer`**: Handles the overlay-mask rendering pipeline. It initializes a mask shader and a screen-sized mask render texture. For each object, it renders the relevant room overlay textures (checked against the object's position) into the mask buffer using additive blending. It then composites a source color texture through the mask by drawing a quad with the mask shader, which multiplies the color by the mask alpha, effectively clipping the object to visible room regions.

---

## `src/engine/renderers/camera_renderer/particle_renderer.hpp`
**Role:**  
Renders particle groups as billboarded circles in 3D space with alpha blending, using a procedurally generated circle texture.

**Components:**
*   **`ParticleRenderer`**: Renders `ParticleGroup` instances by drawing each active particle as a billboarded circle sprite. It lazily generates a 128x128 white-circle texture, constructs a `Camera3D` from the current world camera's position and rotation, and draws all particles with alpha blending. It includes a custom `MyDrawBillboard` implementation for oriented sprite rendering.

---

## `src/engine/renderers/freelook_renderer/freelook_renderer.hpp`
**Role:**  
Provides a free-look debug camera renderer for development, visualizing room bounds, colliders, zones, camera coverage, object positions, tracks, and particle emitters with 3D debug overlays and screen-space text.

**Components:**
*   **`Debug3DText`**: A lightweight structure pairing a 2D screen position with a string and color, used to queue debug text labels projected from 3D world positions.
*   **`FreelookRenderer`**: A debug-oriented renderer inheriting from `BaseRenderer`. It maintains a free-look `Camera` that the user can navigate with `CAMERA_FREE` controls. It renders all game objects in the current stage with debug overlays including bounding boxes, hitboxes, rotation axes, track paths, and 3D-projected text labels. It also visualizes room colliders, zone triggers, camera coverage polygons, particle emitters, and shoot rays. It provides helper methods for rendering debug text at 3D positions projected to screen space.

---

## Module Relationships & Dependencies

### Internal Dependencies (`src/engine/renderers`)
*   **`base_renderer.hpp`** is the foundational dependency for both `camera_renderer.hpp` and `freelook_renderer.hpp`, which inherit from `BaseRenderer` to share world/resource access, camera loading, and bounds-projection utilities.
*   **`camera_renderer/camera_renderer.hpp`** is the central orchestrator within the camera renderer subsystem. It owns and delegates to `ObjectRenderer`, `ParticleRenderer`, and `MaskRenderer` for their respective rendering tasks. It uses `RenderOrder` nodes to build a depth-sorted linked list of renderables.
*   **`camera_renderer/object_renderer.hpp`** is consumed exclusively by `CameraRenderer` to draw individual `GameObject` meshes with custom shader support.
*   **`camera_renderer/mask_renderer.hpp`** is consumed by `CameraRenderer` to generate per-object overlay masks and composite color textures through those masks.
*   **`camera_renderer/particle_renderer.hpp`** is consumed by `CameraRenderer` to render `ParticleGroup` billboard sprites.
*   **`scene_renderer.hpp`** operates independently of the camera renderers and is invoked after the scene texture is fully composed, applying final brightness and light-spot post-processing.
*   **`freelook_renderer/freelook_renderer.hpp`** is an independent debug renderer that does not depend on the camera renderer subsystem. It directly renders objects using Raylib's `DrawMesh` and provides its own debug visualization logic.

### External Dependencies
*   **World & Resources**: All renderers depend on `src/engine/world/world.h` for access to game objects, particle groups, camera state, stage/room data, and world-to-screen projection. They also depend on `src/engine/resources/resources.h` for models, backgrounds, screen textures, configuration, and name decoders.
*   **Stage Geometry**: `camera_renderer.hpp` and `freelook_renderer.hpp` depend on types from `src/engine/resources/stages.h` (`WCamera`, `Stage`, `Room`, `GCameraOverlay`, `Background`) for camera room/overlay data and background textures.
*   **Model Data**: `object_renderer.hpp` and `freelook_renderer.hpp` depend on `src/engine/resources/models.h` (`RModel`) to resolve 3D model meshes and materials. `base_renderer.hpp` also references `Model` for skeletal pose application.
*   **Particle System**: `camera_renderer.hpp` and `particle_renderer.hpp` depend on `src/engine/world/particles.h` (`ParticleGroup`, `Particle`) for particle data and bounds.
*   **Game Objects**: `camera_renderer.hpp`, `object_renderer.hpp`, and `freelook_renderer.hpp` all depend on `src/engine/world/game_objects.h` (`GameObject`) for object positions, rotations, model IDs, and bounds.
*   **Graphics & Math Stack**: All files depend on `src/common/raylib_cpp.hpp` for Raylib types (`Vector3`, `Matrix`, `Texture2D`, `Shader`, `Camera3D`, `Mesh`, `Material`, etc.) and drawing primitives.
*   **Bounds Utilities**: `base_renderer.hpp` and `camera_renderer.hpp` use `Bounds` from `src/engine/resources/bounds.h` for AABB projection and culling.
*   **Shader Files**: `scene_renderer.hpp` loads shaders from `newdata/shaders/glsl330/brightness.vs` and `brightness.fs`. `mask_renderer.hpp` loads from `newdata/shaders/glsl330/mask.vs` and `mask.fs`. `object_renderer.hpp` embeds its shader source inline as GLSL 330 strings.