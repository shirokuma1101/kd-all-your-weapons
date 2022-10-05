#pragma once

#include "System/GameObject/GameObject.h"

class CameraObject : public GameObject
{
public:

    CameraObject(const Math::Matrix& camera_matrix, float fov = 60.f, float aspect = 16.f / 9.f , float near_clipping_distance = 0.001f, float far_clipping_distance = 2000.f)
        : m_spCamera(std::make_shared<CameraSystem>(camera_matrix, fov, aspect, near_clipping_distance, far_clipping_distance))
    {}

    virtual void Init() override {
        m_spCamera->SetToShader();
    }

    virtual void Update(float delta_time) override {
        /* メッシュに埋まらないように光線を利用して当たり判定を行う */
        static const float ray_offset = 0.1f; // ターゲットとカメラの距離に対するオフセット
        
        // ターゲットの位置と回転のみ保存
        auto& target_transform = m_wpFollowingTarget.lock()->GetTransform();
        m_transform.position = target_transform.position;
        m_transform.rotation = target_transform.rotation;

        // ローカル行列を掛けた行列を作成
        Math::Matrix compositied = m_localTransform.matrix * m_transform.Composition();

        // プレイヤーからカメラの位置への方向ベクトルを取得
        Math::Vector3 dir = compositied.Translation() - m_transform.matrix.Translation();
        dir.Normalize();

        // 当たり判定
        std::list<collision::Result> results;
        collision::Ray ray(m_transform.matrix.Translation(), dir, m_localTransform.matrix.Translation().Length() + ray_offset);
        for (const auto& e : Application::Instance().GetGameSystem()->GetGameObjects()) {
            const auto& collider = e->GetCollider();
            if (!collider) continue;
            // 判定 (光線)
            if (collider->Intersects(DefaultCollisionType::Bump, e->GetTransform().matrix, ray, &results)) {
                if (auto result = collision::GetNearest(results); result) {
                    m_transform.position += result->direction * result->depth;
                }
                results.clear();
            }
        }

        // 当たり判定後に再計算
        m_spCamera->SetMatrix(m_transform.matrix = m_localTransform.matrix * m_transform.Composition());
        m_spCamera->SetToShader();
    }

    virtual const std::shared_ptr<CameraSystem>& GetCamera() const final {
        return m_spCamera;
    }
    virtual std::shared_ptr<CameraSystem> GetCamera() final {
        return m_spCamera;
    }

    virtual void SetFollowingTarget(std::shared_ptr<GameObject> obj, const Transform& local_transform = {}) final {
        m_wpFollowingTarget = obj;
        // ローカルの位置と回転のみ保存
        m_localTransform.position = local_transform.position;
        m_localTransform.rotation = local_transform.rotation;
        m_localTransform.Composition();
    }

protected:

    std::shared_ptr<CameraSystem> m_spCamera;
    std::weak_ptr<GameObject>     m_wpFollowingTarget;
    Transform                     m_localTransform;

};
