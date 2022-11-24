#include "Enemy.h"

void Enemy::Init()
{
    CharacterObject::Init();

    m_attackInterval = 1.f;
    m_attackPower    = 1.f;

    m_health = 100.f;
}

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
    if (m_health) {
        m_pRigidActor->setGlobalPose(physx::PxTransform(physx_helper::ToPxMat44(m_transform.matrix)));
        m_transform.Composition();
    }
    else {
        m_transform.matrix = physx_helper::ToMatrix(m_pRigidActor->getGlobalPose());
    }

    PlayAnimation(delta_time);
}

void Enemy::DrawOpaque()
{
    auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();

    if (mm->IsLoaded(m_name) && m_spModel) {
        float height = (*jm)[m_name]["expand"]["status"]["height"].get<float>();
        Math::Matrix mat;
        if (m_health) {
            mat
                = Math::Matrix::CreateRotationY(convert::ToRadians(180.f))
                * Math::Matrix::CreateTranslation(0.f, -height, 0.f)
                * m_transform.matrix;
        }
        else {
            mat
                = Math::Matrix::CreateTranslation(0.f, -convert::ToHalf(height), 0.f)
                * m_transform.matrix;
        }

        DirectX11System::WorkInstance().GetShaderManager()->GetStandardShader().DrawModel(
            *m_spModel,
            Math::Matrix::CreateScale(100.f) * mat,
            { "COL" }
        );
    }
}

void Enemy::PlayAnimation(float delta_time)
{
    if (!m_spModel) return;

    if (m_animator.GetAnimationName() != "zombie_alert_idle") {
        m_animator.SetAnimation(m_spModel->GetAnimation("zombie_alert_idle"));
    }
    //if (m_animator.GetAnimationName() != "zombie_running") {
    //    m_animator.SetAnimation(m_spModel->GetAnimation("zombie_running"));
    //}
    //if (m_animator.GetAnimationName() != "zombie_attack_with_right_hand") {
    //    m_animator.SetAnimation(m_spModel->GetAnimation("zombie_attack_with_right_hand"));
    //}

    // アニメーションを再生(行列を計算)
    m_animator.AdvanceTime(m_spModel->WorkNodes(), 60.f * delta_time);
    // 行列を更新
    m_spModel->CalcNodeMatrices();
}
