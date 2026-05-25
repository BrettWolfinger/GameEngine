#include "ConfigInit.h"
#include "GameConstants.h"
#include <engine/config/ConfigLoader.h>
#include <engine/facade/Config.h>

#ifdef ENABLE_TOOLS
#include <imgui.h>
#include <fstream>
#include <cctype>
#endif

// ---- UFO sizes --------------------------------------------------------------

static UfoConfig ufoFromToml(const toml::table& t) {
    return UfoConfig{
        static_cast<float>(t["render_size"              ].value_or(0.0)) * SCALE,
        static_cast<float>(t["speed"                    ].value_or(0.0)),
        static_cast<float>(t["aim_variance"             ].value_or(0.0)),
        static_cast<float>(t["fire_rate"                ].value_or(0.0)),
        static_cast<int>  (t["sprite_frame"             ].value_or(0)),
        static_cast<int>  (t["score"                    ].value_or(0)),
        static_cast<int>  (t["particle_count"           ].value_or(0)),
        static_cast<float>(t["particle_speed"           ].value_or(0.0)),
        static_cast<float>(t["particle_speed_variance"  ].value_or(0.0)),
        static_cast<float>(t["particle_lifetime"        ].value_or(0.0)),
        static_cast<float>(t["particle_lifetime_variance"].value_or(0.0)),
        static_cast<float>(t["particle_size"            ].value_or(0.0)) * SCALE,
    };
}

static constexpr std::string_view kUfoPath = "games/asteroids/assets/configs/ufo_sizes.toml";

static void loadUfoConfigs() {
    const auto root = Engine::ConfigLoader::load(kUfoPath);
    UfoConfigs::All.clear();
    UfoConfigs::All.push_back(ufoFromToml(*root["large"].as_table()));
    UfoConfigs::All.push_back(ufoFromToml(*root["small"].as_table()));
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
        cfg.particleCount            = static_cast<int>  ((*p)["count"            ].value_or(0));
        cfg.particleSpeed            = static_cast<float>((*p)["speed"            ].value_or(0.0));
        cfg.particleSpeedVariance    = static_cast<float>((*p)["speed_variance"   ].value_or(0.0));
        cfg.particleLifetime         = static_cast<float>((*p)["lifetime"         ].value_or(0.0));
        cfg.particleLifetimeVariance = static_cast<float>((*p)["lifetime_variance"].value_or(0.0));
        cfg.particleSize             = static_cast<float>((*p)["size"             ].value_or(0.0)) * SCALE;
    }

    if (const auto* a = t["audio"].as_table()) {
        cfg.noiseDuration  = static_cast<float>((*a)["noise_duration" ].value_or(0.0));
        cfg.noiseAmplitude = static_cast<float>((*a)["noise_amplitude"].value_or(0.0));
        cfg.noiseFadeTime  = static_cast<float>((*a)["noise_fade_time"].value_or(0.0));
    }

    return cfg;
}

static constexpr std::string_view kAsteroidPath = "games/asteroids/assets/configs/asteroid_sizes.toml";

