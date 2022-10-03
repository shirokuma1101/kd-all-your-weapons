#pragma once

#include "Application/Game/GameObject/ModelObject/CharacterObject/CharacterObject.h"

class Player : public CharacterObject {
public:

    Player(std::string_view name)
        : CharacterObject(name)
    {}

    void Update(float delta_time) override;
    
    void DrawOpaque() override;

    void ImGuiUpdate() override {
        ImGui::Begin("config 1");
        ImGui::Text("power: %.2f", m_nowChargePower);
        ImGui::Text("cool_time: %.2f", m_nowCoolTime);
        ImGui::End();
    }

    void SetFollowerTarget(std::shared_ptr<GameObject> obj) {
        m_wpFollowerTarget = obj;
    }

private:

    void MouseOperator(float narrow_limit, float wide_limit);

    void KeyOperator(float delta_time);

    Math::Vector3 GetWasdKey(const Math::Vector3& rot) {
        auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();
        auto mat = Math::Matrix::CreateFromYawPitchRoll(convert::ToRadians(rot));
        Math::Vector3 dir;
        
        if (km->GetState('W')) {
            dir += mat.Backward();
            dir.y = 0.f;
        }
        if (km->GetState('S')) {
            dir += mat.Forward();
            dir.y = 0.f;
        }
        if (km->GetState('D')) {
            dir += mat.Right();
        }
        if (km->GetState('A')) {
            dir += mat.Left();
        }
        
        dir.Normalize();
        return dir;
    }
    
    float m_initialVelocity = 0.f;
    float m_jumpTime        = 0.f;

    float m_nowChargePower      = 0.f;
    bool  m_isSucceededCoolTime = false;
    bool  m_isFailedCoolTime    = false;
    float m_nowCoolTime         = 0.f;

    std::weak_ptr<GameObject> m_wpFollowerTarget;

};
