#include "Player.h"

#include "Application/Game/GameObject/ModelObject/DynamicObject/DynamicObject.h"
#include "Application/Game/Scene/GameScene/GameScene.h"

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

    
    /* 操作 */
    
    MouseOperator(delta_time, -70.f, 60.f); // MN: カメラの可動範囲
    KeyOperator(delta_time);
    
    
    /* 当たり判定 */

    // 重力計算
    m_transform.position.y += (m_initialVelocity - constant::fG * m_jumpTime) * delta_time;
    bool is_ground = Collision();
    
    /* 操作 (当たり判定後) */
    if (is_ground && km->GetState(VK_SPACE, KeyManager::KEYSTATE_PRESS)) {
        is_ground = false;
        m_initialVelocity = (*jm)[m_name]["expand"]["status"]["jump_power"];
    }
    if (is_ground) {
        m_initialVelocity = 0.f;
        m_jumpTime        = 0.f;
    }
    else {
        m_jumpTime += delta_time;
    }

    /* 行列計算 */

    m_transform.Composition();
    
    Shot(delta_time);
    
    SetCameraTransform(delta_time);

    PlayAnimation(delta_time);

    // デバッグ
    if (km->GetState(VK_CONTROL, KeyManager::KEYSTATE_PRESS)) {
        m_equipWeightLimit += 5.f;
    }
}

void Player::DrawOpaque()
{
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
    auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();
    
    if (mm->IsLoaded(m_name) && m_spModel) {
        auto rotation_matrix = Math::Matrix::CreateFromYawPitchRoll(
            convert::ToRadians(m_angle + 180.f),
            convert::ToRadians(0.f),
            convert::ToRadians(0.f)
        );
        auto translation_matrix = Math::Matrix::CreateTranslation(
            m_transform.position.x,
            m_transform.position.y - (*jm)[m_name]["expand"]["status"]["height"].get<float>(),
            m_transform.position.z
        );

        DirectX11System::WorkInstance().GetShaderManager()->GetStandardShader().DrawModel(
            *m_spModel,
            Math::Matrix::CreateScale(m_transform.scale) * rotation_matrix * translation_matrix
        );
    }
}

