# Tilemap System

The engine includes a tilemap subsystem (`Engine::Tilemap`) that loads and renders
[Tiled](https://www.mapeditor.org/) maps. It handles TMX/TSX file parsing, multi-tileset
maps, and all eight Tiled flip/rotation orientations.

---

## Overview

Two headers make up the tilemap system:

| Header | Responsibility |
|---|---|
| `<engine/tilemap/Tilemap.h>` | Data types, TMX/TSX parsing, flip-bit utilities |
| `<engine/tilemap/TilemapRenderer.h>` | Rendering a tile layer via `Renderer2D` |

The split is intentional. `Tilemap.h` has no renderer dependency — it can be included
anywhere map data is needed (game logic, collision setup, entity spawning) without pulling
in the full rendering stack. `TilemapRenderer.h` depends on both.

---

## Loading a map

```cpp
Engine::Tilemap::Map m_map;

// In onInit():
m_map = Engine::Tilemap::loadMap("games/mygame/assets/maps/level1.tmx");
```

`loadMap` parses the TMX file and resolves all `<tileset>` references — both inline and
external `.tsx` files — relative to the TMX path. It populates:

- `map.cols`, `map.rows` — tile dimensions of the map
- `map.tileWidth`, `map.tileHeight` — tile size in pixels (from the TMX)
- `map.backgroundColor` — the `backgroundcolor` attribute as a `Color4` (RGBA floats)
- `map.layers` — all tile layers in TMX order
- `map.tilesets` — all tileset references with resolved image paths and column counts

---

## Data types

### `Map`

```cpp
struct Map {
    int    cols, rows;
    int    tileWidth, tileHeight;
    Color4 backgroundColor;
    std::vector<TileLayer>  layers;
    std::vector<TilesetRef> tilesets;

    const TileLayer*  findLayer(const std::string& name) const;
    const TilesetRef* tilesetForGid(uint32_t gid) const;
};
```

### `TileLayer`

```cpp
struct TileLayer {
    std::string           name;
    int                   cols, rows;
    std::vector<uint32_t> gids; // cols*rows, row-major; flip bits intact
};
```

GIDs are stored raw with flip bits in the top three bits. Use `stripFlips()` before
comparing to tileset ranges.

### `TilesetRef`

```cpp
struct TilesetRef {
    int         firstGid;
    std::string source;     // path to .tsx, relative to TMX
    std::string imagePath;  // resolved image path (relative to working dir)
    int         columns;
    int         tileCount;
};
```

### `Color4`

```cpp
struct Color4 { float r, g, b, a; };
```

---

## Looking up layers and tilesets

```cpp
// Find a layer by name (returns nullptr if not found)
const Engine::Tilemap::TileLayer* layer = m_map.findLayer("Terrain");

// Find the tileset that owns a given GID (flip bits stripped internally)
const Engine::Tilemap::TilesetRef* ts = m_map.tilesetForGid(1);
```

`tilesetForGid` finds the tileset with the highest `firstGid` that is still ≤ the
stripped GID — making it correct for multi-tileset maps and resilient to GID
renumbering in Tiled.

---

## Rendering a layer

```cpp
#include <engine/tilemap/TilemapRenderer.h>

// Minimal — full layer, no camera offset
Engine::Tilemap::renderLayer(
    m_renderer,        // Engine::Renderer2D&
    *layer,            // const TileLayer&
    *m_tilesetSheet,   // const SpriteSheet& — built from ts->imagePath
    *ts,               // const TilesetRef&
    tileRenderSize,    // float — tile size in screen pixels (often TILE * SCALE)
    kLayerTerrain      // int render layer (default 0)
);

// With camera and viewport culling
Engine::Tilemap::renderLayer(
    m_renderer, *layer, *m_tilesetSheet, *ts,
    tileRenderSize, kLayerTerrain,
    m_cameraX, m_cameraY,          // world-space camera offset
    static_cast<float>(WIN_W),     // viewport width  — enables column culling
    static_cast<float>(WIN_H)      // viewport height — enables row culling
);
```

Tiles with GID 0 (empty cells) are skipped automatically. When viewport dimensions are
provided, only tiles that intersect the visible region are submitted — a meaningful win
for large maps.

### Building a SpriteSheet from a tileset

```cpp
const auto* ts = m_map.tilesetForGid(1);
auto tex   = std::make_shared<Engine::Texture>(ts->imagePath);
auto sheet = std::make_shared<Engine::SpriteSheet>(
    tex,
    ts->columns,
    ts->tileCount / ts->columns
);
```

For maps that hardcode a known sheet layout, columns and rows can be supplied directly as
constants instead of reading from the `TilesetRef`.

---

## Flip bits

Tiled encodes tile orientation in the top three bits of each GID:

| Bit | Constant | Meaning |
|---|---|---|
| 31 | `0x80000000` | Flip horizontal |
| 30 | `0x40000000` | Flip vertical |
| 29 | `0x20000000` | Flip diagonal (transpose) |

The engine provides utilities for working with these:

```cpp
uint32_t rawGid = layer.gids[i];

uint32_t tileId = Engine::Tilemap::stripFlips(rawGid); // strip all flip bits
bool fH = Engine::Tilemap::hasFlipH(rawGid);
bool fV = Engine::Tilemap::hasFlipV(rawGid);
bool fD = Engine::Tilemap::hasFlipD(rawGid);
```

`renderLayer` handles all eight orientations automatically by calling `applyFlips`
internally. `applyFlips` returns a `FlippedUV` — an adjusted `UVRect` and a geometry
rotation angle in radians (positive = CW in Y-down space) — which is passed directly to
`drawTexturedRect`.

If you're reading GIDs manually (e.g. for collision or entity spawning), always call
`stripFlips` before comparing a GID to a tileset range or local tile ID.

---

## Background color

The TMX `backgroundcolor` attribute (e.g. `backgroundcolor="#5c94fc"`) is parsed to a
`Color4` and stored on `Map`. Use it to draw the background rect at startup or in
`onRender`:

```cpp
void MyGame::renderBackground() {
    const auto& c = m_map.backgroundColor;
    m_renderer.drawRect(0.f, 0.f, WIN_W, WIN_H, {c.r, c.g, c.b, c.a}, kLayerBackground);
}
```

Set the background color in Tiled via Map → Map Properties → Background Color.

---

## Multi-tileset maps

A single TMX can reference multiple tilesets. Each has its own `firstGid` range. Load
them separately and pass the matching one to `renderLayer`:

```cpp
const auto* wallTs  = m_map.tilesetForGid(1);
const auto* itemsTs = m_map.tilesetForGid(kItemsFirstGid);

// Build sheets for each
m_wallSheet  = std::make_shared<Engine::SpriteSheet>(...);
m_itemsSheet = std::make_shared<Engine::SpriteSheet>(...);

// Render each layer with its matching tileset
Engine::Tilemap::renderLayer(m_renderer, *wallLayer,  *m_wallSheet,  *wallTs,  tileSize, kLayerWalls);
Engine::Tilemap::renderLayer(m_renderer, *dotsLayer,  *m_itemsSheet, *itemsTs, tileSize, kLayerDots);
```

---

## Using GIDs for game logic

Layers don't have to be rendered — they can be read for collision setup, entity spawning,
or game state:

```cpp
const auto* layer = m_map.findLayer("Dots");
for (size_t i = 0; i < layer->gids.size(); ++i) {
    int localId = static_cast<int>(Engine::Tilemap::stripFlips(layer->gids[i])) - kItemsFirstGid;
    if (localId == kDotTileId)   cells[i] = CellType::Dot;
    if (localId == kPelletTileId) cells[i] = CellType::PowerPellet;
}
```

The row and column for index `i` are: `row = i / layer->cols`, `col = i % layer->cols`.
