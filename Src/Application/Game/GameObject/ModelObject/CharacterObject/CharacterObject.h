#pragma once

#include "Application/Game/GameObject/ModelObject/ModelObject.h"

class CharacterObject : public ModelObject
{
public:

    CharacterObject(std::string_view name)
        : ModelObject(name)
    {}
    virtual ~CharacterObject() {}

    float GetHealth() const noexcept {
        return m_health;
    }
    void AddDamage(float damage) noexcept {
        m_health -= damage;
        if (m_health < 0.f) {
            m_health = 0.f;
        }
    }
    void AddHealth(float health) noexcept {
        m_health += health;
    }

protected:
    
    float m_health = 0.f;

    // アニメーション
    KdAnimator m_animator;
    
};