void Player::MouseOperator(float delta_time, float narrow_limit, float wide_limit)
{
    auto& cm = Application::Instance().GetGameSystem()->GetInputManager()->GetCursorManager();
    auto& kcm = Application::Instance().GetGameSystem()->GetKeyConfigManager();
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();

    auto pos = cm->GetPositionFromCenter(input_helper::CursorData::Position(50, 25)); // MN: 移動量リミット
    cm->LockInCenter();
    
    m_cameraRotaion += Math::Vector3(static_cast<float>(pos.y), static_cast<float>(pos.x), 0) * (*jm)[m_name]["expand"]["status"]["mouse_sensitivity"];
    if (m_cameraRotaion.x <= narrow_limit) {
        m_cameraRotaion.x = narrow_limit;
    }
    if (m_cameraRotaion.x >= wide_limit) {
        m_cameraRotaion.x = wide_limit;
    }

    // 移動時はマウスのY軸方向のみ固定
    if (kcm->GetState(KeyType::MoveForward)
        || kcm->GetState(KeyType::MoveBackward)
        || kcm->GetState(KeyType::StrafeLeft)
        || kcm->GetState(KeyType::StrafeRight)) {
        m_transform.rotation.y = m_cameraRotaion.y;
    }
    // 構え時はマウスの方向固定
    if (kcm->GetState(KeyType::Aim)) {
        m_transform.rotation = m_cameraRotaion;
        m_angle              = m_cameraRotaion.y;
        m_moveDirection      = Math::Matrix::CreateRotationY(convert::ToRadians(m_angle)).Backward();
    }
    // 構え無い場合はプレイヤーの移動方向に回転する
    else {
        m_transform.rotation.x = 0.f;
        
        const float angle_speed = 270.f * delta_time; // MN: 回転速度
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
    auto& kcm = Application::Instance().GetGameSystem()->GetKeyConfigManager();
    auto local_mat = Math::Matrix::CreateFromYawPitchRoll(convert::ToRadians(m_transform.rotation));
    Math::Vector3 dir;
    bool is_move = false;
    
    if (kcm->GetState(KeyType::MoveForward)) {
        dir += local_mat.Backward();
        dir.y = 0.f;
        is_move = true;
    }
    if (kcm->GetState(KeyType::MoveBackward)) {
        dir += local_mat.Forward();
        dir.y = 0.f;
        is_move = true;
    }
    if (kcm->GetState(KeyType::StrafeRight)) {
        dir += local_mat.Right();
        is_move = true;
    }
    if (kcm->GetState(KeyType::StrafeLeft)) {
        dir += local_mat.Left();
        is_move = true;
    }
    dir.Normalize();
    if (kcm->GetState(KeyType::Sprint) && kcm->GetState(KeyType::MoveForward)
        && !kcm->GetState(KeyType::Aim)) {
        m_transform.position += dir * (*jm)[m_name]["expand"]["status"]["sprint_speed"] * delta_time;
    }
    else {
        m_transform.position += dir * (*jm)[m_name]["expand"]["status"]["walk_speed"] * delta_time;
    }
    if (is_move) {
        m_moveDirection = dir;
    }
}

void Player::Shot(float delta_time)
{
    auto& am = Application::Instance().GetGameSystem()->GetAudioManager();
    auto& em = Application::Instance().GetGameSystem()->GetEffekseerManager();
    auto& kcm = Application::Instance().GetGameSystem()->GetKeyConfigManager();
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();

    constexpr float charge_threshold  = 2.f; // MN: キーが被っているので、素早くキーを離した際の閾値
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

    // SceneがGameの場合はGameSceneにキャストする
    if (Application::Instance().GetGameSystem()->GetScene()->GetSceneType() != Scene::SceneType::Game) return;
    // GameSceneと確定しているのでstatic
    auto game_scene = std::static_pointer_cast<GameScene>(Application::WorkInstance().GetGameSystem()->GetScene());

    // 構え
    if (kcm->GetState(KeyType::Aim)) {
        // DynamicObjectを持っていない場合
        if (m_wpEquipObject.expired()) {
            std::list<collision::Result> results;
            collision::Result            nearest_result;
            std::weak_ptr<DynamicObject> nearest_obj;
            // 光線を作成
            auto ray = collision::Ray(position, normalized_backward, (*jm)[m_name]["expand"]["status"]["reachable_range"]);
            for (const auto& e : game_scene->GetDynamicObjects()) {
                const auto  sp_e     = e.lock();
                const auto& collider = sp_e->GetCollider();
                if (!collider) continue;
                // DynamicObjectとの判定 (光線)
                if (collider->Intersects(game_object_helper::DefaultCollisionTypeDynamic, sp_e->GetTransform().matrix, ray, &results)) {
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
                // 選択可能 (重量制限未満)
                if (!m_isShotCT && !m_isSucceededChargeCT && !m_isFailedChargeCT && sp_obj->GetWeight() <= m_equipWeightLimit) {
                    sp_obj->SetSelection(DynamicObject::Selection::Equippable);
                    // DynamicObjectを所持する
                    if (kcm->GetState(KeyType::Interact)) {
                        m_wpEquipObject = nearest_obj;
                    }
                }
                // 選択不可能
                else {
                    sp_obj->SetSelection(DynamicObject::Selection::NotEquippable);
                }
            }
            // 当たらなかった場合と道路の場合、dirtを生成する
            else if (!m_isShotCT && !m_isSucceededChargeCT && !m_isFailedChargeCT && kcm->GetState(KeyType::Interact)) {
                for (const auto& e : Application::Instance().GetGameSystem()->GetScene()->GetGameObjects()) {
                    const auto& collider = e->GetCollider();
                    if (!collider) continue;
                    // Object(Road)との判定 (光線)
                    collider->Intersects(game_object_helper::DefaultCollisionTypeRoad, e->GetTransform().matrix, ray, &results);
                }
                if (results.size()) {
                    auto sp_obj = std::make_shared<DynamicObject>("props_dirt");
                    game_scene->AddDynamicObject(sp_obj);
                    m_wpEquipObject = sp_obj;
                }
            }
        }
        // DynamicObjectを持っている場合
        if (!m_wpEquipObject.expired()) {
            // 溜め
            if (kcm->GetState(KeyType::Shoot, KeyManager::KEYSTATE_HOLD)) {
                // 溜めを増加
                m_nowChargePower += charge_increase_speed * delta_time;
                // 最大値を超えた場合失敗CT有効
                if (m_nowChargePower > charge_max) {
                    m_isFailedChargeCT = true;
                }
                // SEが未だ再生されていない場合
                if (m_wpChargeSE.expired()) {
                    // SEを再生
                    m_wpChargeSE = am->Play("charge_se");
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
                    // エフェクトの行列を設定
                    m_wpEffectTransform.lock()->matrix
                        = Math::Matrix::CreateTranslation(0.5f, -1.f, 2.f)
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
                if (!m_wpChargeSE.expired()) {
                    m_wpChargeSE.lock()->Stop();
                }
                if (!m_wpEffectTransform.expired()) {
                    m_wpEffectTransform.lock()->maxFrame = 0;
                }
            }
            else {
                std::list<collision::Result> results;
                auto dynamic_pos = position + normalized_backward * (*jm)[m_name]["expand"]["status"]["actual_reachable_range"];
                // 光線を作成
                auto ray = collision::Ray(position, normalized_backward, (*jm)[m_name]["expand"]["status"]["actual_reachable_range"]);
                for (const auto& e : Application::Instance().GetGameSystem()->GetScene()->GetGameObjects()) {
                    auto sp_e = m_wpEquipObject.lock();
                    if (e == sp_e) continue;
                    const auto& collider = e->GetCollider();
                    if (!collider) continue;
                    // Objectとの判定 (光線)
                    if (collider->Intersects(game_object_helper::DefaultCollisionTypeBump, e->GetTransform().matrix, ray, &results)) {
                        if (auto result = collision::GetNearest(results); result) {
                            dynamic_pos = result->position;
                        }
                    }
                    // Objectとの判定 (球)
                    if (auto model = sp_e->GetModel(); model) {
                        for (const auto& i : model->GetData()->GetMeshNodeIndices()) {
                            auto bs = DirectX::BoundingSphere(dynamic_pos, model->GetMesh(i)->GetBoundingSphere().Radius);
                            if (collider->Intersects(game_object_helper::DefaultCollisionTypeBump, e->GetTransform().matrix, bs, &results)) {
                                if (auto result = collision::GetNearest(results); result) {
                                    dynamic_pos += result->direction * result->depth;
                                }
                            }
                        }
                    }
                }
                
                auto sp_obj = m_wpEquipObject.lock();
                sp_obj->SetEquipping(true);
                sp_obj->SetMatrix(Math::Matrix::CreateTranslation(dynamic_pos));
                sp_obj->Force(m_transform.matrix.Backward(), ((*jm)[m_name]["expand"]["status"]["shot_power"] + m_nowChargePower));
            }
            // 発射
            if (kcm->GetState(KeyType::Shoot, KeyManager::KEYSTATE_RELEASE)) {
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
                if (!m_wpChargeSE.expired()) {
                    m_wpChargeSE.lock()->Stop();
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
            if (!m_wpChargeSE.expired()) {
                m_wpChargeSE.lock()->Stop();
            }
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
    auto& kcm = Application::Instance().GetGameSystem()->GetKeyConfigManager();

    // カメラがある場合
    if (!m_wpFollowerCamera.expired()) {

        constexpr float zoom_sec = 0.25f; // MN: ズーム秒
        constexpr Math::Vector3 normal_camera = { 0.6f, -0.1f, -1.1f }; // MN: 通常時のカメラの座標
        constexpr Math::Vector3 zoom_camera   = { 0.4f, -0.1f, -0.6f }; // MN: ズーム時のカメラの座標
        Math::Vector3 camera_pos = normal_camera;

        // 構え時にズームする
        if (kcm->GetState(KeyType::Aim)) {
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
    auto& kcm = Application::Instance().GetGameSystem()->GetKeyConfigManager();

    // 構え時 MN: アニメーション名
    if (kcm->GetState(KeyType::Aim)) {
        if (kcm->GetState(KeyType::MoveForward) && kcm->GetState(KeyType::StrafeLeft)) {
            if (m_animator.GetAnimationName() != "rifle_walk_forward_left") {
                m_animator.SetAnimation(m_spModel->GetAnimation("rifle_walk_forward_left"));
            }
        }
        else if (kcm->GetState(KeyType::MoveForward) && kcm->GetState(KeyType::StrafeRight)) {
            if (m_animator.GetAnimationName() != "rifle_walk_forward_right") {
                m_animator.SetAnimation(m_spModel->GetAnimation("rifle_walk_forward_right"));
            }
        }
        else if (kcm->GetState(KeyType::MoveBackward) && kcm->GetState(KeyType::StrafeLeft)) {
            if (m_animator.GetAnimationName() != "rifle_walk_backward_left") {
                m_animator.SetAnimation(m_spModel->GetAnimation("rifle_walk_backward_left"));
            }
        }
        else if (kcm->GetState(KeyType::MoveBackward) && kcm->GetState(KeyType::StrafeRight)) {
            if (m_animator.GetAnimationName() != "rifle_walk_backward_right") {
                m_animator.SetAnimation(m_spModel->GetAnimation("rifle_walk_backward_right"));
            }
        }
        else if (kcm->GetState(KeyType::MoveForward)) {
            if (m_animator.GetAnimationName() != "rifle_walk_foward") {
                m_animator.SetAnimation(m_spModel->GetAnimation("rifle_walk_foward"));
            }
        }
        else if (kcm->GetState(KeyType::MoveBackward)) {
            if (m_animator.GetAnimationName() != "rifle_walk_backward") {
                m_animator.SetAnimation(m_spModel->GetAnimation("rifle_walk_backward"));
            }
        }
        else if (kcm->GetState(KeyType::StrafeRight)) {
            if (m_animator.GetAnimationName() != "rifle_walk_right") {
                m_animator.SetAnimation(m_spModel->GetAnimation("rifle_walk_right"));
            }
        }
        else if (kcm->GetState(KeyType::StrafeLeft)) {
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
    else if (kcm->GetState(KeyType::MoveForward) || kcm->GetState(KeyType::MoveBackward) || kcm->GetState(KeyType::StrafeLeft) || kcm->GetState(KeyType::StrafeRight)) {
        // スプリント時
        if (kcm->GetState(KeyType::Sprint) && kcm->GetState(KeyType::MoveForward)) {
            if (m_animator.GetAnimationName() != "rifle_sprint_foward") {
                m_animator.SetAnimation(m_spModel->GetAnimation("rifle_sprint_foward"));
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
    const float waist_angle = 45.f; // MN: 構え時の腰の角度
    auto spine2 = m_spModel->FindWorkNode("mixamorig:Spine2"); // MN: 上
    auto spine1 = m_spModel->FindWorkNode("mixamorig:Spine1"); // MN: 中
    auto spine  = m_spModel->FindWorkNode("mixamorig:Spine");  // MN: 下
    if (spine2 && spine1 && spine) {
        // m_transform.rotation.xが0の時は0
        float weist = convert::Normalize(m_transform.rotation.x, -90.f, 90.f, -waist_angle, waist_angle);
        float one_third = convert::ToRadians(-m_transform.rotation.x / 3.f);
        float one_third1 = convert::ToRadians(-weist / 3.f);
        spine2->m_localTransform *= Math::Matrix::CreateRotationX(one_third) * Math::Matrix::CreateRotationZ(one_third1);
        spine1->m_localTransform *= Math::Matrix::CreateRotationX(one_third) * Math::Matrix::CreateRotationZ(one_third1);
        spine->m_localTransform  *= Math::Matrix::CreateRotationX(one_third) * Math::Matrix::CreateRotationZ(one_third1);
    }
    // 行列を更新
    m_spModel->CalcNodeMatrices();
}
