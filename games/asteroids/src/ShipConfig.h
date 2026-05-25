#pragma once
#include <string>
#include <vector>

struct ShipConfig {
    std::string name;
    float maxSpeed;
    float thrustForce;
    float drag;
    float rotateSpeed;
    float fireCooldown;
    int   shipFrame;    // 16x16 top-left frame index for 32x32 body sprite
    int   thrustFrame1;
    int   thrustFrame2;
};

namespace ShipConfigs {

// Populated by loadAllConfigs(). Size is determined by the TOML file.
inline std::vector<ShipConfig> All;

} // namespace ShipConfigs
