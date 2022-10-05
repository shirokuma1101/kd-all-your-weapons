#include "Player.h"

#include "Application/Game/GameObject/ModelObject/DynamicObject/DynamicObject.h"

void Player::Update(float delta_time)
{
    auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
    auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();

    if (mm->IsLoadedOnlyOnce(m_name)) {
        m_spModel = mm->CopyData(m_name);
    }
    if (!mm->IsLoaded(m_name)) return;

    
    /**************************************************
    * 操作
    **************************************************/

    if (km->GetState(VK_ESCAPE, KeyManager::KeyState::Press)) {
        if (!m_isPause) {
            m_isPause = true;
        }
        else {
            m_isPause = false;
        }
    }
    if (m_isPause) return;
    MouseOperator(-40.f, 90.f - 1.f);
    KeyOperator(delta_time);
    
    
    /**************************************************
    * 当たり判定
    **************************************************/

    bool is_ground = Collision();
    
    
    /**************************************************
    * 操作 (当たり判定後)
    **************************************************/

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

    // TODO: 装備(stock)するようにする
    Shot(delta_time);
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
    
    auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();
    auto local_mat = Math::Matrix::CreateFromYawPitchRoll(convert::ToRadians(m_transform.rotation));
    Math::Vector3 pos_dir;
    if (km->GetState('W')) {
        pos_dir += local_mat.Backward();
        pos_dir.y = 0.f;
    }
    if (km->GetState('S')) {
        pos_dir += local_mat.Forward();
        pos_dir.y = 0.f;
    }
    if (km->GetState('D')) {
        pos_dir += local_mat.Right();
    }
    if (km->GetState('A')) {
        pos_dir += local_mat.Left();
    }
    pos_dir.Normalize();
    pos_dir *= (*jm)[m_name]["expand"]["status"]["move_speed"] * delta_time;
    m_transform.position += pos_dir;
}

