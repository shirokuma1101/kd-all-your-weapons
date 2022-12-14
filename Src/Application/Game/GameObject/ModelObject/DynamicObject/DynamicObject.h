#pragma once

#include "Application/Game/GameObject/ModelObject/ModelObject.h"

class DynamicObject : public ModelObject {
public:

    enum class Selection {
        NoSelected,
        Equippable,
        NotEquippable,
    };

    DynamicObject(std::string_view name)
        : ModelObject(name)
    {}

    /* GameObject */
    void PreUpdate() override;
    void Update(float delta_time) override;
    void DrawOpaque() override;

    /* DynamicObject */

    bool Collision() override;

    float GetWeight() const noexcept {
        return m_weight;
    }
    void SetEquipping(bool equipping) noexcept {
        m_isEquipping = equipping;
    }
    void SetSelection(Selection selection) noexcept {
        m_selection = selection;
    }

    void Force(const Math::Vector3& dir, float power) noexcept {
        Math::Vector3 force_dir = dir;
        force_dir.Normalize();
        m_force = force_dir * power;
    }

private:

    bool          m_isLoaded       = false;
    float         m_weight         = 0.f;

    bool          m_isEquipping    = false;                 // 装備しているか
    Math::Vector3 m_force;                                  // 力の強さ
    bool          m_isFirstImpulse = false;                 // 一度のみ衝撃を与える
    Selection     m_selection      = Selection::NoSelected; // 選択中のオブジェクトの状態

};
