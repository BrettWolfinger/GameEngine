#pragma once
#include <cstdint>
#include <string>
#include <vector>

// Raw tile data for one layer. GID 0 = empty cell.
// Each GID has flip bits stripped; localId = gid - tilesetFirstGid.
struct TileLayer {
    std::string      name;
    int              cols = 0;
    int              rows = 0;
    std::vector<uint32_t> gids; // cols*rows, row-major, flip bits stripped
};

// One tileset reference as declared in the TMX file.
struct TilesetRef {
    int         firstGid = 1;
    std::string source;   // path to .tsx (relative to TMX)
    int         columns  = 0; // filled in after parsing .tsx
};

struct Color4 {
    float r = 0.f, g = 0.f, b = 0.f, a = 1.f;
};

struct MarioMap {
    int                    cols            = 0;
    int                    rows            = 0;
    int                    tileWidth       = 16;
    int                    tileHeight      = 16;
    Color4                 backgroundColor;   // from <map backgroundcolor="#rrggbb">
    std::vector<TileLayer> layers;
    std::vector<TilesetRef> tilesets;

    // Returns the layer with the given name, or nullptr.
    const TileLayer* findLayer(const std::string& name) const;

    // Returns the tileset whose range contains gid, or nullptr.
    const TilesetRef* tilesetForGid(uint32_t gid) const;
};

MarioMap loadMap(const std::string& tmxPath);
