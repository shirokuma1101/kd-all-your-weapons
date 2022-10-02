#pragma once

#include "Application/Game/GameObject/ModelObject/ModelObject.h"

class DynamicObject : public ModelObject {
public:

    //TODO: 敵つくる、enemylist?とhpとダメージ実装
    //TODO: 持っていられる時間を大きさに反比例させる、
    //TODO: タイトルつくる
    //TODO: チャージ時、ショット時のエフェクト追加

    DynamicObject(std::string_view name)
        : ModelObject(name)
    {}

    void PreUpdate() override {
        m_hasInHand = false;
    }

    void Update(float delta_time) override {
        if (m_hasInHand) {
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
            ModelObject::Update(delta_time);
            if (m_pRigidActor && m_pRigidActor->is<physx::PxRigidDynamic>()) {
                SetMatrix(physx_helper::ToMatrix(m_pRigidActor->getGlobalPose()));
            }
        }

        if (m_spRigidActorHolder) {
            m_spRigidActorHolder->Update();
            auto vec = m_spRigidActorHolder->GetMoveVector();
        }
    }

    void SetHasInHand(bool has_in_hand) {
        m_hasInHand = has_in_hand;
    }

    void Force(const Math::Vector3& dir, float power) noexcept {
        Math::Vector3 force_dir = dir;
        force_dir.Normalize();
        m_force = force_dir * power;
    }

private:

    Math::Vector3 m_force;
    bool          m_isFirstImpulse = false;
    bool          m_hasInHand = false;
    
    float         m_weight = 0.f;

};
