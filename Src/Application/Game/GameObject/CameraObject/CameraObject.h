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

    virtual void Update(float) override {
        /* メッシュに埋まらないように光線を利用して当たり判定を行う */
        Collision();

        // 当たり判定後に再計算
        m_spCamera->SetMatrix(m_transform.matrix);
        m_spCamera->SetToShader();
    }

    virtual const std::shared_ptr<CameraSystem>& GetCamera() const final {
        return m_spCamera;
    }
    virtual std::shared_ptr<CameraSystem> GetCamera() final {
        return m_spCamera;
    }

    virtual void SetCameraTransform(const Transform& transform, const Transform& local_transform = {}) {
        // 位置と回転のみ保存
        m_transform.position      = transform.position;
        m_transform.rotation      = transform.rotation;
        m_localTransform.position = local_transform.position;
        m_localTransform.rotation = local_transform.rotation;
        m_transform.Composition();
        m_localTransform.Composition();
    }

protected:

    virtual void Collision() {
        auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();
        
        static const float head_to_waist_length = 0.7f; // MN: 頭から腰までの長さ
        static const float ray_offset           = 0.1f; // MN: 当たり判定の際の当たった場所から内側の距離オフセット
        
        Math::Matrix player_matrix
            = Math::Matrix::CreateFromYawPitchRoll(convert::ToRadians(m_transform.rotation))
            * Math::Matrix::CreateTranslation(m_transform.position);
        
        // 構え時は腰の角度によって移動した頭の行列を計算する
        if (km->GetState(VK_RBUTTON)) {
            Math::Matrix head_matrix
                = Math::Matrix::CreateTranslation({ 0.f, head_to_waist_length, 0.f })
                * Math::Matrix::CreateFromYawPitchRoll(convert::ToRadians(m_transform.rotation))
                * Math::Matrix::CreateTranslation(0.f, -head_to_waist_length, 0.f);
            // 頭の行列を考慮したプレイヤー行列を計算する
            player_matrix
                = head_matrix
                * Math::Matrix::CreateTranslation(m_transform.position);
        }
        
        // ローカル行列を合成したカメラの行列を計算する
        Math::Matrix camera_matrix
            = m_localTransform.Composition()
            * player_matrix;
        
        // プレイヤーからカメラへの方向ベクトルを取得
        Math::Vector3 dir = camera_matrix.Translation() - player_matrix.Translation();
        dir.Normalize();
        // 当たり判定
        std::list<collision::Result> results;
        collision::Ray ray(m_transform.matrix.Translation(), dir, m_localTransform.matrix.Translation().Length() + ray_offset);
        for (const auto& e : Application::Instance().GetGameSystem()->GetScene()->GetGameObjects()) {
            const auto& collider = e->GetCollider();
            if (!collider) continue;
            // 判定 (光線)
            if (collider->Intersects(DefaultCollisionType::Bump, e->GetTransform().matrix, ray, &results)) {
                if (auto result = collision::GetNearest(results); result) {
                    camera_matrix *= Math::Matrix::CreateTranslation(result->direction * result->depth);
                }
                results.clear();
            }
        }
        m_transform.matrix = camera_matrix;
    }

    std::shared_ptr<CameraSystem> m_spCamera;
    Transform                     m_localTransform;

};
