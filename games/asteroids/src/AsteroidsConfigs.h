#pragma once
#include "UfoConfig.h"
#include "AsteroidConfig.h"
#include "ShipConfig.h"

// Load all Asteroids configs from their TOML files under
// games/asteroids/assets/configs/. Call once at game startup.
void loadAllConfigs();
