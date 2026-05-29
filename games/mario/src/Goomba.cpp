#include "Goomba.h"

Goomba::Goomba(std::shared_ptr<Engine::SpriteSheet> sheet, float x, float y)
    : m_x(x), m_y(y), m_animator(std::move(sheet))
{
    m_animator.addClip("walk", { {0, 1}, 0.15f });
    m_animator.setClip("walk");
}

void Goomba::update(float dt, float gravity, const GoombaConfig& cfg, const Engine::Tilemap::Collider& collider) {
    if (m_dead) return;
    m_vy += gravity * dt;
    resolveCollision(dt, cfg, collider);
    m_animator.update(dt);
}

void Goomba::resolveCollision(float dt, const GoombaConfig& cfg, const Engine::Tilemap::Collider& collider) {
    const float vx  = static_cast<float>(m_dir) * static_cast<float>(cfg.walkSpeed);
    const auto  hit = collider.sweep(m_x, m_y, hitboxW(), hitboxH(), vx * dt, m_vy * dt);
    m_x += hit.dx;
    m_y += hit.dy;
    if (hit.hitX) m_dir = -m_dir;
    if (hit.hitY) m_vy  = 0.f;
}

void Goomba::render(Engine::Renderer2D& renderer, float cameraX, int layer) const {
    if (m_dead) return;
    const Engine::UVRect uv = m_animator.currentFrameUVs();
    const float w  = hitboxW();
    const float h  = hitboxH();
    const float u0 = m_dir >= 0 ? uv.u1 : uv.u0;
    const float u1 = m_dir >= 0 ? uv.u0 : uv.u1;
    renderer.drawTexturedRect(m_x - cameraX, m_y, w, h,
                              m_animator.sheet().texture(),
                              u0, uv.v0, u1, uv.v1,
                              0.f, { 1.f, 1.f, 1.f, 1.f }, layer);
}

void Goomba::stomp() {
    m_dead = true;
}

Goomba::~Goomba() { deregisterColliders(); }

void Goomba::registerColliders(IContactable* contactable, ContactEffect stompEffect) {
    const float headH = static_cast<float>(GOOMBA_HEAD_H * SCALE);
    const float bodyH = hitboxH() - headH;

    m_headHandle = Engine::Collision::add(
        Engine::Collision::ColliderDesc::makeAABB(
            kLayerEnemyHead, kLayerPlayerStomp,
            hitboxX(), hitboxY(), hitboxW(), headH),
        [this, headH] {
            Engine::Collision::updateAABB(m_headHandle,
                hitboxX(), hitboxY(), hitboxW(), headH);
        },
        [this, contactable, stompEffect](Engine::Collision::ColliderHandle, Engine::Collision::ColliderHandle) {
            if (!m_dead) {
                stomp();
                contactable->applyContactEffect(stompEffect);
            }
        });

    m_bodyHandle = Engine::Collision::add(
        Engine::Collision::ColliderDesc::makeAABB(
            kLayerEnemyBody, kLayerPlayer,
            hitboxX(), hitboxY() + headH, hitboxW(), bodyH),
        [this, headH, bodyH] {
            Engine::Collision::updateAABB(m_bodyHandle,
                hitboxX(), hitboxY() + headH, hitboxW(), bodyH);
        },
        [this, contactable](Engine::Collision::ColliderHandle, Engine::Collision::ColliderHandle) {
            if (!m_dead) contactable->applyContactEffect({ ContactEffect::Type::Kill });
        });
}

void Goomba::deregisterColliders() {
    Engine::Collision::remove(m_headHandle);
    Engine::Collision::remove(m_bodyHandle);
    m_headHandle = m_bodyHandle = Engine::Collision::NULL_COLLIDER;
}
