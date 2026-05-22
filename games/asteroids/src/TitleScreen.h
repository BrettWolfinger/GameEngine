#pragma once
#include "GameContext.h"

class TitleScreen {
public:
    explicit TitleScreen(GameContext& ctx);

    void   onEnter();
    void   preStep(float dt);
    Screen update(float dt);
    void   render();

private:
    GameContext& m_ctx;
};
