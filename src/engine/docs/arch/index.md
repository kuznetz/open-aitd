# Engine Architecture Index

## src/engine/docs/arch/world.md
**Description:** Documents the core game state management, entity definitions, and particle systems. It details how game objects, inventory, physics, and animation states are structured and serialized for saving.
**Components:** `GOFlags`, `InventoryFlags`, `AnimationFlags`, `BoundsType`, `GOAnimation`, `GORotateAnim`, `GOInvItem`, `GOPhysics`, `GOLifeMode`, `GOTrackMode`, `GOTrack`, `GODamage`, `GOHit`, `GOThrowing`, `GameObject`, `Particle`, `ParticleGroup`, `ParticleGroups`, `SaveSlot`, `SaveHelper`, `PlayerAnimations`, `Player`, `Picture`, `BookData`, `Shake`, `World`.

## src/engine/docs/arch/renderers.md
**Description:** Outlines the rendering pipeline, including base renderer utilities, camera-driven scene composition, and post-processing effects. It covers the separation of object, mask, and particle rendering, as well as a debug freelook renderer.
**Components:** `BaseRenderer`, `SceneRenderer`, `RenderOrder`, `CameraRenderer`, `ObjectRenderer`, `MaskRenderer`, `ParticleRenderer`, `Debug3DText`, `FreelookRenderer`.

## src/engine/docs/arch/controllers.md
**Description:** Describes the game logic subsystems, including animation playback, physics simulation, collision detection, and player input handling. It also details the Lua scripting integration via `LifeController` and various action controllers for hits, throws, and tracks.
**Components:** `AnimationController`, `CameraController`, `HitAction`, `HitController`, `LifeController`, `LifeFunc`, `LifeCore`, `LifeCommands`, `LerpAngle`, `LerpEulerShortest`, `ObjRotateController`, `ParticleController`, `PhysicsController`, `PlayerController`, `ShakeController`, `ShootAction`, `ShootController`, `ThrowAction`, `ThrowController`, `TracksController`.

## src/engine/docs/arch/common.md
**Description:** Details the foundational utility modules shared across the engine, such as 3D math abstractions, coordinate metrics, and name resolution systems. It acts as the bridge between the graphics library and engine-specific logic.
**Components:** `EulerAngles`, `Vector3i`, `Metrics`, `NameCollection`, `NameDecoders`, `raylib`.

## src/engine/docs/arch/resources.md
**Description:** Covers the asset management layer, handling the loading and caching of audio, models, backgrounds, stages, and configuration data. It defines the structures for 3D geometry, collision bounds, and cinematic tracks.
**Components:** `Audio`, `BackgroundOverlay`, `Background`, `IntRect`, `Backgrounds`, `Bounds`, `Config`, `loadConfig`, `saveConfig`, `DataPath`, `MissedAnim`, `MissedAnims`, `BoneChannels`, `Animation`, `Bone`, `Model`, `RModel`, `RModels`, `Resources`, `Screen`, `RoomCollider`, `RoomZoneType`, `RoomZone`, `Room`, `GCameraOverlay`, `GCameraRoom`, `WCamera`, `Stage`, `isPointInPoly`, `findNode`, `NodeToBounds`, `loadLineAcc2d`, `Texts`, `TrackItemType`, `TrackItem`, `Track`.

## src/engine/docs/arch/screens.md
**Description:** Summarizes the user interface architecture, including in-game overlays, menus, and reusable UI widgets. It explains how screens interact with the world state to display inventory, books, saves, and debug consoles.
**Components:** `BookType`, `bookTypes`, `BookScreen`, `CharSelectScreen`, `ConsoleScreen`, `FoundScreen`, `InventoryScreen`, `MenuScreenResult`, `MenuScreenState`, `MenuScreen`, `ScreenRes`, `resolutions`, `OptionsScreen`, `PictureScreen`, `SavesScreen::Mode`, `SavesScreen`, `ButtonWidget`, `TextWidget`, `VerticalMenuWidget`.