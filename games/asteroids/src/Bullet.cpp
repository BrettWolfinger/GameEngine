#include "Bullet.h"

void Bullet::update(float dt) {
    pos += vel * dt;
    lifetime -= dt;
}

void Bullet::render(Engine::Renderer2D& renderer) const {
    const float half = SIZE * 0.5f;
    if (tex)
        renderer.drawTexturedRect(pos.x - half, pos.y - half, SIZE, SIZE,
                                  *tex, uv.u0, uv.v0, uv.u1, uv.v1);
    else
        renderer.drawRect(pos.x - half, pos.y - half, SIZE, SIZE, { 1.f, 1.f, 1.f, 1.f });
}
