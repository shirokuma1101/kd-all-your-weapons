#include "Enemy.h"

void Enemy::Init()
{
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();

    CharacterObject::Init();

    m_attackInterval = 1.f;
    m_attackPower = (*jm)[m_name]["expand"]["status"]["attack_power"];
    m_health = (*jm)[m_name]["expand"]["status"]["health"];

    m_state = State::Idle;
}

void Enemy::Update(float delta_time)
{
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
    auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();

    // モデルのロード時
    if (!m_spModel && mm->IsLoadedOnlyOnce(m_name)) {
        // モデルを取得
        m_spModel = mm->CopyData(m_name);

        // モデルの当たり判定を作成
        CreateModelCollision();
    }
    // モデルのロード
    if (!mm->IsLoaded(m_name)) return;

    // 行列の計算
    if ((*jm)[m_name]["transform"]["fixed"]) {
        // 固定
        m_transform = game_object_helper::ToTransform((*jm)[m_name]["transform"]);
        m_transform.Composition();
        if (m_pRigidActor) {
            m_pRigidActor->setGlobalPose(physx::PxTransform(
                physx_helper::ToPxVec3(m_transform.position),
                physx_helper::ToPxQuat(Math::Quaternion::CreateFromYawPitchRoll(convert::ToRadians(m_transform.rotation)))
            ));
        }
    }
    else {
        // 当たり判定
        if (m_targetFound) {
            m_transform.position.y += -constant::fG * delta_time;
            Collision();
        }
    }

    m_transform.Composition();
    
    if (m_health) {
        // プレイヤーがいる場合
        if (!m_wpTargetPlayer.expired()) {
            constexpr float sighting_range = 10.f; // MN: 視界
            constexpr float attack_range = 2.f;  // MN: 攻撃範囲
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
                m_targetFound = true;
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

                        m_state = State::Attack;
                    }
                }
                // 攻撃範囲外の場合
                else {
                    // 移動する
                    m_transform.position += dir * ((*jm)[m_name]["expand"]["status"]["run_speed"] * delta_time);
                    m_attackInterval = 0.f;

                    m_state = State::Chase;
                }
            }
            else {
                m_state = State::Idle;
            }
        }
        physx_helper::PutToSleep(m_pRigidActor);
        m_transform.Composition();
        m_pRigidActor->setGlobalPose(physx::PxTransform(physx_helper::ToPxMat44(m_transform.matrix)));
    }
    else {
        physx_helper::WakeUp(m_pRigidActor);
        m_transform.matrix = physx_helper::ToMatrix(m_pRigidActor->getGlobalPose());
        m_state = State::Dead;
    }
    
    PlayAnimation(delta_time);
    
    if (m_state == State::Dead && m_animator.IsAnimationEnd()) {
        m_isObjectAlive = false;
        m_wpTargetPlayer.lock()->AddEquipWeightLimit(5.f);
    }
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

    switch (m_state) {
    case Enemy::State::Idle:
        if (m_animator.GetAnimationName() != "zombie_alert_idle") {
            m_animator.SetAnimation(m_spModel->GetAnimation("zombie_alert_idle"));
        }
        break;
    case Enemy::State::Chase:
        if (m_animator.GetAnimationName() != "zombie_running") {
            m_animator.SetAnimation(m_spModel->GetAnimation("zombie_running"));
        }
        break;
    case Enemy::State::Attack:
        if (m_animator.GetAnimationName() != "zombie_attack_with_right_hand") {
            m_animator.SetAnimation(m_spModel->GetAnimation("zombie_attack_with_right_hand"));
        }
        break;
    case Enemy::State::Dead:
        if (m_animator.GetAnimationName() != "zombie_getting_hit_and_failing_onto_back") {
            m_animator.SetAnimation(m_spModel->GetAnimation("zombie_getting_hit_and_failing_onto_back"), false);
        }
        break;
    default:
        break;
    }
    
    // アニメーションを再生(行列を計算)
    m_animator.AdvanceTime(m_spModel->WorkNodes(), 60.f * delta_time);
    // 行列を更新
    m_spModel->CalcNodeMatrices();
}
