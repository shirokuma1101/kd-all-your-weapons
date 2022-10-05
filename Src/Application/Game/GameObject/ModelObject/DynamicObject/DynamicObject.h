#pragma once

#include "Application/Game/GameObject/ModelObject/ModelObject.h"

class DynamicObject : public ModelObject {
public:

    //TODO: 敵つくる、enemylist?とhpとダメージ実装
    //TODO: タイトルつくる

    enum class Selection {
        NoSelected,
        Equippable,
        NotEquippable,
    };

    DynamicObject(std::string_view name)
        : ModelObject(name)
    {}

    void PreUpdate() override {
        m_isEquipping = false;
        m_selection = Selection::NoSelected;
    }

    void Update(float delta_time) override {
        auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
        auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();
        
        m_weight = (*jm)[m_name]["expand"]["weight"];
        
        ModelObject::Update(delta_time);
        if (!mm->IsLoaded(m_name)) return;

        // Modelロード時のみ実行
        if (!m_isLoaded) {
            m_isLoaded = true;
            physx_helper::PutToSleep(m_pRigidActor);
        }
        
        if (m_isEquipping) {
            m_pRigidActor->setGlobalPose(physx::PxTransform(physx_helper::ToPxMat44(m_transform.matrix)));
            physx_helper::PutToSleep(m_pRigidActor);
            m_isFirstImpulse = true;
        }
        else {
            if (physx_helper::IsSleeping(m_pRigidActor)) {
                if (m_isFirstImpulse) {
                    physx_helper::AddForce(m_pRigidActor, m_force, physx::PxForceMode::eIMPULSE);
                    m_isFirstImpulse = false;
                }
            }
            if (m_pRigidActor && m_pRigidActor->is<physx::PxRigidDynamic>()) {
                SetMatrix(physx_helper::ToMatrix(m_pRigidActor->getGlobalPose()));
            }
        }

        if (m_spRigidActorHolder) {
            m_spRigidActorHolder->Update();
            auto vec = m_spRigidActorHolder->GetMoveVector();
        }
    }

    void DrawOpaque() override {
        auto& rim_light = DirectX11System::Instance().GetShaderManager()->m_standardShader.RimLightCB();
        
        switch (m_selection) {
        case DynamicObject::Selection::NoSelected:
            rim_light.Get()->RimPower = 0.f;
            break;
        case DynamicObject::Selection::Equippable:
            rim_light.Get()->RimColor = { 0.f, 1.f, 0.f };
            rim_light.Get()->RimPower = 1.f;
            break;
        case DynamicObject::Selection::NotEquippable:
            rim_light.Get()->RimColor = { 1.f, 0.f, 0.f };
            rim_light.Get()->RimPower = 1.f;
            break;
        default:
            break;
        }
        
        rim_light.Write();
        ModelObject::DrawOpaque();
        rim_light.Get()->RimPower = 0.f;
        rim_light.Write();
    }

    float GetWeight() const noexcept {
        return m_weight;
    }
    void SetEquipping(bool equipping) {
        m_isEquipping = equipping;
    }
    void SetSelection(Selection selection) {
        m_selection = selection;
    }

    void Force(const Math::Vector3& dir, float power) noexcept {
        Math::Vector3 force_dir = dir;
        force_dir.Normalize();
        m_force = force_dir * power;
    }

private:

    float         m_weight         = 0.f;
    bool          m_isLoaded       = false;

    Math::Vector3 m_force;                                  // 力の強さ
    bool          m_isFirstImpulse = false;                 // 一度のみ衝撃を与える
    bool          m_isEquipping    = false;                 // 装備しているか
    Selection     m_selection      = Selection::NoSelected; // 選択中のオブジェクトの状態

};
