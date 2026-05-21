#include "Asteroid.h"
#include "AsteroidsConfig.h"

// ---- frame table definitions (constexpr must be defined in exactly one TU) ----
constexpr int Asteroid::MEDIUM_FRAMES[4];
constexpr int Asteroid::SMALL_FRAMES[4][4];

// ---- factory helpers --------------------------------------------------------

Asteroid Asteroid::makeLarge(glm::vec2 pos, glm::vec2 vel, float rotSpeed) {
    Asteroid a;
    a.pos       = pos;
    a.vel       = vel;
    a.rotSpeed  = rotSpeed;
    a.size      = AsteroidSize::Large;
    a.frameIndex = 196;   // top-left of the 4×4 large asteroid sprite
    a.cellCount  = 4;
    return a;
}

Asteroid Asteroid::makeMedium(int variant, glm::vec2 pos, glm::vec2 vel, float rotSpeed) {
    Asteroid a;
    a.pos        = pos;
    a.vel        = vel;
    a.rotSpeed   = rotSpeed;
    a.size       = AsteroidSize::Medium;
    a.frameIndex = MEDIUM_FRAMES[variant & 3];
    a.cellCount  = 2;
    return a;
}

Asteroid Asteroid::makeSmall(int group, int idx, glm::vec2 pos, glm::vec2 vel, float rotSpeed) {
    Asteroid a;
    a.pos        = pos;
    a.vel        = vel;
    a.rotSpeed   = rotSpeed;
    a.size       = AsteroidSize::Small;
    a.frameIndex = SMALL_FRAMES[group & 3][idx & 3];
    a.cellCount  = 1;
    return a;
}

// ---- update -----------------------------------------------------------------

void Asteroid::update(float dt, int screenW, int screenH) {
    pos   += vel * dt;
    angle += rotSpeed * dt;

    // Screen wrap — same margin convention as Ship (-16/+16)
    if (pos.x < -16.f)              pos.x += screenW + 32.f;
    if (pos.x > screenW + 16.f)     pos.x -= screenW + 32.f;
    if (pos.y < -16.f)              pos.y += screenH + 32.f;
    if (pos.y > screenH + 16.f)     pos.y -= screenH + 32.f;
}

// ---- render -----------------------------------------------------------------

void Asteroid::render(Engine::Renderer2D& renderer, const Engine::SpriteSheet& sheet) const {
    const float renderSize = cellCount * 16.f * SCALE;
    const float half       = renderSize * 0.5f;

    const Engine::UVRect uv = sheet.getFrameUVs(frameIndex, cellCount, cellCount);
    renderer.drawTexturedRect(pos.x - half, pos.y - half, renderSize, renderSize,
                              sheet.texture(),
                              uv.u0, uv.v0, uv.u1, uv.v1,
                              angle);
}
