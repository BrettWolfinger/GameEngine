#pragma once

struct ContactEffect {
    enum class Type { Kill, Bounce };
    Type  type;
    float value = 0.f;
};

struct IContactable {
    virtual void applyContactEffect(const ContactEffect& effect) = 0;
    virtual ~IContactable() = default;
};
