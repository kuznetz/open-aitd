# open-AITD
A converter of resources from Alone In The Dark 1992 to open formats and an engine that runs on these open formats.

The project does not contain any data from the original game, and a legal copy of Alone In The Dark 1992 is required to run it.

The project is currently unplayable. Ready-made builds, build and installation instructions will appear when the game becomes passable.

Based on sources:
- https://github.com/jmimu/FITD
- https://github.com/yaz0r/FITD
- https://github.com/tigrouind/AITD-roomviewer

## The difference from the FITD project
FITD uses algorithms identical to the original, in open-AITD:
- The code has been completely rewritten for better readability
- For better precision, using floating point calculations for 3d ( FITD project does not, as original )
- No hard limit for fps
- Before loading, all data converting to json, gltf, png, lua, wav and other open formats, for easy view and modding
- Models converts to gltf format with animations, added texture support for custom models
- Life scripts translates to LUA
- Background images can be any size, background masks has alpha channel
- Alternative camera algorithm: the engine tries to select a camera facing the character's direction
- Character run on separate key
