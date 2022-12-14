#include "GameScene.h"

#include "Application/Game/GameObject/CameraObject/CameraObject.h"
#include "Application/Game/GameObject/ModelObject/ModelObject.h"
#include "Application/Game/GameObject/ModelObject/CharacterObject/Player/Player.h"
#include "Application/Game/GameObject/ModelObject/CharacterObject/Enemy/Enemy.h"
#include "Application/Game/GameObject/ModelObject/DynamicObject/DynamicObject.h"

#include "Application/Game\GameObject/UIObject/GameUI/GameUI.h"

#include "Math/Random.h"

void GameScene::Init()
{
    input_helper::CursorData::ShowCursor(false);

    /* Shader */

    auto camera_cb = DirectX11System::WorkInstance().GetShaderManager()->GetCameraCB().Get();
    camera_cb->distanceFogEnable = true;
    camera_cb->distanceFogColor = { 0.75f, 0.64f, 0.6f };
    camera_cb->distanceFogStart = -19.3f;
    camera_cb->distanceFogEnd = 143.6f;
    DirectX11System::WorkInstance().GetShaderManager()->GetCameraCB().Write();

    auto light_cb = DirectX11System::WorkInstance().GetShaderManager()->GetLightCB().Get();
    light_cb->ambientLight = { 0.7f, 0.6f, 0.6f };
    light_cb->directionalLightDirection = { -0.5, -0.8f, 0.3f };
    light_cb->directionalLightDirection.Normalize();
    light_cb->directionalLightColor = { 0.7f, 0.7f, 0.7f };
    DirectX11System::WorkInstance().GetShaderManager()->GetLightCB().Write();

    DirectX11System::WorkInstance().GetShaderManager()->ClearPointLight();
    DirectX11System::WorkInstance().GetShaderManager()->AddPointLight({ 0, 1, 1 }, { 2.0f, 1.0f, 0.1f }, { 0.f, 1.f, 0.f });
    
    /* GameObject */

    auto camera = std::make_shared<CameraObject>(Math::Matrix::Identity, 60.f, 16.f / 9.f, 0.001f, 500.f);
    AddGameObject(camera);
    
    auto player = std::make_shared<Player>("player");
    player->SetFollowerTarget(camera);
    AddGameObject(player);

    for (const auto& name : {
        "enemy_01",
        "enemy_02",
        "enemy_03",
        "enemy_04",
        "enemy_05",
        "enemy_06",
        "enemy_07",
        "enemy_08",
        "enemy_09",
        "enemy_10",
        "enemy_11",
        "enemy_12",
        }) {
        auto enemy = std::make_shared<Enemy>(name);
        enemy->SetTarget(player);
        AddEnemyObject(enemy);
    }
    for (const auto& name : {
        "environment_sphere",
        "camp_fire",

        "broken_building",
        "broken_buildings_01",
        "broken_buildings_02",
        "broken_buildings_03",
        "broken_buildings_04",
        "broken_buildings_05",
        "broken_buildings_06",
        "building_construction",
        "favela",
        "hospital",
        "old_rusty_car",
        "ruin_building",
        "security_hut",
        "wasteland_stores_01",
        "wasteland_stores_02",

        "btr_80a",
        "pzkpfw_vi_tiger_1",
        "tent_01",
        "tent_02",
        "apartment_01",
        "apartment_02",
        "apartment_03",
        "apartment_04",
        "apartment_corner_1",
        "apartment_corner_2_01",
        "apartment_corner_2_02",
        "buildings_front_01",
        "buildings_front_02",
        "buildings_front_03",
        "buildings_front_04",
        "buildings_pack_01",
        "buildings_pack_02",
        "buildings_pack_03",
        "buildings_pack_04",
        "buildings_pack_05",
        "buildings_pack_06",
        "buildings_pack_07",
        "tower_building",
        
        "sidewalk_01",
        "sidewalk_02",
        "sidewalk_03",
        "sidewalk_04",
        "sidewalk_05",
        "sidewalk_06",
        "sidewalk_07",
        "sidewalk_08",
        "sidewalk_09",
        "sidewalk_10",
        "sidewalk_11",
        "sidewalk_12",
        "road_straight_01",
        "road_straight_02",
        "road_straight_03",
        "road_straight_04",
        "road_straight_05",
        "road_straight_06",
        "road_straight_07",
        "road_straight_08",
        "road_straight_09",
        "road_straight_10",
        "road_t_01",
        "road_t_02",
        }) {
        AddGameObject(std::make_shared<ModelObject>(name));
    }
    for (const auto& name : {
        "rebel_army_wooden_barricade_01",
        "rebel_army_wooden_barricade_02",
        "rebel_army_wooden_barricade_03",
        "rebel_army_wooden_barricade_04",
        "rebel_army_wooden_barricade_05",
        "rebel_army_wooden_barricade_06",

        "abandoned_oil_drum",
        "abandoned_urban_garbage_01",
        "abandoned_urban_garbage_02",
        "abandoned_urban_garbage_03",
        "abandoned_urban_garbage_04",
        "abandoned_urban_garbage_05",
        "abandoned_urban_garbage_06",
        "abandoned_urban_garbage_07",
        }) {
        AddDynamicObject(std::make_shared<DynamicObject>(name));
    }
    
    auto game_ui = std::make_shared<GameUI>();
    game_ui->SetPlayer(player);
    AddGameObject(game_ui);

    m_nextSceneType = SceneType::Result;
    m_wpDeletionDecisionObject = game_ui;

    auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();
    mm->AsyncLoad("dirt");
}

