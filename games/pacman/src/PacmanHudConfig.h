#pragma once
#include <engine/config/ConfigGroup.h>

/// HUD layout tunables — positions, scales, and sizes for the score,
/// hi-score, and lives display. Registered separately from PacmanConfig
/// so gameplay and layout can be tuned independently.
struct PacmanHudConfig : Engine::ConfigGroup {
    Engine::Field<float> fontScale { this, "font_scale", 2.5f  };
    Engine::Field<float> margin    { this, "margin",     5.0f  };
    Engine::Field<float> iconSize  { this, "icon_size",  40.0f };
};
