# Adding a New Game

## Directory Structure

Each game lives under `games/` and follows the same layout:

```
games/
  mygame/
    CMakeLists.txt
    assets/          ← textures, audio, etc.
    src/
      main.cpp
      MyGame.h
      MyGame.cpp
      ... (game-specific source files)
```

---

## Step 1 — Create the CMakeLists.txt

Copy the pattern from any existing game. The only things that change are the target name and the source file list:

```cmake
add_executable(mygame
    src/main.cpp
    src/MyGame.cpp
)

target_include_directories(mygame PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(mygame PRIVATE engine)

target_compile_definitions(mygame PRIVATE $<$<NOT:$<CONFIG:Release>>:ENABLE_DEV_KEYS>)
target_compile_definitions(mygame PRIVATE $<$<NOT:$<CONFIG:Release>>:ENABLE_TOOLS>)
```

- `ENABLE_DEV_KEYS` — gate cheat keys or debug overlays that shouldn't ship.
- `ENABLE_TOOLS` — required for hot-reload (`Engine::Config::watch`) and the ImGui overlay (`onImGuiRender`). Must match the same flag the engine sets on itself.

---

## Step 2 — Register in the Root CMakeLists.txt

Add one line to the bottom of `/CMakeLists.txt`:

```cmake
add_subdirectory(games/mygame)
```

---

## Step 3 — Write main.cpp

Every game's `main.cpp` is the same pattern — instantiate the game class and call `run()`:

```cpp
#include "MyGame.h"
#include <iostream>

int main() {
    try {
        MyGame game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
```

---

## Step 4 — Write the Game Class

Subclass `Engine::Application` and override whichever lifecycle hooks you need:

```cpp
// MyGame.h
#pragma once
#include <engine/core/Application.h>
#include <engine/renderer/Renderer2D.h>
#include <engine/Engine.h>  // Engine::Audio, Engine::Collision, Engine::Particles

class MyGame : public Engine::Application {
public:
    MyGame();

protected:
    void preStep(float dt)  override;  // move objects, sync colliders
    void onUpdate(float dt) override;  // game logic, collision response
    void onRender()         override;  // draw calls
};
```

```cpp
// MyGame.cpp
#include "MyGame.h"

MyGame::MyGame() : Engine::Application("My Game", 800, 600) {
    // load assets, register colliders, spawn initial objects
}

void MyGame::preStep(float dt) {
    // move all objects and sync their collider positions
}

void MyGame::onUpdate(float dt) {
    // respond to input, handle collision results, spawn/erase objects
}

void MyGame::onRender() {
    m_renderer.beginScene(800, 600);
    // draw calls
}
```

By the time the constructor body runs, all engine subsystems are ready — objects can call `Engine::Collision::add()`, `Engine::Audio::playTone()`, etc. immediately.

---

## Step 5 — Build

Re-run CMake to pick up the new target, then build:

```bash
cmake -S . -B build
cmake --build build
```

The binary lands at `build/games/mygame/mygame`.

---

## Hook Reference

| Hook | Called | Typical use |
|---|---|---|
| `onInit()` | Once, before the loop | Late setup requiring a ready window |
| `preStep(dt)` | Every tick, before collision | Move objects, call `updateCircle/updateAABB` |
| `onUpdate(dt)` | Every tick, after collision | Input, collision response, spawning, erasing |
| `onRender()` | Every frame | `beginScene` then world draw calls |
| `onOverlayRender()` | Every frame, after particles | HUD, menus, overlays that appear above particles |
| `onImGuiRender()` | Every frame, when F1 overlay is on | ImGui debug widgets (debug builds only) |
| `onShutdown()` | Once, after loop exits | Cleanup |

Override `getRenderer()` to return your `Renderer2D` instance — this is **required** for the render layer flush and particle rendering to fire:

```cpp
Engine::Renderer2D* getRenderer() override { return &m_renderer; }
```

`dt` is always `1/60 ≈ 0.01667s`. See `docs/engine/application-game-loop.md` for the full tick order and timing details.

---

## Config Header Convention

Split game config into two files:

**`GameConstants.h`** — window dimensions, `SCALE`, and layer bitmasks. These values are compile-time constants that appear in many files:

```cpp
// GameConstants.h
#pragma once
#include <cstdint>

inline constexpr int W       = 800;
inline constexpr int H       = 600;
inline constexpr float SCALE = 1.f;

inline constexpr uint32_t kPlayerLayer = 1 << 0;
inline constexpr uint32_t kEnemyLayer  = 1 << 1;
```

Keeping `SCALE` here and deriving all render sizes from it (`16.f * SCALE`) means you can resize sprites globally without hunting through source files.

**`ConfigInit.h/.cpp`** — runtime-loaded per-entity tuning data (speeds, scores, particle params, etc.) stored in TOML files. See `docs/engine/runtime-config.md` for the full pattern.

```cpp
// ConfigInit.h
void loadAllConfigs();   // call once at startup
void watchAllConfigs();  // registers hot-reload callbacks (no-op in release)

#ifdef ENABLE_TOOLS
void renderConfigEditor(); // ImGui editor window; call from onImGuiRender()
#endif
```
