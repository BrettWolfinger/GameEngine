#pragma once
#include <cstdint>
#include <vector>

enum class CellType { Empty, Dot, PowerPellet };

struct WallTile {
    uint32_t gid;
    bool     flipH;
    bool     flipV;
    bool     flipD;
};

struct MapData {
    int                   cols = 0;
    int                   rows = 0;
    std::vector<WallTile> walls; // cols*rows, row-major; gid==0 means empty
    std::vector<CellType> dots;  // cols*rows, row-major
};