Scene::SceneType GameScene::Update(float delta_time)
{
    const auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();

    DirectX11System::WorkInstance().GetShaderManager()->GetLightCB().Get()->pointLight[0].attenuation = { 0.f, (float)random::RandomDistribution(0.75f, 1.f), 0.f };
    DirectX11System::WorkInstance().GetShaderManager()->GetLightCB().Write();

    if (km->GetState(VK_ESCAPE, KeyManager::KEYSTATE_PRESS)) {
        input_helper::CursorData::ShowCursor(true);
        if (MessageBoxA(Application::Instance().GetWindow().GetWindowHandle(), "終了しますか?", "終了確認", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) {
            SendMessage(Application::Instance().GetWindow().GetWindowHandle(), WM_CLOSE, 0, 0);
        }
        else {
            input_helper::CursorData::ShowCursor(false);
        }
    }
    
    static bool is_pause = false;
    if (km->GetState(VK_F1, KeyManager::KEYSTATE_PRESS)) {
        if (!is_pause) {
            is_pause = true;
        }
        else {
            is_pause = false;
        }
    }
    if (is_pause) return m_sceneType;

    
    SceneType scene = Scene::Update(delta_time);
    
    for (auto iter = m_wpEnemyObjects.begin(); iter != m_wpEnemyObjects.end();) {
        if (iter->expired()) {
            iter = m_wpEnemyObjects.erase(iter);
        }
        else {
            ++iter;
        }
    }
    for (auto iter = m_wpDynamicObjects.begin(); iter != m_wpDynamicObjects.end();) {
        if (iter->expired()) {
            iter = m_wpDynamicObjects.erase(iter);
        }
        else {
            ++iter;
        }
    }
    
    return scene;
}

void GameScene::AddEnemyObject(const std::shared_ptr<Enemy>& game_object, bool init)
{
    m_wpEnemyObjects.push_back(game_object);
    if (init) {
        game_object->Init();
    }
    m_spGameObjects.push_back(game_object);
}

void GameScene::AddDynamicObject(const std::shared_ptr<DynamicObject>& game_object, bool init)
{
    m_wpDynamicObjects.push_back(game_object);
    if (init) {
        game_object->Init();
    }
    m_spGameObjects.push_back(game_object);
}
