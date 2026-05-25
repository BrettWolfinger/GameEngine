#include "AsteroidsConfigs.h"
#include "AsteroidsConfig.h"
#include <engine/config/ConfigLoader.h>

// ---- UFO sizes --------------------------------------------------------------

static UfoConfig ufoFromToml(const toml::table& t) {
    return UfoConfig{
        static_cast<float>(t["render_size"  ].value_or(0.0)) * SCALE,
        static_cast<float>(t["speed"        ].value_or(0.0)),
        static_cast<float>(t["aim_variance" ].value_or(0.0)),
        static_cast<float>(t["fire_rate"    ].value_or(0.0)),
        static_cast<int>  (t["sprite_frame" ].value_or(0)),
        static_cast<int>  (t["score"        ].value_or(0)),
        static_cast<int>  (t["particle_count"           ].value_or(0)),
        static_cast<float>(t["particle_speed"           ].value_or(0.0)),
        static_cast<float>(t["particle_speed_variance"  ].value_or(0.0)),
        static_cast<float>(t["particle_lifetime"        ].value_or(0.0)),
        static_cast<float>(t["particle_lifetime_variance"].value_or(0.0)),
        static_cast<float>(t["particle_size"            ].value_or(0.0)) * SCALE,
    };
}

// ---- Asteroid sizes ---------------------------------------------------------

static AsteroidSizeConfig asteroidFromToml(const toml::table& t) {
    AsteroidSizeConfig cfg{};
    cfg.cellCount = static_cast<int>(t["cell_count"].value_or(0));
    cfg.score     = static_cast<int>(t["score"     ].value_or(0));

    if (const auto* frames = t["frames"].as_table()) {
        for (int v = 0; v < 4; ++v) {
            const std::string key = "variant_" + std::to_string(v);
            if (const auto* arr = (*frames)[key].as_array()) {
                for (int f = 0; f < 4 && f < static_cast<int>(arr->size()); ++f)
                    cfg.frames[v][f] = static_cast<int>((*arr)[f].value_or(0));
            }
        }
    }

    if (const auto* p = t["particles"].as_table()) {
        cfg.particleCount          = static_cast<int>  ((*p)["count"            ].value_or(0));
        cfg.particleSpeed          = static_cast<float>((*p)["speed"            ].value_or(0.0));
        cfg.particleSpeedVariance  = static_cast<float>((*p)["speed_variance"   ].value_or(0.0));
        cfg.particleLifetime       = static_cast<float>((*p)["lifetime"         ].value_or(0.0));
        cfg.particleLifetimeVariance = static_cast<float>((*p)["lifetime_variance"].value_or(0.0));
        cfg.particleSize           = static_cast<float>((*p)["size"             ].value_or(0.0)) * SCALE;
    }

    if (const auto* a = t["audio"].as_table()) {
        cfg.noiseDuration  = static_cast<float>((*a)["noise_duration" ].value_or(0.0));
        cfg.noiseAmplitude = static_cast<float>((*a)["noise_amplitude"].value_or(0.0));
        cfg.noiseFadeTime  = static_cast<float>((*a)["noise_fade_time"].value_or(0.0));
    }

    return cfg;
}

// ---- Ships ------------------------------------------------------------------

static ShipConfig shipFromToml(const toml::table& t) {
    return ShipConfig{
        std::string(t["name"          ].value_or("")),
        static_cast<float>(t["max_speed"     ].value_or(0.0)),
        static_cast<float>(t["thrust_force"  ].value_or(0.0)),
        static_cast<float>(t["drag"          ].value_or(0.0)),
        static_cast<float>(t["rotate_speed"  ].value_or(0.0)),
        static_cast<float>(t["fire_cooldown" ].value_or(0.0)),
        static_cast<int>  (t["ship_frame"    ].value_or(0)),
        static_cast<int>  (t["thrust_frame_1"].value_or(0)),
        static_cast<int>  (t["thrust_frame_2"].value_or(0)),
    };
}

// ---- Entry point ------------------------------------------------------------

void loadAllConfigs() {
    {
        const auto root = Engine::ConfigLoader::load(
            "games/asteroids/assets/configs/ufo_sizes.toml");
        UfoConfigs::All.clear();
        UfoConfigs::All.push_back(ufoFromToml(*root["large"].as_table()));
        UfoConfigs::All.push_back(ufoFromToml(*root["small"].as_table()));
    }
    {
        const auto root = Engine::ConfigLoader::load(
            "games/asteroids/assets/configs/asteroid_sizes.toml");
        AsteroidSizeConfigs::All.clear();
        AsteroidSizeConfigs::All.push_back(asteroidFromToml(*root["large" ].as_table()));
        AsteroidSizeConfigs::All.push_back(asteroidFromToml(*root["medium"].as_table()));
        AsteroidSizeConfigs::All.push_back(asteroidFromToml(*root["small" ].as_table()));
    }
    {
        const auto root = Engine::ConfigLoader::load(
            "games/asteroids/assets/configs/ships.toml");
        ShipConfigs::All.clear();
        for (const auto& [key, val] : root) {
            if (const auto* t = val.as_table())
                ShipConfigs::All.push_back(shipFromToml(*t));
        }
    }
}
