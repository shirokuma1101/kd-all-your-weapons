#pragma once

#include "Application/Game/GameObject/ModelObject/CharacterObject/CharacterObject.h"
#include "Application/Game/GameObject/CameraObject/CameraObject.h"

class Player : public CharacterObject {
public:

    Player(std::string_view name)
        : CharacterObject(name)
    {}

    void Init() override;

    void Update(float delta_time) override;
    
    void DrawOpaque() override;

    void ImGuiUpdate() override {
        ImGui::Begin("config 1");
        ImGui::Text("Power: %.2f", m_nowChargePower);
        ImGui::Text("CT: %.2f", m_nowCT);
        ImGui::Text("ppx: %.2f", m_transform.position.x);
        ImGui::Text("ppy: %.2f", m_transform.position.y);
        ImGui::Text("ppz: %.2f", m_transform.position.z);
        ImGui::End();
    }

    void SetFollowerTarget(std::shared_ptr<CameraObject> obj) {
        m_wpFollowerCamera = obj;
    }
    void SetEquipWeightLimit(float equip_weight_limit) {
        m_equipWeightLimit = equip_weight_limit;
    }

private:

    void MouseOperator(float delta_time, float narrow_limit, float wide_limit);

    void KeyOperator(float delta_time);

    void Shot(float delta_time);

    void SetCameraTransform(float delta_time);

    void PlayAnimation(float delta_time);

    // カメラ
    std::weak_ptr<CameraObject> m_wpFollowerCamera;
    Math::Vector3 m_cameraRotaion;
    float m_zoomTime = 0.f;
    bool  m_isZoom   = false;
    
    // 移動系
    float m_initialVelocity = 0.f;
    float m_jumpTime        = 0.f;
    Math::Vector3 m_moveDirection;
    float m_angle           = 0.f;

    // 攻撃系
    std::weak_ptr<DynamicObject> m_wpEquipObject;
    float m_equipWeightLimit    = 0.f;

    float m_nowCT               = 0.f;
    bool  m_isShotCT            = false;
    float m_nowChargePower      = 0.f;
    bool  m_isSucceededChargeCT = false;
    bool  m_isFailedChargeCT    = false;

    // Gravity Gunの行列
    Math::Matrix m_gravityGunMatrix;
    // プレイヤーのアニメーション
    KdAnimator m_animator;
    // SE
    std::weak_ptr<audio_helper::SoundInstance> m_wpChargeSE;
    // エフェクト
    std::weak_ptr<effekseer_helper::EffectTransform> m_wpEffectTransform;

};