bool Player::Collision()
{
    bool is_ground = false;
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();

    std::list<collision::Result> results;
    auto rays = ToRays((*jm)[m_name]["collision"]["active"]);
    auto spheres = ToSpheres((*jm)[m_name]["collision"]["active"]);

    for (const auto& e : Application::Instance().GetGameSystem()->GetGameObjects()) {
        const auto& collider = e->GetCollider();
        if (!collider) continue;
        // 地面との判定 (光線)
        for (const auto& ray : rays) {
            if (collider->Intersects(DefaultCollisionType::Bump, e->GetTransform().matrix, ray, &results)) {
                if (auto result = collision::GetNearest(results); result) {
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

    return is_ground;
}

void Player::Shot(float delta_time)
{
    auto& em = Application::Instance().GetGameSystem()->GetEffekseerManager();
    auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();

    static const float release_range = 4.f; // MN: 指定された場所からどれだけ離すか
    float shot_ct               = (*jm)[m_name]["expand"]["shot"]["shot_ct"];
    float charge_max            = (*jm)[m_name]["expand"]["shot"]["charge_max"];
    float charge_increase_speed = (*jm)[m_name]["expand"]["shot"]["charge_increase_speed"];
    float succeeded_charge_ct   = (*jm)[m_name]["expand"]["shot"]["succeeded_charge_ct"];
    float failed_charge_ct      = (*jm)[m_name]["expand"]["shot"]["failed_charge_ct"];

    // プレイヤーを基準とする
    auto position = m_transform.position;
    auto normalized_backward = m_transform.matrix.Backward();
    normalized_backward.Normalize();
    // カメラがある場合
    if (!m_wpFollowerCamera.expired()) {
        // カメラを基準とする
        position = m_wpFollowerCamera.lock()->GetMatrix().Translation();
        normalized_backward = m_wpFollowerCamera.lock()->GetMatrix().Backward();
    }
    normalized_backward.Normalize();

    // DynamicObjectを持っていない場合
    if (m_wpEquipObject.expired()) {
        std::list<collision::Result> results;
        collision::Result nearest_result;
        std::weak_ptr<DynamicObject> nearest_obj;

        // 光線を作成
        auto ray = collision::Ray(position, normalized_backward, (*jm)[m_name]["expand"]["status"]["reachable_range"]);
        for (const auto& e : Application::Instance().GetGameSystem()->GetDynamicObjects()) {
            const auto sp_e = e.lock();
            const auto& collider = sp_e->GetCollider();
            if (!collider) continue;
            // DynamicObjectとの判定 (光線)
            if (collider->Intersects(DefaultCollisionType::Bump, sp_e->GetTransform().matrix, ray, &results)) {
                if (auto result = collision::GetNearest(results); result) {
                    if (!nearest_result.depth || result->depth > nearest_result.depth) {
                        nearest_result = *result;
                        nearest_obj = e;
                    }
                }
            }
        }

        // DynamicObjectが当たった場合
        if (collision::GetNearest(results)) {
            auto sp_obj = nearest_obj.lock();
            // 選択可能
            if (sp_obj->GetWeight() <= m_equipWeightLimit) {
                sp_obj->SetSelection(DynamicObject::Selection::Equippable);
                // 重量制限未満、CT時以外にDynamicObjectを所持する
                if (!m_isShotCT && !m_isSucceededChargeCT && !m_isFailedChargeCT && km->GetState(VK_LBUTTON)) {
                    sp_obj->SetEquipping(true);
                    sp_obj->SetMatrix(Math::Matrix::CreateTranslation(position + normalized_backward * release_range));
                    m_wpEquipObject = nearest_obj;
                }
            }
            // 選択不可能
            else {
                sp_obj->SetSelection(DynamicObject::Selection::NotEquippable);
            }
        }
        m_nowChargePower = 0.f;
        if (!m_wpEffectTransform.expired()) {
            m_wpEffectTransform.lock()->maxFrame = 0;
        }
    }
    // DynamicObjectを持っている場合
    else {
        // 溜め
        if (km->GetState(VK_RBUTTON)) {
            // 溜めを増加
            m_nowChargePower += charge_increase_speed * delta_time;
            // 最大値を超えたら失敗CT有効
            if (m_nowChargePower > charge_max) {
                m_wpEquipObject.lock()->Force(m_transform.matrix.Backward(), 0.f);
                m_wpEquipObject.reset();
                m_isFailedChargeCT = true;
            }
            if (m_wpEffectTransform.expired()) {
                effekseer_helper::EffectTransform effect_transform;
                effect_transform.maxFrame = 1200;
                m_wpEffectTransform = em->Emit("charge", effect_transform);
            }
            else {
                m_wpEffectTransform.lock()->matrix
                    = Math::Matrix::CreateTranslation(0.f, 0.f, 2.f)
                    * Math::Matrix::CreateScale(0.25f)
                    * Math::Matrix::CreateFromYawPitchRoll(convert::ToRadians(m_transform.rotation))
                    * Math::Matrix::CreateTranslation(m_transform.position);
            }
        }
        else {
            m_nowChargePower = 0.f;
            if (!m_wpEffectTransform.expired()) {
                m_wpEffectTransform.lock()->maxFrame = 0;
            }
        }
        if (!m_isFailedChargeCT) {
            auto sp_obj = m_wpEquipObject.lock();
            sp_obj->SetEquipping(true);
            sp_obj->SetMatrix(Math::Matrix::CreateTranslation(position + normalized_backward * release_range));
            sp_obj->Force(m_transform.matrix.Backward(), ((*jm)[m_name]["expand"]["status"]["shot_power"] + m_nowChargePower));
        }
        // 発射
        if (km->GetState(VK_LBUTTON, KeyManager::KeyState::Release)) {
            m_wpEquipObject.reset();
            // 溜めが無い(通常)の場合
            if (!m_nowChargePower) {
                // 発射CT有効
                m_isShotCT = true;
            }
            // 溜めがある場合
            else {
                // 成功CT有効
                m_isSucceededChargeCT = true;
            }
        }
    }

    // CT
    if (m_isShotCT) {
        m_nowChargePower = 0.f;
        m_nowCT += delta_time;
        if (m_nowCT >= shot_ct) {
            m_isShotCT = false;
            m_nowCT = 0.f;
        }
    }
    // 成功CT中
    if (m_isSucceededChargeCT) {
        m_nowChargePower = 0.f;
        m_nowCT += delta_time;
        if (m_nowCT >= succeeded_charge_ct) {
            m_isSucceededChargeCT = false;
            m_nowCT = 0.f;
        }
    }
    // 失敗CT中
    if (m_isFailedChargeCT) {
        m_nowChargePower = 0.f;
        m_nowCT += delta_time;
        if (m_nowCT >= failed_charge_ct) {
            m_isFailedChargeCT = false;
            m_nowCT = 0.f;
        }
    }
}