static void loadAsteroidConfigs() {
    const auto root = Engine::ConfigLoader::load(kAsteroidPath);
    AsteroidSizeConfigs::All.clear();
    AsteroidSizeConfigs::All.push_back(asteroidFromToml(*root["large" ].as_table()));
    AsteroidSizeConfigs::All.push_back(asteroidFromToml(*root["medium"].as_table()));
    AsteroidSizeConfigs::All.push_back(asteroidFromToml(*root["small" ].as_table()));
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

static constexpr std::string_view kShipsPath = "games/asteroids/assets/configs/ships.toml";

static void loadShipConfigs() {
    const auto root = Engine::ConfigLoader::load(kShipsPath);
    ShipConfigs::All.clear();
    for (const auto& [key, val] : root) {
        if (const auto* t = val.as_table())
            ShipConfigs::All.push_back(shipFromToml(*t));
    }
}

// ---- Entry points -----------------------------------------------------------

void loadAllConfigs() {
    loadUfoConfigs();
    loadAsteroidConfigs();
    loadShipConfigs();
}

void watchAllConfigs() {
    Engine::Config::watch(kUfoPath,      loadUfoConfigs);
    Engine::Config::watch(kAsteroidPath, loadAsteroidConfigs);
    Engine::Config::watch(kShipsPath,    loadShipConfigs);
}

#ifdef ENABLE_TOOLS

// ---- toToml helpers ---------------------------------------------------------

static toml::table ufoToToml(const UfoConfig& c) {
    toml::table t;
    t.insert_or_assign("render_size",                static_cast<double>(c.renderSize / SCALE));
    t.insert_or_assign("speed",                      static_cast<double>(c.speed));
    t.insert_or_assign("aim_variance",               static_cast<double>(c.aimVariance));
    t.insert_or_assign("fire_rate",                  static_cast<double>(c.fireRate));
    t.insert_or_assign("sprite_frame",               static_cast<int64_t>(c.spriteFrame));
    t.insert_or_assign("score",                      static_cast<int64_t>(c.score));
    t.insert_or_assign("particle_count",             static_cast<int64_t>(c.particleCount));
    t.insert_or_assign("particle_speed",             static_cast<double>(c.particleSpeed));
    t.insert_or_assign("particle_speed_variance",    static_cast<double>(c.particleSpeedVariance));
    t.insert_or_assign("particle_lifetime",          static_cast<double>(c.particleLifetime));
    t.insert_or_assign("particle_lifetime_variance", static_cast<double>(c.particleLifetimeVariance));
    t.insert_or_assign("particle_size",              static_cast<double>(c.particleSize / SCALE));
    return t;
}

static toml::table asteroidToToml(const AsteroidSizeConfig& c) {
    toml::table frames;
    for (int v = 0; v < 4; ++v) {
        toml::array arr;
        for (int f = 0; f < 4; ++f)
            arr.push_back(static_cast<int64_t>(c.frames[v][f]));
        frames.insert_or_assign("variant_" + std::to_string(v), std::move(arr));
    }

    toml::table particles;
    particles.insert_or_assign("count",             static_cast<int64_t>(c.particleCount));
    particles.insert_or_assign("speed",             static_cast<double>(c.particleSpeed));
    particles.insert_or_assign("speed_variance",    static_cast<double>(c.particleSpeedVariance));
    particles.insert_or_assign("lifetime",          static_cast<double>(c.particleLifetime));
    particles.insert_or_assign("lifetime_variance", static_cast<double>(c.particleLifetimeVariance));
    particles.insert_or_assign("size",              static_cast<double>(c.particleSize / SCALE));

    toml::table audio;
    audio.insert_or_assign("noise_duration",  static_cast<double>(c.noiseDuration));
    audio.insert_or_assign("noise_amplitude", static_cast<double>(c.noiseAmplitude));
    audio.insert_or_assign("noise_fade_time", static_cast<double>(c.noiseFadeTime));

    toml::table t;
    t.insert_or_assign("cell_count", static_cast<int64_t>(c.cellCount));
    t.insert_or_assign("score",      static_cast<int64_t>(c.score));
    t.insert_or_assign("frames",     std::move(frames));
    t.insert_or_assign("particles",  std::move(particles));
    t.insert_or_assign("audio",      std::move(audio));
    return t;
}

static toml::table shipToToml(const ShipConfig& c) {
    toml::table t;
    t.insert_or_assign("name",           c.name);
    t.insert_or_assign("max_speed",      static_cast<double>(c.maxSpeed));
    t.insert_or_assign("thrust_force",   static_cast<double>(c.thrustForce));
    t.insert_or_assign("drag",           static_cast<double>(c.drag));
    t.insert_or_assign("rotate_speed",   static_cast<double>(c.rotateSpeed));
    t.insert_or_assign("fire_cooldown",  static_cast<double>(c.fireCooldown));
    t.insert_or_assign("ship_frame",     static_cast<int64_t>(c.shipFrame));
    t.insert_or_assign("thrust_frame_1", static_cast<int64_t>(c.thrustFrame1));
    t.insert_or_assign("thrust_frame_2", static_cast<int64_t>(c.thrustFrame2));
    return t;
}

// ---- Save functions ---------------------------------------------------------

static void saveUfoConfigs() {
    static constexpr std::string_view kKeys[] = {"large", "small"};
    toml::table root;
    for (size_t i = 0; i < UfoConfigs::All.size() && i < 2; ++i)
        root.insert_or_assign(std::string(kKeys[i]), ufoToToml(UfoConfigs::All[i]));
    std::ofstream out{std::string(kUfoPath)};
    out << root;
}

static void saveAsteroidConfigs() {
    static constexpr std::string_view kKeys[] = {"large", "medium", "small"};
    toml::table root;
    for (size_t i = 0; i < AsteroidSizeConfigs::All.size() && i < 3; ++i)
        root.insert_or_assign(std::string(kKeys[i]), asteroidToToml(AsteroidSizeConfigs::All[i]));
    std::ofstream out{std::string(kAsteroidPath)};
    out << root;
}

static void saveShipConfigs() {
    toml::table root;
    for (const auto& ship : ShipConfigs::All) {
        std::string key = ship.name;
        std::transform(key.begin(), key.end(), key.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        root.insert_or_assign(key, shipToToml(ship));
    }
    std::ofstream out{std::string(kShipsPath)};
    out << root;
}

// ---- Editor -----------------------------------------------------------------

void renderConfigEditor() {
    ImGui::Begin("Config Editor");

    if (ImGui::CollapsingHeader("UFOs")) {
        ImGui::PushID("ufos");
        static const char* kLabels[] = {"Large", "Small"};
        for (int i = 0; i < static_cast<int>(UfoConfigs::All.size()); ++i) {
            auto& c = UfoConfigs::All[i];
            ImGui::PushID(i);
            if (ImGui::TreeNode(i < 2 ? kLabels[i] : "Unknown")) {
                ImGui::DragFloat("Render Size",        &c.renderSize,              0.5f);
                ImGui::DragFloat("Speed",              &c.speed,                   1.f);
                ImGui::DragFloat("Aim Variance",       &c.aimVariance,             0.01f);
                ImGui::DragFloat("Fire Rate",          &c.fireRate,                0.01f);
                ImGui::DragInt(  "Sprite Frame",       &c.spriteFrame);
                ImGui::DragInt(  "Score",              &c.score);
                ImGui::DragInt(  "Particle Count",     &c.particleCount);
                ImGui::DragFloat("Particle Speed",     &c.particleSpeed,           1.f);
                ImGui::DragFloat("Particle Speed Var", &c.particleSpeedVariance,   1.f);
                ImGui::DragFloat("Particle Lifetime",  &c.particleLifetime,        0.01f);
                ImGui::DragFloat("Particle Life Var",  &c.particleLifetimeVariance,0.01f);
                ImGui::DragFloat("Particle Size",      &c.particleSize,            0.5f);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::PopID();
        if (ImGui::Button("Save UFO Configs")) saveUfoConfigs();
    }

    if (ImGui::CollapsingHeader("Asteroids")) {
        ImGui::PushID("asteroids");
        static const char* kLabels[] = {"Large", "Medium", "Small"};
        for (int i = 0; i < static_cast<int>(AsteroidSizeConfigs::All.size()); ++i) {
            auto& c = AsteroidSizeConfigs::All[i];
            ImGui::PushID(i);
            if (ImGui::TreeNode(i < 3 ? kLabels[i] : "Unknown")) {
                ImGui::DragInt(  "Cell Count",         &c.cellCount);
                ImGui::DragInt(  "Score",              &c.score);
                ImGui::DragInt(  "Particle Count",     &c.particleCount);
                ImGui::DragFloat("Particle Speed",     &c.particleSpeed,           1.f);
                ImGui::DragFloat("Particle Speed Var", &c.particleSpeedVariance,   1.f);
                ImGui::DragFloat("Particle Lifetime",  &c.particleLifetime,        0.01f);
                ImGui::DragFloat("Particle Life Var",  &c.particleLifetimeVariance,0.01f);
                ImGui::DragFloat("Particle Size",      &c.particleSize,            0.5f);
                ImGui::DragFloat("Noise Duration",     &c.noiseDuration,           0.01f);
                ImGui::DragFloat("Noise Amplitude",    &c.noiseAmplitude,          0.01f);
                ImGui::DragFloat("Noise Fade Time",    &c.noiseFadeTime,           0.01f);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::PopID();
        if (ImGui::Button("Save Asteroid Configs")) saveAsteroidConfigs();
    }

    if (ImGui::CollapsingHeader("Ships")) {
        ImGui::PushID("ships");
        for (int i = 0; i < static_cast<int>(ShipConfigs::All.size()); ++i) {
            auto& c = ShipConfigs::All[i];
            ImGui::PushID(i);
            if (ImGui::TreeNode(c.name.c_str())) {
                ImGui::DragFloat("Max Speed",      &c.maxSpeed,     1.f);
                ImGui::DragFloat("Thrust Force",   &c.thrustForce,  1.f);
                ImGui::DragFloat("Drag",           &c.drag,         0.001f, 0.f, 1.f);
                ImGui::DragFloat("Rotate Speed",   &c.rotateSpeed,  0.01f);
                ImGui::DragFloat("Fire Cooldown",  &c.fireCooldown, 0.01f);
                ImGui::DragInt(  "Ship Frame",     &c.shipFrame);
                ImGui::DragInt(  "Thrust Frame 1", &c.thrustFrame1);
                ImGui::DragInt(  "Thrust Frame 2", &c.thrustFrame2);
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        ImGui::PopID();
        if (ImGui::Button("Save Ship Configs")) saveShipConfigs();
    }

    ImGui::End();
}

#endif // ENABLE_TOOLS
