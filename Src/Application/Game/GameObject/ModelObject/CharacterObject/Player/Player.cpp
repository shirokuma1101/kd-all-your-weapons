#include "Player.h"

#include "Application/Game/GameObject/ModelObject/DynamicObject/DynamicObject.h"

void Player::Update(float delta_time)
{
    auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
    auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();

    if (mm->IsLoadedOnlyOnce(m_name)) {
        m_spModel = mm->CopyData(m_name);
        m_animator.SetAnimation(m_spModel->GetAnimation("t_pose"));
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
    
    MouseOperator(delta_time, -40.f, 60.f); // MN: カメラの可動範囲
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

    // TODO: 装備(stock)するようにする
    // TODO: 持っている際に地面に埋まらないようにする
    // 持てないオブジェクトを障害にして、敵を絶対倒すようにする
    Shot(delta_time);
    
    SetCameraTransform(delta_time);

    PlayAnimation(delta_time);
}

void Player::DrawOpaque()
{
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
    auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();
    
    if (mm->IsLoaded(m_name) && m_spModel) {
        auto rotation_matrix = Math::Matrix::CreateFromYawPitchRoll(
            convert::ToRadians(m_angle + 180.f),
            convert::ToRadians(0.f),
            convert::ToRadians(m_transform.rotation.z)
        );
        auto translation_matrix = Math::Matrix::CreateTranslation(
            m_transform.position.x,
            m_transform.position.y - (*jm)[m_name]["expand"]["status"]["height"].get<float>(),
            m_transform.position.z
        );
        
        DirectX11System::Instance().GetShaderManager()->m_standardShader.DrawModel(
            *m_spModel,
            Math::Matrix::CreateScale(m_transform.scale) * rotation_matrix * translation_matrix
        );
    }
}

void Player::MouseOperator(float delta_time, float narrow_limit, float wide_limit)
{
    auto& cm = Application::Instance().GetGameSystem()->GetInputManager()->GetCursorManager();
    auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();

    auto pos = cm->GetPositionFromCenter();
    cm->LockInCenter();
    
    m_cameraRotaion += Math::Vector3(static_cast<float>(pos.y), static_cast<float>(pos.x), 0) * (*jm)[m_name]["expand"]["status"]["mouse_sensitivity"];
    if (m_cameraRotaion.x <= narrow_limit) {
        m_cameraRotaion.x = narrow_limit;
    }
    if (m_cameraRotaion.x >= wide_limit) {
        m_cameraRotaion.x = wide_limit;
    }

    // 移動時はマウスのY軸方向のみ固定
    if (km->GetState('W') || km->GetState('S') || km->GetState('A') || km->GetState('D')) {
        m_transform.rotation.y = m_cameraRotaion.y;
    }
    // 構え時はマウスの方向固定
    if (km->GetState(VK_RBUTTON)) {
        m_transform.rotation = m_cameraRotaion;
        m_angle              = m_cameraRotaion.y;
        m_moveDirection      = Math::Matrix::CreateRotationY(convert::ToRadians(m_angle)).Backward();
    }
    // 構え無い場合はプレイヤーの移動方向に回転する
    else {
        m_transform.rotation.x = 0.f;
        
        const float angle_speed = 180.f * delta_time; // MN: 回転速度
        // -180 ～ 180
        float angle = convert::ToDegrees(std::atan2(m_moveDirection.x, m_moveDirection.z));
        // 0 ～ 360
        if (angle < 0.f) {
            angle += 360.f;
        }
        if (m_angle < 0.f) {
            m_angle += 360.f;
        }
        if (m_angle > 360.f) {
            m_angle -= 360.f;
        }

        if (m_angle > angle + angle_speed || m_angle < angle - angle_speed) {
            bool is_turn_right = false;
            float difference = angle - m_angle;
            if (std::abs(difference) < 180.f) {
                if (difference > 0.f) {
                    is_turn_right = true;
                }
                else {
                    is_turn_right = false;
                }
            }
            else {
                if (difference > 0.f) {
                    is_turn_right = false;
                }
                else {
                    is_turn_right = true;
                }
            }
            if (is_turn_right) {
                m_angle += angle_speed;
            }
            if (!is_turn_right) {
                m_angle += -angle_speed;
            }
        }
    }
}

void Player::KeyOperator(float delta_time)
{
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
    auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();
    auto local_mat = Math::Matrix::CreateFromYawPitchRoll(convert::ToRadians(m_transform.rotation));
    Math::Vector3 dir;
    bool is_move = false;
    
    if (km->GetState('W')) {
        dir += local_mat.Backward();
        dir.y = 0.f;
        is_move = true;
    }
    if (km->GetState('S')) {
        dir += local_mat.Forward();
        dir.y = 0.f;
        is_move = true;
    }
    if (km->GetState('D')) {
        dir += local_mat.Right();
        is_move = true;
    }
    if (km->GetState('A')) {
        dir += local_mat.Left();
        is_move = true;
    }
    dir.Normalize();
    if (km->GetState(VK_SHIFT) && km->GetState('W') && !km->GetState(VK_RBUTTON)) {
        m_transform.position += dir * (*jm)[m_name]["expand"]["status"]["sprint_speed"] * delta_time;
    }
    else {
        m_transform.position += dir * (*jm)[m_name]["expand"]["status"]["walk_speed"] * delta_time;
    }
    if (is_move) {
        m_moveDirection = dir;
    }
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

    static const float charge_threshold  = 2.f; // MN: キーが被っているので、素早くキーを離した際の閾値
    static const float release_range_max = 4.f; // MN: 装備しているものをプレイヤーからどれだけ離すかの最大値
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

    // 構え
    if (km->GetState(VK_RBUTTON)) {
        // DynamicObjectを持っていない場合
        if (m_wpEquipObject.expired()) {
            std::list<collision::Result> results;
            collision::Result            nearest_result;
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
                    if (!m_isShotCT && !m_isSucceededChargeCT && !m_isFailedChargeCT && km->GetState('F')) {
                        m_wpEquipObject = nearest_obj;
                    }
                }
                // 選択不可能
                else {
                    sp_obj->SetSelection(DynamicObject::Selection::NotEquippable);
                }
            }
        }
        // DynamicObjectを持っている場合
        if (!m_wpEquipObject.expired()) {
            // 溜め
            if (km->GetState(VK_LBUTTON, KeyManager::KeyState::Hold)) {
                // 溜めを増加
                m_nowChargePower += charge_increase_speed * delta_time;
                // 最大値を超えた場合失敗CT有効
                if (m_nowChargePower > charge_max) {
                    m_isFailedChargeCT = true;
                }
                // エフェクトが未だ再生されていない場合
                if (m_wpEffectTransform.expired()) {
                    // 閾値を下回っている場合エフェクトの再生開始
                    if (m_nowChargePower >= charge_threshold) {
                        effekseer_helper::EffectTransform effect_transform;
                        effect_transform.maxFrame = 1200;
                        m_wpEffectTransform = em->Emit("charge", effect_transform, true);
                    }
                }
                else {
                    m_wpEffectTransform.lock()->matrix
                        = Math::Matrix::CreateTranslation(0.f, 0.f, 2.f)
                        * Math::Matrix::CreateScale(0.25f)
                        * Math::Matrix::CreateFromYawPitchRoll(convert::ToRadians(m_transform.rotation))
                        * Math::Matrix::CreateTranslation(m_transform.position);
                }
            }
            // 失敗CT時
            if (m_isFailedChargeCT) {
                m_wpEquipObject.lock()->Force(m_transform.matrix.Backward(), 0.f);
                m_wpEquipObject.reset();
                m_nowChargePower = 0.f;
                if (!m_wpEffectTransform.expired()) {
                    m_wpEffectTransform.lock()->maxFrame = 0;
                }
            }
            else {
                auto sp_obj = m_wpEquipObject.lock();
                sp_obj->SetEquipping(true);
                sp_obj->SetMatrix(Math::Matrix::CreateTranslation(position + normalized_backward * release_range_max));
                sp_obj->Force(m_transform.matrix.Backward(), ((*jm)[m_name]["expand"]["status"]["shot_power"] + m_nowChargePower));
            }
            // 発射
            if (km->GetState(VK_LBUTTON, KeyManager::KeyState::Release)) {
                m_wpEquipObject.reset();
                if (m_nowChargePower < charge_threshold) {
                    m_nowChargePower = 0.f;
                }
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
                if (!m_wpEffectTransform.expired()) {
                    m_wpEffectTransform.lock()->maxFrame = 0;
                }
            }
        }
    }
    // 構え解除
    else {
        if (!m_wpEquipObject.expired()) {
            m_wpEquipObject.lock()->Force(m_transform.matrix.Backward(), 0.f);
            m_wpEquipObject.reset();
            m_nowChargePower = 0.f;
            if (!m_wpEffectTransform.expired()) {
                m_wpEffectTransform.lock()->maxFrame = 0;
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

void Player::SetCameraTransform(float delta_time)
{
    auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();
    
    // カメラがある場合
    if (!m_wpFollowerCamera.expired()) {

        static const float zoom_sec = 0.25f;
        static const Math::Vector3 normal_camera = { 0.6f, -0.1f, -1.1f }; // MN: 通常時のカメラの座標
        static const Math::Vector3 zoom_camera   = { 0.4f, -0.1f, -0.6f }; // MN: ズーム時のカメラの座標
        Math::Vector3 camera_pos = normal_camera;

        // 構え時にズームする
        if (km->GetState(VK_RBUTTON)) {
            m_isZoom = true;
        }
        else {
            m_isZoom = false;
        }
        m_wpFollowerCamera.lock()->SetCameraTransform(
            Transform(m_transform.position, m_cameraRotaion),
            Transform(camera_pos.Lerp(normal_camera, zoom_camera, convert::Normalize(m_zoomTime, 0.f, zoom_sec)))
        );

        if (m_isZoom) {
            m_zoomTime += delta_time;
            if (m_zoomTime > zoom_sec) {
                m_zoomTime = zoom_sec;
            }
        }
        else {
            m_zoomTime -= delta_time;
            if (m_zoomTime < 0.f) {
                m_zoomTime = 0.f;
            }
        }
    }
}

void Player::PlayAnimation(float delta_time)
{
    auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();

    // 構え時 MN: アニメーション名
    if (km->GetState(VK_RBUTTON)) {        
        if (km->GetState('W') && km->GetState('A')) {
            if (m_animator.GetAnimationName() != "rifle_walk_forward_left") {
                m_animator.SetAnimation(m_spModel->GetAnimation("rifle_walk_forward_left"));
            }
        }
        else if (km->GetState('W') && km->GetState('D')) {
            if (m_animator.GetAnimationName() != "rifle_walk_forward_right") {
                m_animator.SetAnimation(m_spModel->GetAnimation("rifle_walk_forward_right"));
            }
        }
        else if (km->GetState('S') && km->GetState('A')) {
            if (m_animator.GetAnimationName() != "rifle_walk_backward_left") {
                m_animator.SetAnimation(m_spModel->GetAnimation("rifle_walk_backward_left"));
            }
        }
        else if (km->GetState('S') && km->GetState('D')) {
            if (m_animator.GetAnimationName() != "rifle_walk_backward_right") {
                m_animator.SetAnimation(m_spModel->GetAnimation("rifle_walk_backward_right"));
            }
        }
        else if (km->GetState('W')) {
            if (m_animator.GetAnimationName() != "rifle_walk_foward") {
                m_animator.SetAnimation(m_spModel->GetAnimation("rifle_walk_foward"));
            }
        }
        else if (km->GetState('S')) {
            if (m_animator.GetAnimationName() != "rifle_walk_backward") {
                m_animator.SetAnimation(m_spModel->GetAnimation("rifle_walk_backward"));
            }
        }
        else if (km->GetState('D')) {
            if (m_animator.GetAnimationName() != "rifle_walk_right") {
                m_animator.SetAnimation(m_spModel->GetAnimation("rifle_walk_right"));
            }
        }
        else if (km->GetState('A')) {
            if (m_animator.GetAnimationName() != "rifle_walk_left") {
                m_animator.SetAnimation(m_spModel->GetAnimation("rifle_walk_left"));
            }
        }
        else {
            if (m_animator.GetAnimationName() != "rifle_standing_aiming_idle") {
                m_animator.SetAnimation(m_spModel->GetAnimation("rifle_standing_aiming_idle"));
            }
        }
    }
    // 移動時
    else if (km->GetState('W') || km->GetState('S') || km->GetState('A') || km->GetState('D')) {
        // スプリント時
        if (km->GetState(VK_SHIFT) && km->GetState('W')) {
            if (km->GetState('W') && km->GetState('A')) {
                if (m_animator.GetAnimationName() != "rifle_sprint_forward_left") {
                    m_animator.SetAnimation(m_spModel->GetAnimation("rifle_sprint_forward_left"));
                }
            }
            else if (km->GetState('W') && km->GetState('D')) {
                if (m_animator.GetAnimationName() != "rifle_sprint_forward_right") {
                    m_animator.SetAnimation(m_spModel->GetAnimation("rifle_sprint_forward_right"));
                }
            }
            else {
                if (m_animator.GetAnimationName() != "rifle_sprint_foward") {
                    m_animator.SetAnimation(m_spModel->GetAnimation("rifle_sprint_foward"));
                }
            }
        }
        else {
            if (m_animator.GetAnimationName() != "walking_with_rifle_down") {
                m_animator.SetAnimation(m_spModel->GetAnimation("walking_with_rifle_down"));
            }
        }
    }
    else {
        if (m_animator.GetAnimationName() != "two_hand_lowered_gun_rifle_idle") {
            m_animator.SetAnimation(m_spModel->GetAnimation("two_hand_lowered_gun_rifle_idle"));
        }
    }
    
    // アニメーションを再生(行列を計算)
    m_animator.AdvanceTime(m_spModel->WorkNodes(), 60.f * delta_time);
    // 腰の角度を計算
    auto spine2 = m_spModel->FindWorkNode("mixamorig:Spine2"); // MN: 上
    auto spine1 = m_spModel->FindWorkNode("mixamorig:Spine1"); // MN: 中
    auto spine = m_spModel->FindWorkNode("mixamorig:Spine");   // MN: 下
    if (spine2 && spine1 && spine) {
        float one_third = convert::ToRadians(-m_transform.rotation.x / 3.f);
        spine2->m_localTransform *= Math::Matrix::CreateRotationX(one_third);
        spine1->m_localTransform *= Math::Matrix::CreateRotationX(one_third);
        spine->m_localTransform *= Math::Matrix::CreateRotationX(one_third);
    }
    // 行列を更新
    m_spModel->CalcNodeMatrices();
}
