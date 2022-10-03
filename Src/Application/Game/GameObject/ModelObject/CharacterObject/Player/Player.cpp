#include "Player.h"

#include "Application/Game/GameObject/ModelObject/DynamicObject/DynamicObject.h"

void Player::Update(float delta_time)
{
    auto& em = Application::Instance().GetGameSystem()->GetEffekseerManager();
    auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
    auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();

    if (mm->IsLoadedOnlyOnce(m_name)) {
        m_spModel = mm->CopyData(m_name);
    }
    if (!mm->IsLoaded(m_name)) return;

    static bool is_push = false;
    if (km->GetState(VK_F3, KeyManager::KeyState::Press)) {
        if (!is_push) is_push = true;
        else is_push = false;
    }
    if (is_push) return;
    
    
    /**************************************************
    * 操作
    **************************************************/

    MouseOperator(-40.f, 90.f - 1.f);
    KeyOperator(delta_time);
    
    
    /**************************************************
    * 当たり判定
    **************************************************/

    bool is_ground = false;
    
    std::list<collision::Result> results;
    auto rays = ToRays((*jm)[m_name]["collision"]["active"]);
    auto spheres = ToSpheres((*jm)[m_name]["collision"]["active"]);

    for (const auto& e : Application::Instance().GetGameSystem()->GetGameObjects()) {
        const auto& collider = e->GetCollider();
        if (!collider) continue;
        // 地面との判定 (光線)
        for (const auto& ray : rays) {
            if (collider->Intersects(DefaultCollisionType::Bump, e->GetTransform().matrix, ray, &results)) {
                if (auto result = collision::GetFarthest(results); result) {
                    m_transform.position += result->direction * result->depth;
                    is_ground = true;
                }
                results.clear();
            }
        }
        // 壁との判定 (球)
        for (const auto& sphere : spheres) {
            if (collider->Intersects(DefaultCollisionType::Bump, e->GetTransform().matrix, sphere, &results)) {
                if (auto result = collision::GetNearest(results); result) {
                    m_transform.position += result->direction * result->depth;
                }
                results.clear();
            }
        }
    }

    // 地面がマイナスになることはない
    if (m_transform.position.y < 0) {
        is_ground = true;
        m_transform.position.y = 0.f;
    }

    if (is_ground && km->GetState(VK_SPACE, KeyManager::KeyState::Press)) {
        is_ground = false;
        m_initialVelocity = (*jm)[m_name]["expand"]["status"]["jump_power"];
    }
    
    if (is_ground) {
        m_initialVelocity = 0.f;
        m_jumpTime        = 0.f;
    }
    else {
        m_transform.position.y += (m_initialVelocity - constant::fG * m_jumpTime) * delta_time;
        m_jumpTime += delta_time;
    }

    m_transform.Composition();
    m_pRigidActor->setGlobalPose(physx::PxTransform(physx_helper::ToPxVec3(m_transform.position)));

    float max                 = (*jm)[m_name]["expand"]["charge_power"]["max"];
    float increase_speed      = (*jm)[m_name]["expand"]["charge_power"]["increase_speed"];
    float succeeded_cool_time = (*jm)[m_name]["expand"]["charge_power"]["succeeded_cool_time"];
    float failed_cool_time    = (*jm)[m_name]["expand"]["charge_power"]["failed_cool_time"];

    // 発射
    if (!m_isSucceededCoolTime && !m_isFailedCoolTime && km->GetState(VK_LBUTTON)) {

        // 溜め
        if (km->GetState(VK_RBUTTON)) {
            // クールタイムがどちらも無い状態
            if (!m_isSucceededCoolTime && !m_isFailedCoolTime) {
                // 溜めを増加
                m_nowChargePower += increase_speed * delta_time;
                // 最大値を超えたら失敗クールタイム有効
                if (m_nowChargePower > max) {
                    m_isFailedCoolTime = true;
                }
            }
            else {
                // cool time now
            }
        }
        else {
            m_nowChargePower = 0.f;
        }

        // TODO: 装備(stock)するようにする
        // DynamicObjectを持つ
        std::weak_ptr<DynamicObject> nearest_obj;
        auto position = m_transform.position;
        auto normalized_backward = m_transform.matrix.Backward();
        if (!m_wpFollowerTarget.expired()) {
            position = m_wpFollowerTarget.lock()->GetMatrix().Translation();
            normalized_backward = m_wpFollowerTarget.lock()->GetMatrix().Backward();
        }
        normalized_backward.Normalize();
        auto ray = collision::Ray(position, normalized_backward, (*jm)[m_name]["expand"]["status"]["reachable_range"]);
        for (const auto& e : Application::Instance().GetGameSystem()->GetDynamicObjects()) {
            auto sp_e = e.lock();
            const auto& collider = sp_e->GetCollider();
            if (!collider) continue;
            // DynamicObjectとの判定 (光線)
            if (collider->Intersects(DefaultCollisionType::Bump, sp_e->GetTransform().matrix, ray, &results)) {
                if (collision::GetNearest(results)) {
                    nearest_obj = e;
                }
            }
        }
        if (collision::GetNearest(results)) {
            auto sp_obj = nearest_obj.lock();
            sp_obj->SetHasInHand(true);
            sp_obj->SetMatrix(Math::Matrix::CreateTranslation(
                position + normalized_backward * 4.f
            ));
            if (m_isFailedCoolTime) {
                sp_obj->Force(m_transform.matrix.Backward(), 0.f);
            }
            else {
                sp_obj->Force(m_transform.matrix.Backward(), ((*jm)[m_name]["expand"]["status"]["shot_power"] + m_nowChargePower));
            }
            // 離した(発射した)瞬間
            if (km->GetState(VK_RBUTTON, KeyManager::KeyState::Release)) {
                m_isSucceededCoolTime = true;
            }
        }
    }

    // 成功クールタイム中
    if (m_isSucceededCoolTime) {
        m_nowChargePower = 0.f;
        m_nowCoolTime += delta_time;
        if (m_nowCoolTime >= succeeded_cool_time) {
            m_isSucceededCoolTime = false;
            m_nowCoolTime = 0.f;
        }
    }
    // 失敗クールタイム中
    if (m_isFailedCoolTime) {
        m_nowChargePower = 0.f;
        m_nowCoolTime += delta_time;
        if (m_nowCoolTime >= failed_cool_time) {
            m_isFailedCoolTime = false;
            m_nowCoolTime = 0.f;
        }
    }
    
    if (km->GetState(VK_CONTROL, KeyManager::KeyState::Press)) {
        effekseer_helper::EffectTransform effect_transform;
        effect_transform.matrix = Math::Matrix::CreateTranslation({ 0.f, 5.f, 0.f });
        effect_transform.maxFrame = 120;
        em->Emit("exp", effect_transform);
    }
}

