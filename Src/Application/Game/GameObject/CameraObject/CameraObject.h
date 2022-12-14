#pragma once

#include "System/GameObject/GameObject.h"

class CameraObject : public GameObject
{
public:

    CameraObject(const Math::Matrix& camera_matrix = Math::Matrix::Identity, float fov = 60.f, float aspect = 16.f / 9.f , float near_clipping_distance = 0.001f, float far_clipping_distance = 1000.f)
        : m_spCamera(std::make_shared<CameraProperties>(camera_matrix, fov, aspect, near_clipping_distance, far_clipping_distance))
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

    virtual std::shared_ptr<const CameraProperties> GetCamera() const final {
        return m_spCamera;
    }
    virtual std::shared_ptr<CameraProperties> GetCamera() final {
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
        auto& kcm = Application::Instance().GetGameSystem()->GetKeyConfigManager();
        
        constexpr float head_to_waist_length = 0.7f; // MN: 頭から腰までの長さ
        constexpr float ray_offset           = 0.1f; // MN: 当たり判定の際の当たった場所から内側の距離オフセット
        
        // プレイヤーの行列を計算
        Math::Matrix player_matrix
            = Math::Matrix::CreateFromYawPitchRoll(convert::ToRadians(m_transform.rotation))
            * Math::Matrix::CreateTranslation(m_transform.position);

        // プレイヤー行列とローカル行列を合成してカメラ行列を計算
        Math::Matrix camera_matrix
            = m_localTransform.matrix;

        // 構え時は腰の角度によって移動した頭の行列を計算する
        if (kcm->GetState(KeyType::Aim)) {
            Math::Matrix head_matrix
                = Math::Matrix::CreateTranslation(0.f, head_to_waist_length, 0.f)
                * Math::Matrix::CreateFromYawPitchRoll(convert::ToRadians(m_transform.rotation))
                * Math::Matrix::CreateTranslation(0.f, -head_to_waist_length, 0.f);
            // 頭の行列を考慮したプレイヤー行列を計算する
            camera_matrix
                *= head_matrix
                * Math::Matrix::CreateTranslation(m_transform.position);
            //player_matrix._41 += camera_matrix.Translation().x;
            //player_matrix._43 += camera_matrix.Translation().z;
        }
        else {
            camera_matrix *= player_matrix;
        }


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
            if (collider->Intersects(game_object_helper::DefaultCollisionTypeBump, e->GetTransform().matrix, ray, &results)) {
                if (auto result = collision::GetNearest(results); result) {
                    camera_matrix *= Math::Matrix::CreateTranslation(result->direction * result->depth);
                }
                results.clear();
            }
        }
        
        m_transform.matrix = camera_matrix;
    }

    std::shared_ptr<CameraProperties> m_spCamera;
    Transform                         m_localTransform;

};
