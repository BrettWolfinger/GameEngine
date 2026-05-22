# Runtime Config System

Upgrade the engine so that per-type tuning data (asteroid sizes, UFO behaviour,
ship stats, etc.) loads from TOML files at runtime instead of being hardcoded as
`constexpr` tables. Adds hot-reload in debug builds and an ImGui editor for
designer iteration without recompiling.

---

## Motivation

All games built on this engine follow a config file pattern (`AsteroidConfig.h`,
`UfoConfig.h`, `ShipConfig.h`, …) that consolidates tuneable parameters in one
place per entity type. The current `constexpr` tables are a good intermediate
step — they established the right structure — but they require a recompile on
every value change. Moving to runtime-loaded files removes that bottleneck and
enables a live editing workflow.

---

## Format: TOML

**Chosen over JSON and YAML.**

- **JSON** — no comment support. Config files without comments are hostile to
  designers. `nlohmann/json` is the best C++ JSON library but the format itself
  is a dealbreaker for hand-edited files.
- **YAML** — indentation-sensitive syntax is a footgun for non-programmers.
  Enormous spec with surprising edge cases. Good when tooling generates the
  files; bad when humans edit them.
- **TOML** — designed for config files. Supports comments, explicit types, clean
  section syntax, no whitespace traps. Maps almost 1:1 to the existing config
  struct layout. `toml++` is header-only and fits the existing FetchContent
  pattern.

**Switching later is low cost.** The deserializer functions (`fromToml()`) are
the only format-specific code. The store, call sites, and hot-reload watcher are
all format-agnostic. Swapping the library and rewriting the deserializers is the
full scope of a format change.

---

## Phases

### Phase 1 — TOML Foundation

**New dependency: `toml++`**

```cmake
FetchContent_Declare(tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG        v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)
```

**New engine module: `engine/config/`**

- `ConfigLoader.h/.cpp` — loads and parses a TOML file from disk (debug) or
  from a compiled-in string literal (release). No knowledge of specific config
  types. Resolves paths relative to an assets root set at engine init.
- `ConfigStore.h` — typed registry. Games register their config arrays by name
  with a deserializer function. The store owns the runtime data and is the
  single point of access.

```cpp
namespace Engine {
class ConfigStore {
public:
    template<typename T>
    void registerConfig(std::string_view name,
                        std::string_view tomlSource,
                        std::function<T(const toml::table&)> deserializer);

    template<typename T>
    const T& get(std::string_view name, int index) const;
};
}
```

**Per-config deserializers (game-level)**

Each config file gets a `fromToml()` free function alongside its struct. The
`constexpr All[]` table is replaced by a `std::array` populated at game init.
Call sites (`SomeConfigs::All[idx].field`) are unchanged.

**TOML file layout**

One file per config type under `assets/configs/<game>/`. Example
`asteroid_sizes.toml`:

```toml
[large]
cell_count = 4
score = 20

  [large.frames]
  variant_0 = [196, 0, 0, 0]

  [large.particles]
  count = 20
  speed = 130.0
  speed_variance = 70.0
  lifetime = 1.2
  lifetime_variance = 0.30
  size = 4.0            # raw value; engine multiplies by SCALE on load

  [large.audio]
  noise_duration = 0.60
  noise_amplitude = 0.50
  noise_fade_time = 0.50

[medium]
# ...

[small]
  [small.frames]
  variant_0 = [204, 205, 220, 221]
  variant_1 = [206, 207, 222, 223]
  variant_2 = [236, 237, 252, 253]
  variant_3 = [238, 239, 254, 255]
```

Note: the awkward `frames[4][4]` C++ encoding becomes named keys per variant in
TOML, which is far more readable.

**Migration order**

1. Add `toml++`, wire into CMake
2. Add `engine/config/ConfigLoader`
3. Migrate one config (e.g. `UfoConfig` — simplest shape) as proof of concept
4. Migrate remaining configs one at a time
5. Remove all `constexpr` tables

---

### Phase 2 — Hot-Reload (`ENABLE_TOOLS` only)

**New engine component: `engine/config/ConfigWatcher`**

Polls `std::filesystem::last_write_time` on the main thread at the start of
each frame. No threading needed — polling is cheap at a 1–2 Hz check rate.

```cpp
class ConfigWatcher {
public:
    void watch(std::string_view path, std::function<void()> onChanged);
    void poll(); // called once per frame before game logic
};
```

When a change is detected the callback re-parses the file and updates the config
array in the `ConfigStore`. Existing code indexes into the store each frame so
new values are picked up immediately with no further propagation.

**Gated behind `ENABLE_TOOLS`**

```cpp
#ifdef ENABLE_TOOLS
    m_configWatcher.poll();
#endif
```

Set automatically by build type in CMake:

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Engine PRIVATE ENABLE_TOOLS)
endif()
```

Hot-reload is stripped entirely from release builds. Players run with embedded,
immutable config data.

---

### Phase 3 — ImGui Editor (`ENABLE_TOOLS` only)

**New dependency: Dear ImGui**

Integrated via `imgui_impl_opengl3` and `imgui_impl_glfw` backends using
FetchContent. Also gated behind `ENABLE_TOOLS`.

**New engine module: `engine/tools/`**

- `ConfigEditor.h/.cpp` — renders an editor window. Iterates registered configs
  in the `ConfigStore`, renders typed widgets per field (sliders for floats, int
  inputs, color pickers for `glm::vec3`), writes back to TOML on save.

Each config struct that opts in provides an `imguiEdit()` function alongside
`fromToml()` and `toToml()`. Manual per-struct editor functions are the right
starting point — less boilerplate machinery, explicit control.

**Save path**

Editor writes the modified struct back via `toToml()`, saves the file. The
`ConfigWatcher` detects the change and hot-reloads it, closing the loop. No
special save codepath needed.

**Toggle:** keybind (e.g. F1) in debug builds only.

---

## Release Build: Preventing Player Editing

Config files are **not shipped on disk** in release builds. They are embedded in
the binary at compile time as string literals using CMake's `file(READ ...)`:

```cmake
file(READ "assets/configs/asteroids/asteroid_sizes.toml" ASTEROID_SIZES_TOML)
configure_file(ConfigData.h.in ConfigData.h)
```

`ConfigLoader` has two code paths:

```cpp
#ifdef ENABLE_TOOLS
    // Debug: read from disk, supports hot-reload
    return toml::parse_file(resolvePath(name));
#else
    // Release: read from compiled-in string
    return toml::parse(getEmbeddedConfig(name));
#endif
```

There are no TOML files in the release build directory. Casual editing is
impossible; there is nothing to find.

---

## Key Decisions

| Decision | Choice | Reason |
|---|---|------|
| File format | TOML | Comment support, designer-friendly, easy to switch |
| C++ library | `toml++` | Header-only, C++17, fits FetchContent pattern |
| Hot-reload mechanism | Polling (`last_write_time`) | Simple, no platform-specific file watchers needed |
| Editor UI | Dear ImGui | Standard for game dev tools, integrates with existing OpenGL/GLFW |
| Release distribution | Embedded in binary | No files on disk = no player editing, no path management |
| Tools gate | `ENABLE_TOOLS` (Debug only) | Consistent with existing `ENABLE_DEV_KEYS` pattern |
| SCALE handling | Applied by deserializer on load | Call sites stay identical to current code |
| Missing file fallback | `constexpr` defaults | Keeps things working during incremental migration |

---

## What Stays the Same

- All call sites: `SomeConfigs::All[idx].field` — no changes across the codebase
- Config struct field definitions
- The one-file-per-entity-type pattern and discoverability for designers
