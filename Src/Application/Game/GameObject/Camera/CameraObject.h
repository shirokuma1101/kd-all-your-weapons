#pragma once

#include "System/GameObject/GameObject.h"

class CameraObject : public GameObject
{
public:

    virtual void Init() override {
        m_spCamera = std::make_shared<CameraSystem>();
        float aspect = 16.f / 9.f;
        m_spCamera->SetProjection(60.f, aspect, 0.01f, 2000.f);
    }

    virtual void Update(float delta_time) override {
        auto& target = m_wpFollowingTarget.lock()->GetTransform();
        m_transform.position = target.position;
        m_transform.rotation = target.rotation;
        m_spCamera->SetMatrix(m_localMatrix * m_transform.Composition());
        m_spCamera->SetToShader();
    }

    virtual const DirectX::SimpleMath::Matrix GetMatrix() const noexcept {
        return m_localMatrix * m_transform.matrix;
    }

    virtual std::shared_ptr<CameraSystem> GetCamera() final {
        return m_spCamera;
    }

    virtual void SetFollowingTarget(std::shared_ptr<GameObject> obj, const Math::Matrix& local_mat = {}) final {
        m_wpFollowingTarget = obj;
        m_localMatrix = local_mat;
    }

protected:

    std::shared_ptr<CameraSystem> m_spCamera;
    std::weak_ptr<GameObject>     m_wpFollowingTarget;
    Math::Matrix                  m_localMatrix;

};
