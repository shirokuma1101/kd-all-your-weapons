#include "Enemy.h"

void Enemy::Update(float delta_time)
{
    CharacterObject::Update(delta_time);

    m_transform.Composition();
    
    // プレイヤーがいる場合
    if (!m_wpTargetPlayer.expired()) {
        constexpr float sighting_range  = 10.f; // MN: 視界
        constexpr float attack_range    = 2.f;  // MN: 攻撃範囲
        constexpr float walk_speed      = 1.f;  // MN: 歩く速度
        constexpr float attack_interval = 1.f;  // MN: 攻撃間隔
        
        auto sp_obj = m_wpTargetPlayer.lock();
        // 自分からプレイヤーへの方向ベクトルを計算
        auto dir = sp_obj->GetTransform().position - m_transform.position;
        // プレイヤーとの距離を計算
        auto distance = dir.Length();
        // 正規化
        dir.Normalize();
        // プレイヤーとの距離が視認範囲の場合
        if (distance < sighting_range) {
            // プレイヤーの方向を向く
            m_transform.rotation = { 0.f, convert::ToDegrees(std::atan2(dir.x, dir.z)), 0.f };
            // 攻撃範囲内の場合
            if (attack_range > distance) {
                // 攻撃間隔を計算
                m_attackInterval += delta_time;
                // 攻撃間隔が経過している場合
                if (m_attackInterval > attack_interval) {
                    // 攻撃間隔をリセット
                    m_attackInterval = 0.f;
                    // プレイヤーにダメージを与える
                    sp_obj->AddDamage(m_attackPower);
                }
            }
            // 攻撃範囲外の場合
            else {
                // 移動する
                m_transform.position += dir * (walk_speed * delta_time);
                m_attackInterval = 0.f;
            }
        }
    }
    //
    //// if hp = 0 dynamic process
    //m_transform.Composition();
    //m_pRigidActor->setGlobalPose(physx::PxTransform(physx_helper::ToPxMat44(
    //    Math::Matrix::CreateRotationZ(convert::ToRadians(90.f)) * m_transform.matrix
    //)));

}

void Enemy::DrawOpaque()
{
    auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();

    if (mm->IsLoaded(m_name) && m_spModel) {
        auto rotation_matrix = Math::Matrix::CreateRotationY(convert::ToRadians(180.f));
        auto translation_matrix = Math::Matrix::CreateTranslation(
            m_transform.position.x,
            m_transform.position.y - 1.86f,
            m_transform.position.z
        );
        DirectX11System::WorkInstance().GetShaderManager()->GetStandardShader().DrawModel(
            *m_spModel,
            Math::Matrix::CreateScale(100.f)
            * rotation_matrix
            * translation_matrix,
            {"COL"}
        );

    }
}
