#pragma once
#include "Asteroid.h"
#include "Bullet.h"
#include "Ship.h"
#include <engine/renderer/Renderer2D.h>
#include <engine/renderer/SpriteSheet.h>
#include <engine/core/SaveData.h>
#include <engine/ui/Menu.h>
#include <memory>
#include <optional>
#include <random>
#include <vector>

enum class Screen { Title, ShipSelect, Playing, GameOver, Quit };

struct GameContext {
    Engine::Renderer2D&                     renderer;
    std::shared_ptr<Engine::SpriteSheet>&   sheet;
    std::mt19937&                           rng;
    Engine::SaveData&                       saveData;
    Engine::Menu&                           titleMenu;

    std::vector<std::unique_ptr<Asteroid>>& bgAsteroids;
    std::optional<Ship>&                    ship;
    std::vector<std::unique_ptr<Bullet>>&   bullets;
    std::vector<std::unique_ptr<Asteroid>>& asteroids;

    int&   score;
    int&   highScore;
    int&   lives;
    int&   nextLifeScore;
    int&   wave;
    float& waveTimer;
    bool&  newHighScore;
    int&   selectedShip;
};
