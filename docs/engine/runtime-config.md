# Runtime Config System

Per-entity tuning data (speeds, scores, particle parameters, etc.) loads from
TOML files at runtime rather than being hardcoded as `constexpr` tables. In
debug builds, files are watched for changes and hot-reloaded automatically. An
ImGui editor lets you adjust values live and save them back to disk.

---

## TOML Files

One file per config type under `games/<game>/assets/configs/`. Human-readable,
comment-friendly. The loader applies `SCALE` to size values on load — the files
store raw pixel-agnostic values.

```toml
# ufo_sizes.toml
[large]
render_size  = 32.0   # raw; loader multiplies by SCALE
speed        = 80.0
score        = 200
fire_rate    = 2.0
# ...

[small]
render_size  = 16.0
speed        = 120.0
score        = 1000
# ...
```

---

## Config Structs

Each entity type gets a header with a plain struct and a namespace holding the
runtime array:

```cpp
// UfoConfig.h
struct UfoConfig {
    float renderSize;
    float speed;
    int   score;
    // ...
};

namespace UfoConfigs {
inline std::vector<UfoConfig> All;  // populated by loadAllConfigs()
}
```

`inline std::vector<T> All` is an ODR-safe C++17 inline variable — no extern
declaration needed. Size is driven entirely by the TOML file, not by a C++
constant.

---

## Engine Modules

### `Engine::ConfigLoader`

Loads and parses a TOML file. Returns a `toml::table`; throws
`std::runtime_error` on missing file or parse failure.

```cpp
#include <engine/config/ConfigLoader.h>

toml::table root = Engine::ConfigLoader::load("games/mygame/assets/configs/enemies.toml");
```

### `Engine::Config::watch` (facade)

Registers a callback to fire whenever a file changes on disk. No-op in release
builds — call sites need no `#ifdef`:

```cpp
#include <engine/Engine.h>

Engine::Config::watch("games/mygame/assets/configs/enemies.toml", loadEnemyConfigs);
```

The watcher polls `std::filesystem::last_write_time` once per frame (before
game logic). When a change is detected, the callback runs immediately —
typically the same `loadXxxConfigs()` function used at startup.

---

## ConfigInit Pattern

Each game centralises its config loading in two files:

**`ConfigInit.h`**
```cpp
void loadAllConfigs();   // parse TOML → populate All vectors
void watchAllConfigs();  // register hot-reload callbacks

#ifdef ENABLE_TOOLS
void renderConfigEditor(); // ImGui editor; call from onImGuiRender()
#endif
```

**`ConfigInit.cpp`**

Three sections per config type:

1. **`fromToml()`** — deserializer. Reads a `toml::table`, applies `SCALE` to
   size fields, returns the struct.
2. **`loadXxxConfigs()`** — clears `All`, calls `ConfigLoader::load()`, pushes
   entries built with `fromToml()`.
3. **`ENABLE_TOOLS` block** — `toToml()` serializer, `saveXxxConfigs()` writer,
   and the `renderConfigEditor()` ImGui implementation.

Call both entry points from the game constructor:

```cpp
AsteroidsGame::AsteroidsGame() : Engine::Application("Asteroids", W, H) {
    loadAllConfigs();
    watchAllConfigs();
}
```

---

## ImGui Editor

In non-Release builds, `renderConfigEditor()` renders a window with collapsing
headers per config type. Each entry expands to show drag widgets for every
tunable field. **Save** writes the current values back to TOML; the watcher
detects the change and hot-reloads it on the next frame, closing the loop.

Ctrl+click (or double-click) any drag widget to type a value directly.

Wire it up via the `onImGuiRender()` hook:

```cpp
// MyGame.h
void onImGuiRender() override;

// MyGame.cpp
void MyGame::onImGuiRender() {
#ifdef ENABLE_TOOLS
    renderConfigEditor();
#endif
}
```

Press **F1** in a debug build to toggle the overlay.

---

## Release Builds

- `Engine::Config::watch()` is a no-op — no `ConfigWatcher` overhead.
- `onImGuiRender()` is declared but never called by the engine.
- `renderConfigEditor()` is not compiled — `ENABLE_TOOLS` is undefined.
- TOML files remain on disk (not embedded); config values are loaded once at
  startup and never change.

---

## Adding a New Config Type

1. Create `MyConfig.h` — plain struct + `namespace MyConfigs { inline std::vector<MyConfig> All; }`.
2. Create a TOML file under `assets/configs/`.
3. In `ConfigInit.cpp`:
   - Write `myConfigFromToml(const toml::table&)` to deserialize one entry.
   - Write `loadMyConfigs()` to clear `All` and populate it.
   - Add `loadMyConfigs()` to `loadAllConfigs()`.
   - Add `Engine::Config::watch(kMyPath, loadMyConfigs)` to `watchAllConfigs()`.
   - In the `ENABLE_TOOLS` block, add `myConfigToToml()`, `saveMyConfigs()`, and
     a collapsing header in `renderConfigEditor()`.
