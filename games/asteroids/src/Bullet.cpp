#include "Bullet.h"

void Bullet::update(float dt) {
    pos += vel * dt;
    lifetime -= dt;
}

void Bullet::render(Engine::Renderer2D& renderer) const {
    renderer.drawRect(pos.x - SIZE * 0.5f, pos.y - SIZE * 0.5f, SIZE, SIZE,
                      { 1.f, 1.f, 1.f, 1.f });
}
