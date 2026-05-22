#pragma once
#include "GameContext.h"

class ShipSelectScreen {
public:
    explicit ShipSelectScreen(GameContext& ctx);

    void   preStep(float dt);
    Screen update(float dt);
    void   render();

private:
    GameContext& m_ctx;
};