void Player::DrawOpaque()
{
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
    auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();
    if (mm->IsLoaded(m_name) && m_spModel) {
        DirectX11System::Instance().GetShaderManager()->m_standardShader.DrawModel(
            *m_spModel,
            Math::Matrix::CreateScale(m_transform.scale)
            * Math::Matrix::CreateFromYawPitchRoll(
                convert::ToRadians(m_transform.rotation.y),
                convert::ToRadians(90.f),
                convert::ToRadians(m_transform.rotation.z + 180.f)
            )
            * Math::Matrix::CreateTranslation(
                m_transform.position.x,
                m_transform.position.y - (*jm)[m_name]["expand"]["status"]["height"].get<float>(),
                m_transform.position.z
            )
        );
    }
}

void Player::MouseOperator(float narrow_limit, float wide_limit)
{
    auto& cm = Application::Instance().GetGameSystem()->GetInputManager()->GetCursorManager();
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
    
    auto pos = cm->GetPositionFromCenter();
    cm->LockInCenter();
    Math::Vector3 rot_dir = Math::Vector3(static_cast<float>(pos.y), static_cast<float>(pos.x), 0) * (*jm)[m_name]["expand"]["status"]["mouse_sensitivity"];
    m_transform.rotation += rot_dir;
    if (m_transform.rotation.x <= narrow_limit) {
        m_transform.rotation.x = narrow_limit;
    }
    if (m_transform.rotation.x >= wide_limit) {
        m_transform.rotation.x = wide_limit;
    }
}

void Player::KeyOperator(float delta_time)
{
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
    
    Math::Vector3 pos_dir = GetWasdKey(m_transform.rotation);
    pos_dir *= (*jm)[m_name]["expand"]["status"]["move_speed"] * delta_time;
    m_transform.position += pos_dir;
}
