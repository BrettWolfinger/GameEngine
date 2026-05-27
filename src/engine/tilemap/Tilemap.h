#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "../renderer/UVRect.h"

namespace Engine::Tilemap {

// ---- data types ------------------------------------------------------------

struct Color4 {
    float r = 0.f, g = 0.f, b = 0.f, a = 1.f;
};

/// One tile layer as read from the TMX file.
/// gids are stored raw (flip bits intact); use stripFlips() / applyFlips() to interpret them.
struct TileLayer {
    std::string           name;
    int                   cols = 0;
    int                   rows = 0;
    std::vector<uint32_t> gids; // cols*rows, row-major
};

/// One tileset reference declared in the TMX <tileset> element.
struct TilesetRef {
    int         firstGid  = 1;
    std::string source;     // path to .tsx, relative to TMX
    std::string imagePath;  // resolved image path (relative to working dir)
    int         columns   = 0;
    int         tileCount = 0;
};

/// One object from a TMX <objectgroup> layer.
/// Point objects have width == 0 and height == 0.
/// x and y are in map pixel space (col * tileWidth, row * tileHeight).
struct MapObject {
    int         id     = 0;
    std::string name;
    std::string type;   // "class" in Tiled 1.9+, "type" in older versions
    float       x      = 0.f;
    float       y      = 0.f;
    float       width  = 0.f;
    float       height = 0.f;

    bool isPoint() const { return width == 0.f && height == 0.f; }
};

/// One object layer as read from a TMX <objectgroup> element.
struct ObjectLayer {
    std::string            name;
    std::vector<MapObject> objects;
};

/// Full parsed map.
struct Map {
    int                      cols        = 0;
    int                      rows        = 0;
    int                      tileWidth   = 16;
    int                      tileHeight  = 16;
    Color4                   backgroundColor;
    std::vector<TileLayer>   layers;
    std::vector<TilesetRef>  tilesets;
    std::vector<ObjectLayer> objectLayers;

    /// Returns the tile layer with the given name, or nullptr.
    const TileLayer*   findLayer(const std::string& name) const;

    /// Returns the object layer with the given name, or nullptr.
    const ObjectLayer* findObjectLayer(const std::string& name) const;

    /// Returns the first object with the given name across all object layers, or nullptr.
    const MapObject*   findObject(const std::string& name) const;

    /// Returns all objects whose type matches across all object layers.
    std::vector<const MapObject*> findObjectsByType(const std::string& type) const;

    /// Returns the tileset whose GID range contains gid (flip bits stripped internally).
    const TilesetRef*  tilesetForGid(uint32_t gid) const;
};

// ---- flip bit utilities ----------------------------------------------------

constexpr uint32_t kFlipMask = 0xE0000000u;

/// Strip all flip bits from a raw GID, leaving just the tile index.
inline uint32_t stripFlips(uint32_t gid) { return gid & ~kFlipMask; }

inline bool hasFlipH(uint32_t gid) { return (gid & 0x80000000u) != 0; }
inline bool hasFlipV(uint32_t gid) { return (gid & 0x40000000u) != 0; }
inline bool hasFlipD(uint32_t gid) { return (gid & 0x20000000u) != 0; }

/// Result of applying TMX flip flags to a UV rect.
struct FlippedUV {
    UVRect uv;
    float  angle = 0.f;
};

/// Apply the flip bits encoded in rawGid to uv, returning the adjusted UV rect
/// and any geometry rotation angle (radians, positive = CW in Y-down space).
FlippedUV applyFlips(UVRect uv, uint32_t rawGid);

// ---- loader ----------------------------------------------------------------

/// Load a TMX file, parse all tile layers and tileset references (including TSX files).
Map loadMap(const std::string& tmxPath);

} // namespace Engine::Tilemap
