# Dream BBQ Alternative

First-person OpenGL 3.3 game inspired by *Dream BBQ*. Course: Computer Graphics Fundamentals, Astana IT University.
Project #7 - team: Daniyal Abrayev, Damir Sheneusizov (SE-2411).

Repository: https://github.com/ShD8686/Dream-BBQ-Alternative
Backup video: https://youtu.be/iT8DU8DNamU

## Headline technique
Atmosphere rendering in a custom fragment shader (`default.frag`): exponential-squared distance fog, a warm point-light
model for the street lamps (distance attenuation + facing term), and an emissive "glow" channel that partly pierces the fog.
Press **F3** to switch fog + lamp lighting ON / OFF and compare. The first location (snow world) shows it immediately.

## Requirements
- Windows 10 / 11, x64
- Visual Studio 2022 (toolset v143) or newer, workload "Desktop development with C++"
- A GPU / driver with OpenGL 3.3 core

## Build
1. Open `Dream BBQ Alternative 2.sln` in Visual Studio.
2. Select configuration **Debug | x64** (or Release | x64).
3. Build (Ctrl+Shift+B), then run (F5). The working directory must be the project folder (it is by default),
   because shaders (`default.vert`, `default.frag`) and `Resources/` are loaded by relative path.
4. All libraries are already in `Libraries/` (GLFW, glad, glm, stb, tinyobjloader) or in the project (Dear ImGui, miniaudio).
   Nothing else has to be installed. All `.cpp` files must be in the project, including `Audio.cpp` and `stb.cpp`.
5. Missing audio files are skipped silently (the game still runs); missing textures and shader errors are printed to the console.

## Controls (also shown in game with F1 or H)
| Key | Action |
|---|---|
| W A S D | move |
| Shift | run |
| Space | jump |
| Mouse | look around |
| E | talk / pick up / enter the palace |
| Tab | open / close the phone (quest list) |
| F1 or H | help window (game paused while open) |
| P | pause / resume |
| F3 | headline technique ON / OFF (fog + lamp point lights) |
| Left / Right arrows | rotate the sun (light direction) |
| (automatic) | a lamp above the device on the 2nd location swings and moves its light |
| Esc | minimize window |

## How to play (about 5-10 minutes)
1. **Snow world.** Dense fog. Talk to the cook and the guard, find the fish, plug the fountain, clear the fog. Dim street lamps point the way.
2. **Dark stairs.** Walk to the exit.
3. **The road.** Run toward the palace before the 40 s timer ends. If you fail, the authors speak and you restart from the first location.
   After a failure you will find a small device on the second location. Take it and the palace will stop running away.
4. Enter the palace with **E** to reach the ending.

Back-face culling is enabled globally (counter-clockwise front faces): imported models must use that winding.

Debug start arguments: `--run`, `--run2`, `--stairs2`, `--fail`, `--bsod`, `--true` (jump directly to a part of the game).

## Third-party libraries and assets
GLFW (zlib), glad (MIT), glm (MIT), stb_image (public domain), tinyobjloader (MIT), tinygltf (MIT), Dear ImGui (MIT), miniaudio (public domain / MIT-0).
Audio files in `Resources/audio` - add the source and license of every file you ship here.
All geometry (map, lamps, palace, NPCs) is generated in code.


