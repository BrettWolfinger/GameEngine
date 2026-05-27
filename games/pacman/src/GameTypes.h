#pragma once

// Game-level tile semantics derived from the raw Dots layer.
enum class CellType { Empty, Dot, PowerPellet };

// Cardinal movement directions (None = stationary).
enum class Dir { None, Up, Down, Left, Right };
