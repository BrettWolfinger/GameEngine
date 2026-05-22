#pragma once
#include "GameContext.h"

class GameOverScreen {
public:
    explicit GameOverScreen(GameContext& ctx);

    Screen update(float dt);
    void   render();

private:
    GameContext& m_ctx;
};
