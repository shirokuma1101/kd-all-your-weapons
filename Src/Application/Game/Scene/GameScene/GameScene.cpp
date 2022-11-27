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

    auto camera = std::make_shared<CameraObject>();
    AddGameObject(camera);
    
    auto player = std::make_shared<Player>("player");
    player->SetEquipWeightLimit(2.f);
    player->SetFollowerTarget(camera);
    AddGameObject(player);

    for (const auto& name : {
        "enemy_01",
        "enemy_02",
        }) {
        auto enemy = std::make_shared<Enemy>(name);
        enemy->SetTarget(player);
        AddEnemyObject(enemy);
    }
    for (const auto& name : {
        "environment_sphere",

        "abandoned_abandon_hospital2",
        "abandoned_abandoned_security_hut_low_poly",
        "abandoned_building_construction",
        "abandoned_camp_fire",
        "abandoned_favela",
        "abandoned_old_rusty_car",
        "abandoned_post_apocalyptic_building",
        "post-apocalyptic_building_1",
        "post-apocalyptic_building_2",
        "post-apocalyptic_building_3",
        "post-apocalyptic_building_4",
        "post-apocalyptic_building_5",
        "post-apocalyptic_building_6",
        "abandoned_ruin_building",

        "rebel-army_btr_80a",
        "rebel-army_citizen_bordeaux_flat_1_corner_france",
        "rebel-army_citizen_bordeaux_flat_2_corner_france_01",
        "rebel-army_citizen_bordeaux_flat_2_corner_france_02",
        "rebel-army_citizen_buildings_front_01",
        "rebel-army_citizen_buildings_front_02",
        "rebel-army_citizen_buildings_front_03",
        "rebel-army_citizen_buildings_front_04",
        "rebel-army_citizen_buildings_pack_1_01",
        "rebel-army_citizen_buildings_pack_1_02",
        "rebel-army_citizen_buildings_pack_1_03",
        "rebel-army_citizen_buildings_pack_1_04",
        "rebel-army_citizen_buildings_pack_1_05",
        "rebel-army_citizen_buildings_pack_1_06",
        "rebel-army_citizen_buildings_pack_1_07",
        "rebel-army_pzkpfw_vi_tiger_1",
        "rebel-army_tent_01",
        "rebel-army_tent_02",
        "rebel-army_tent_03",

        "terrain_road_01",
        "terrain_road_02",
        "terrain_road_03",
        "terrain_road_04",
        "terrain_road_05",
        "terrain_road_06",
        "terrain_road_07",
        "terrain_road_08",
        "terrain_road_09",
        "terrain_road_10",
        "terrain_road_11",
        "terrain_road_12",
        "terrain_road_13",
        "terrain_sidewalk_01",
        "terrain_sidewalk_02",
        "terrain_sidewalk_03",
        "terrain_sidewalk_04",
        "terrain_sidewalk_05",
        "terrain_sidewalk_06",
        "terrain_sidewalk_07",
        "terrain_sidewalk_08",
        "terrain_sidewalk_09",
        "terrain_sidewalk_10",
        "terrain_sidewalk_11",
        "terrain_sidewalk_12",
        }) {
        AddGameObject(std::make_shared<ModelObject>(name));
    }
    for (const auto& name : {
        "props_01",
        "props_02",
        "props_03",
        "props_04",
        "props_05",
        "props_06",
        "props_07",
        }) {
        AddDynamicObject(std::make_shared<DynamicObject>(name));
    }
    
    auto game_ui = std::make_shared<GameUI>();
    AddGameObject(game_ui);

    m_nextSceneType = SceneType::Result;
    m_wpDeletionDecisionObject = game_ui;

    /* Shader */
    
    auto camera_cb = DirectX11System::WorkInstance().GetShaderManager()->GetCameraCB().Get();
    camera_cb->distanceFogEnable = true;
    camera_cb->distanceFogColor  = { 0.75f, 0.64f, 0.6f };
    camera_cb->distanceFogStart  = -19.3f;
    camera_cb->distanceFogEnd    = 143.6f;
    DirectX11System::WorkInstance().GetShaderManager()->GetCameraCB().Write();
    
    auto light_cb = DirectX11System::WorkInstance().GetShaderManager()->GetLightCB().Get();
    light_cb->ambientLight              = { 0.7f, 0.6f, 0.6f };
    light_cb->directionalLightDirection = { -0.5, -0.8f, 0.3f };
    light_cb->directionalLightDirection.Normalize();
    light_cb->directionalLightColor     = { 0.7f, 0.7f, 0.7f };
    DirectX11System::WorkInstance().GetShaderManager()->GetLightCB().Write();

    DirectX11System::WorkInstance().GetShaderManager()->AddPointLight({ 0, 1, 1 }, { 2.0f, 1.0f, 0.1f }, { 0.f, 1.f, 0.f });

    auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();
    mm->Load("props_dirt");
}

Scene::SceneType GameScene::Update(float delta_time)
{
    const auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();

    DirectX11System::WorkInstance().GetShaderManager()->GetLightCB().Get()->pointLight[0].attenuation = { 0.f, (float)random::RandomDistribution(0.75f, 1.f), 0.f };
    DirectX11System::WorkInstance().GetShaderManager()->GetLightCB().Write();

    if (km->GetState(VK_ESCAPE)) {
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
