#pragma once
#include "GameTypes.h"

struct DotEaten         { int col; int row; int points; };
struct PowerPelletEaten { int col; int row; int points; };
struct GhostEaten       { GhostType type; int multiplier; int points; };
struct PacmanCaught     {};
struct LevelCleared     {};
struct DeathStarted     {};
struct LifeLost         { int livesRemaining; };
struct GameOverEvent    { int finalScore; int highScore; };
struct GameStarted      {};
