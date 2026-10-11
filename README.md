# open-AITD
A converter of resources from Alone In The Dark 1992 to open formats and an engine that runs on these open formats.

The project does not contain any data from the original game, and a legal copy of Alone In The Dark 1992 is required to run it.

The project is currently almost playable, but buggy.

Based on sources:
- https://github.com/jmimu/FITD
- https://github.com/yaz0r/FITD
- https://github.com/tigrouind/AITD-roomviewer

## Main features
- Almost all features from origial realisesd
- Built-in modding support
- The code has been completely rewritten from FITD for better readability
- Full‑fledged 3D rendering of models via rlgl
- For better precision, using floating point calculations for 3d ( FITD project does not, as original )
- This engine uses a fixed 4:3 aspect ratio; the original used a 320×200 resolution, which was scaled by CRT screens.
- Shooting over raycast, more accurate
- Rewrited animated UI
- TTF font render with multilanguage support
- No hard limit for fps
- Internal CUE/BIN CD image ripper for music and audiobooks
- Internal ADLIB music renderer to OGG music
- Character run on separate key
- Stable throwing items
- Save/load system with unlimited slots
- Developer 3d fly mode - with object indexes and additional info
- Developer console: teleport, take items, change script vars and more
- Object, models, scripts ID decoding over name tables
- Cutscene video playback for mods
- Alternative camera algorithm: the engine tries to select a camera facing the character's direction

## Supported AITD Versions
The engine testing on AitD 1 CD vesion from Steam, Floppy version needs more patches for stable playing

## Formats
- Before start special library convert all data to open formats, for easy view and modding
- Models converts to GLTF format with animations, added texture support for custom models and animations
- Original game logic (Life scripts) translates to LUA with modification and branch detecting
- Stages data (colliders, zones, cameras, etc) stored in GLTF scene with extra properties
- Background images and masks stored in PNG, background masks has alpha channel. Background can be any size, scaled to 4:3
- Music and audiobooks ripped into OGG
- SFX stored in WAV
- Game objects, waypoint tracks, start variables, saves stored in JSON
- Localized texts in UTF-8 with per-language TTF fonts and codepoint lists
- Cutscenes Ogg/Theora .ogv (video only)

## Libraries
- [Raylib](https://github.com/raysan5/raylib) 5.5 (raylib + raymath + rcamera + rlgl) — window, input, rendering, audio and math (its bundled `cgltf` and `stb_image_resize2` are used as well)
- [Lua](https://github.com/marovira/lua) 5.4.8 
- [luacpp](https://github.com/ouonline/luacpp) — embedded scripting runtime for the converted life scripts
- [nlohmann/json](https://github.com/nlohmann/json) 3.12.0 — JSON serialization for configs, saves, stages, tracks and objects
- [tinygltf](https://github.com/syoyo/tinygltf) 2.9.7 — glTF parsing and generating
- [libpng](https://github.com/pnggroup/libpng) 1.6.53 + [zlib-ng](https://github.com/zlib-ng/zlib-ng) 2.2.5 — PNG export and PAK block decompression
- [Ogg](https://github.com/xiph/ogg) + [Vorbis](https://github.com/xiph/vorbis) — Ogg/Vorbis music encoding and decoding
- [Theora](https://github.com/xiph/theora) — Ogg/Theora (.ogv) cutscene video decoding
- tiny-cdio — header-only CUE/BIN + ISO9660 reader for the internal CD ripper
- OPL2/OPL3 emulator — renders the original AdLib music to Ogg/Vorbis

## Building release (Windows)
- Install MS Build Tools 2022 (with cmake, vcpkg)
- Clone this repo with submodules
- Run in dir with sources (via x64 Native Tools Command Prompt for VS 2022):
```
build-release.cmd
```

## Develop environment (Windows)
- Install Visual Studio Code
- Install MS Build Tools 2022 (with cmake, vcpkg)
- Clone this repo with submodules
- Run in Prompt for VS 2022 (in dir with sources):
```
vcpkg install
```
- Open dir in VSCode with environment, for example:
```
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
cd "C:\Program Files\Microsoft VS Code"
start code
```
- run in VSCode "Cmake configure"
- then run in VSCode "Debug open-aitd"

