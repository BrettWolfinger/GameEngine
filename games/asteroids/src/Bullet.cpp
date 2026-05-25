#include "Bullet.h"
#include "AsteroidsConfig.h"
#include <engine/Engine.h>

Bullet::Bullet(glm::vec2 pos, glm::vec2 vel, Engine::UVRect uv, const Engine::Texture* tex,
               uint32_t selfLayer, uint32_t targetLayer)
    : pos(pos), vel(vel), lifetime(LIFETIME), uv(uv), tex(tex)
{
    m_colliderHandle = Engine::Collision::add(
        Engine::ColliderDesc::makeCircle(selfLayer, targetLayer, pos.x, pos.y, SIZE * 0.5f),
        [this](Engine::ColliderHandle self, Engine::ColliderHandle) {
            lifetime = 0.f;
            Engine::Collision::remove(self);
            m_colliderHandle = Engine::NULL_COLLIDER;
        });
}

Bullet::~Bullet() {
    Engine::Collision::remove(m_colliderHandle);
}

void Bullet::update(float dt) {
    pos      += vel * dt;
    lifetime -= dt;

    if (m_colliderHandle != Engine::NULL_COLLIDER)
        Engine::Collision::updateCircle(m_colliderHandle, pos.x, pos.y, SIZE * 0.5f);
}

void Bullet::render(Engine::Renderer2D& renderer) const {
    const float half = SIZE * 0.5f;
    if (tex)
        renderer.drawTexturedRect(pos.x - half, pos.y - half, SIZE, SIZE,
                                  *tex, uv.u0, uv.v0, uv.u1, uv.v1);
    else
        renderer.drawRect(pos.x - half, pos.y - half, SIZE, SIZE, { 1.f, 1.f, 1.f, 1.f });
}
