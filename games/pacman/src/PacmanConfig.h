#pragma once
#include <engine/config/ConfigGroup.h>

/// Declarative config for Pac-Man tunables.
/// Registered via Application::registerConfig() in onInit(); the engine
/// auto-generates the TOML file on first run from these defaults.
struct PacmanConfig : Engine::ConfigGroup {
    Engine::Field<float> pacmanSpeed          { this, "pacman_speed",           7.5f };
    Engine::Field<float> ghostSpeed           { this, "ghost_speed",            6.0f };
    Engine::Field<float> ghostSpeedFrightened { this, "ghost_speed_frightened", 3.0f };
    Engine::Field<float> frightenedDuration   { this, "frightened_duration",    7.0f };
    Engine::Field<int>   startLives           { this, "start_lives",            3    };
    Engine::Field<float> deathPause           { this, "death_pause",            1.0f };
    Engine::Field<float> levelClearPause      { this, "level_clear_pause",      3.0f };
};
