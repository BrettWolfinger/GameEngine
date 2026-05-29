# IContactable / ContactEffect (Game-Level Pattern)

## Overview

A lightweight interface pattern for entity-vs-entity collision response. When two colliders overlap, the initiating entity (e.g. an enemy) calls `applyContactEffect` on the target via an `IContactable*` captured at registration time. The target handles its own response via a single switch. Neither party needs to know the concrete type of the other.

---

## The Two Types

`Contactable.h` defines both:

```cpp
struct ContactEffect {
    enum class Type { Kill, Bounce /*, add new effects here */ };
    Type  type;
    float value = 0.f; // payload — e.g. bounce velocity
};

struct IContactable {
    virtual void applyContactEffect(const ContactEffect& effect) = 0;
    virtual ~IContactable() = default;
};
```

`ContactEffect` is plain data — no dependencies. `IContactable` is a pure interface.

---

## How It Works

**The target** (e.g. Player) implements `IContactable`:

```cpp
class Player : public IContactable {
public:
    void applyContactEffect(const ContactEffect& effect) override {
        if (m_dead) return;
        switch (effect.type) {
            case ContactEffect::Type::Kill:
                m_dead = true;
                break;
            case ContactEffect::Type::Bounce:
                m_vy       = effect.value;
                m_onGround = false;
                break;
        }
    }
};
```

**The initiator** (e.g. Goomba) captures `IContactable*` and a `ContactEffect` at collider registration time:

```cpp
void Goomba::registerColliders(IContactable* contactable, ContactEffect stompEffect) {
    // head zone — stomp
    m_headHandle = Engine::Collision::add(desc, syncFn,
        [this, contactable, stompEffect](...) {
            if (!m_dead) {
                stomp();
                contactable->applyContactEffect(stompEffect);
            }
        });

    // body zone — side hit
    m_bodyHandle = Engine::Collision::add(desc, syncFn,
        [this, contactable](...) {
            if (!m_dead)
                contactable->applyContactEffect({ ContactEffect::Type::Kill });
        });
}
```

**The game class** is the only wiring point — it decides what effect each enemy produces and passes it at spawn time:

```cpp
goomba.registerColliders(m_player.get(),
    { ContactEffect::Type::Bounce, m_config.stompBounceVel });
```

---

## Key Properties

- **Initiator knows nothing about the target's concrete type** — only `IContactable*`
- **Target has one method** — adding a new enemy type adds zero code to the target
- **Adding a new effect type** — add a value to `ContactEffect::Type` and a case in the target's switch
- **The game class owns the wiring** — it decides what effect an enemy applies, which keeps enemy classes data-agnostic

---

## Limitations

- `ContactEffect` is captured by value at registration time — changes to effect parameters (e.g. via hot-reload config) don't propagate to already-registered entities without re-registering
- The `IContactable*` must remain valid for the lifetime of the registered colliders — owned by a stable container (e.g. `std::unique_ptr` in the game class)
- Works best when contact targets are known at spawn time; see issue #121 for the registry approach when that assumption breaks down

---

## When To Use

Use this pattern when:
- Multiple enemy/hazard types need to affect the same target in different ways
- You want to add new enemy types without modifying the target class
- Contact is point-to-point (one initiator, one target) rather than broadcast

Use events (`Engine::Events`) instead when:
- Multiple systems need to react to the same contact (score, audio, UI)
- The contact is an announcement rather than a direct instruction

The two compose naturally: the target's `applyContactEffect` applies the immediate physical response, then emits an event for downstream systems to react to.
