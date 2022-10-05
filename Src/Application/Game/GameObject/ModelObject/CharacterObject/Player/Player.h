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
        ImGui::Text("Power: %.2f", m_nowChargePower);
        ImGui::Text("CT: %.2f", m_nowCT);
        ImGui::End();
    }

    void SetFollowerTarget(std::shared_ptr<GameObject> obj) {
        m_wpFollowerCamera = obj;
    }
    void SetEquipWeightLimit(float equip_weight_limit) {
        m_equipWeightLimit = equip_weight_limit;
    }

private:

    void MouseOperator(float narrow_limit, float wide_limit);

    void KeyOperator(float delta_time);

    bool Collision();

    void Shot(float delta_time);

    // ポーズ画面を表示
    bool m_isPause = false;

    // カメラ
    std::weak_ptr<GameObject> m_wpFollowerCamera;
    
    // 移動系
    float m_initialVelocity = 0.f;
    float m_jumpTime        = 0.f;

    // 攻撃系
    std::weak_ptr<DynamicObject> m_wpEquipObject;
    float m_equipWeightLimit    = 0.f;

    float m_nowCT               = 0.f;
    bool  m_isShotCT            = false;
    float m_nowChargePower      = 0.f;
    bool  m_isSucceededChargeCT = false;
    bool  m_isFailedChargeCT    = false;

    // エフェクト
    std::weak_ptr<effekseer_helper::EffectTransform> m_wpEffectTransform;

};
