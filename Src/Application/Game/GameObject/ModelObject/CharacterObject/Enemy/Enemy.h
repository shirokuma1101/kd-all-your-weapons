#pragma once

#include "Application/Game/GameObject/ModelObject/CharacterObject/CharacterObject.h"
#include "Application/Game/GameObject/ModelObject/CharacterObject/Player/Player.h"

class Enemy : public CharacterObject {
public:

    Enemy(std::string_view name)
        : CharacterObject(name)
    {}

    void Init() override;
    void Update(float delta_time) override;
    void DrawOpaque() override;

    void SetTarget(const std::shared_ptr<Player>& target) {
        m_wpTargetPlayer = target;
    }
    
private:

    void PlayAnimation(float delta_time);

    float m_attackInterval = 0.f;
    float m_attackPower    = 0.f;

    // アニメーション
    KdAnimator m_animator;

    std::weak_ptr<Player> m_wpTargetPlayer;
    
};
