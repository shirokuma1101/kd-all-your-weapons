#pragma once

#include "ExternalDependencies/Effekseer/EffekseerManager.h"
#include "ExternalDependencies/Input/InputManager.h"
#include "ExternalDependencies/PhysX/PhysXManager.h"

#include "System/Asset/AssetManager/AssetManager.h"
#include "System/GameObject/GameObject.h"

class ModelObject : public GameObject
{
public:

    ModelObject(std::string_view name)
        : m_name(name)
    {}

    /* GameObject */

    virtual void Init() override;
    virtual void Update(float delta_time) override;
    virtual void DrawOpaque() override;


    /* ModelObject */
    
    virtual std::shared_ptr<const KdModelWork> GetModel() const final {
        return m_spModel;
    }
    virtual bool IsDynamic() const final {
        return m_pRigidActor && m_pRigidActor->is<physx::PxRigidDynamic>();
    }

    virtual void SetModel(std::shared_ptr<KdModelWork> model) noexcept final {
        m_spModel = model;
    }
    virtual void SetRigidActorHolder(physx::PxRigidActor* actor) final {
        if (!actor) return;
        m_spRigidActorHolder = std::make_shared<physx_helper::RigidActorHolder>(actor);
    }
    
protected:

    virtual void CreatePrimitiveCollision();

    virtual void CreateModelCollision();

    virtual bool Collision();

    const std::string                               m_name;
    std::string                                     m_collisionNodeName;
    std::shared_ptr<KdModelWork>                    m_spModel            = nullptr;
    physx::PxRigidActor*                            m_pRigidActor        = nullptr;
    std::shared_ptr<physx_helper::RigidActorHolder> m_spRigidActorHolder = nullptr;

};
