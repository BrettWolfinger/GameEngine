#pragma once
#include "UfoConfig.h"
#include "AsteroidSizeConfig.h"
#include "ShipConfig.h"

// Load all Asteroids configs from their TOML files under
// games/asteroids/assets/configs/. Call once at game startup.
void loadAllConfigs();

// Register hot-reload callbacks so configs refresh when files change on disk.
// No-op in release builds. Call once after loadAllConfigs().
void watchAllConfigs();

#ifdef ENABLE_TOOLS
// Render the ImGui config editor window. Call from onImGuiRender().
void renderConfigEditor();
#endif
