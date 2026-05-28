#pragma once

// Game-level tile semantics derived from the raw Dots layer.
enum class CellType { Empty, Dot, PowerPellet };

// Cardinal movement directions (None = stationary).
enum class Dir { None, Up, Down, Left, Right };

// Ghost identity — determines chase target and scatter corner.
// Values match spritesheet row order: 0=red(Blinky), 1=cyan(Inky), 2=pink(Pinky), 3=orange(Clyde).
enum class GhostType { Blinky = 0, Inky = 1, Pinky = 2, Clyde = 3 };

// Ghost behaviour state.
// House   = bobbing inside the ghost house, waiting to be released.
// Leaving = navigating from the house interior up through the door to the exit tile.
enum class GhostMode { Scatter, Chase, Frightened, Eyes, House, Leaving };
