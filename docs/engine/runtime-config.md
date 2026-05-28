# Runtime Config System

Per-entity tuning data (speeds, timers, lives, particle parameters, etc.) loads
from TOML files at runtime rather than being hardcoded. In debug builds, files
are watched for changes and hot-reloaded automatically. An ImGui editor lets you
adjust values live without restarting. The struct is the single source of truth
— the TOML file is auto-generated on first run from the struct's defaults.

---

## Quick Start

1. Define a config struct that inherits `Engine::ConfigGroup` and declares
   `Engine::Field<T>` members:

```cpp
// PacmanConfig.h
#pragma once
#include <engine/config/ConfigGroup.h>

struct PacmanConfig : Engine::ConfigGroup {
    Engine::Field<float> pacmanSpeed        { this, "pacman_speed",         7.5f };
    Engine::Field<float> ghostSpeed         { this, "ghost_speed",          6.0f };
    Engine::Field<float> frightenedDuration { this, "frightened_duration",  7.0f };
    Engine::Field<int>   startLives         { this, "start_lives",          3    };
};
```

2. Add a member to your game class and register it in `onInit()`:

```cpp
// MyGame.h
#include "PacmanConfig.h"
PacmanConfig m_config;

// MyGame.cpp
void MyGame::onInit() {
    registerConfig("games/mygame/assets/configs/pacman.toml", &m_config);
}
```

3. Use the fields anywhere in game code — they convert implicitly to their
   underlying type:

```cpp
pacman.update(dt, m_config.pacmanSpeed);
m_frightenedTimer = m_config.frightenedDuration;
```

That's it. The engine handles the rest.

---

## What `registerConfig` Does

When you call `Application::registerConfig(path, group)`:

1. **If the TOML file exists** — parses it and populates the `Field<T>` members.
2. **If the TOML file does not exist** — serialises the `Field<T>` defaults to a
   new TOML file on disk. No hand-authoring required; the struct drives
   everything.
3. **Registers a hot-reload watcher** — whenever the file changes on disk
   (debug builds only), all `Field<T>` values are updated automatically on the
   next frame.
4. **Registers the group with the ImGui editor** — the "Config Editor" window
   (F1) shows a collapsing section for every registered group, with drag widgets
   for each field.

---

## `Field<T>`

`Field<T>` is a self-registering config field. Supported types: `float`, `int`,
`bool`, `std::string`.

```cpp
Engine::Field<float> speed { this, "speed", 7.5f };
//                          ^     ^          ^
//                          |     |          default value
//                          |     TOML key
//                          parent ConfigGroup (always `this`)
```

Fields convert implicitly to `T` on read and accept `T` on write:

```cpp
float s = m_config.speed;   // implicit read
m_config.speed = 8.0f;      // write (persisted on next save)
```

`Field<T>` is not copyable — it holds a raw pointer to its parent and registers
itself at construction. Always declare fields as direct members of a
`ConfigGroup` subclass, never in a vector or dynamically allocated.

---

## TOML Files

Files live under `games/<game>/assets/configs/`. They are auto-generated on the
first run, human-readable, and comment-friendly.

```toml
# pacman.toml — auto-generated; edit freely
pacman_speed = 7.5
ghost_speed = 6.0
frightened_duration = 7.0
start_lives = 3
```

Keys match the second argument of each `Field<T>` constructor. Adding a new
field to the struct and deleting the old TOML file regenerates it cleanly;
unknown keys in an existing file are silently ignored.

---

## Multiple Config Groups

Register as many groups as you like — each with its own path:

```cpp
registerConfig("games/mygame/assets/configs/entities.toml", &m_entityConfig);
registerConfig("games/mygame/assets/configs/gameplay.toml", &m_gameplayConfig);
```

Each group appears as a separate collapsing section in the F1 editor, labelled
with the file stem (`entities`, `gameplay`).

---

## Engine Modules

### `Engine::ConfigGroup`

Base class for declarative config structs. `Field<T>` members self-register on
construction. You never call `ConfigGroup` methods directly — `registerConfig`
drives everything.

### `Engine::Field<T>`

Type-safe, self-registering config field. Declared as a member of a
`ConfigGroup` subclass.

### `Engine::ConfigLoader`

Loads and parses a TOML file. Returns a `toml::table`; throws
`std::runtime_error` on failure. Used internally by `ConfigRegistry`; games
rarely need to call it directly.

```cpp
#include <engine/config/ConfigLoader.h>
toml::table root = Engine::ConfigLoader::load("path/to/file.toml");
```

### `Engine::Config::watch` (facade)

Registers a raw callback to fire whenever a file changes on disk. No-op in
release builds. Prefer `registerConfig` for standard config groups — `watch` is
for custom reload logic that doesn't fit the `Field<T>` model.

```cpp
#include <engine/Engine.h>
Engine::Config::watch("path/to/file.toml", myReloadCallback);
```

---

## ImGui Editor

Press **F1** in a debug build to open the overlay. The "Config Editor" window
shows one `CollapsingHeader` per registered group. Drag any field to change its
value; the change takes effect immediately in game code (fields are read each
frame). Changes are **not** automatically written back to disk — edit the TOML
file directly if you want to persist a value, and the watcher will hot-reload it.

Ctrl+click (or double-click) any drag widget to type a value directly.

---

## Hot-Reload

The file watcher polls `std::filesystem::last_write_time` once per frame. When
a change is detected, all `Field<T>` values in the group are updated from the
new file contents. Because game code reads `m_config.field` each frame (not a
cached copy), the change is visible on the very next update tick.

Hot-reload is **debug-only** (`ENABLE_TOOLS` is undefined in Release). The
`Config::watch` facade call compiles in all configurations but is a no-op in
Release, so no `#ifdef` is needed at call sites.

---

## Release Builds

- TOML files are still loaded once at startup — config is not hardcoded.
- Hot-reload and `ConfigWatcher` are disabled — no polling overhead.
- The ImGui editor is not compiled — `ENABLE_TOOLS` is undefined.
- `Field<T>` values are set at startup and remain constant for the session.
