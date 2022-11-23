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

        "rebel-army_abandoned_security_hut_low_poly.json",

        "abandoned_abandon_hospital2",
        "abandoned_building_construction",
        "abandoned_camp_fire",
        "abandoned_favela",
        "abandoned_post_apocalyptic_building",
        "post-apocalyptic_building_1",
        "post-apocalyptic_building_2",
        "post-apocalyptic_building_3",
        "post-apocalyptic_building_4",
        "post-apocalyptic_building_5",
        "post-apocalyptic_building_6",
        "abandoned_ruin_building",
        
        "terrain_road_01",
        "terrain_road_02",
        "terrain_road_03",
        "terrain_road_04",
        "terrain_road_05",
        "terrain_sidewalk_01",
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
    DirectX11System::WorkInstance().GetShaderManager()->ChangeRasterizerState(DirectX11System::Instance().GetShaderManager()->m_pRSSolidNone);
    auto camera_cb = DirectX11System::WorkInstance().GetShaderManager()->GetCameraCB().Get();
    camera_cb->distanceFogEnable = true;
    camera_cb->distanceFogColor  = { 0.75f, 0.64f, 0.6f };
    camera_cb->distanceFogStart  = -19.3f;
    camera_cb->distanceFogEnd    = 143.6f;
    DirectX11System::WorkInstance().GetShaderManager()->GetCameraCB().Write();
    auto light_cb = DirectX11System::WorkInstance().GetShaderManager()->GetLightCB().Get();
    light_cb->ambientLight              = { 0.7f, 0.6f, 0.6f };
    light_cb->directionalLightDirection = { -0.5, -0.8f, 0 };
    light_cb->directionalLightDirection.Normalize();
    light_cb->directionalLightColor     = { 0.7f, 0.7f, 0.7f };
    DirectX11System::WorkInstance().GetShaderManager()->GetLightCB().Write();

    DirectX11System::WorkInstance().GetShaderManager()->AddPointLight({ 0, 1, 0 }, { 2.0f, 1.0f, 0.1f }, { 0.f, 1.f, 0.f });

    auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();
    mm->Load("props_dirt");
}

Scene::SceneType GameScene::Update(float delta_time)
{
    const auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();

    DirectX11System::WorkInstance().GetShaderManager()->GetLightCB().Get()->pointLight[0].attenuation = { 0.f, (float)random::RandomDistribution(0.75f, 1.f), 0.f };
    DirectX11System::WorkInstance().GetShaderManager()->GetLightCB().Write();

    static bool is_pause = false;
    if (km->GetState(VK_ESCAPE, KeyManager::KEYSTATE_PRESS)) {
        if (!is_pause) {
            is_pause = true;
        }
        else {
            is_pause = false;
        }
    }
    if (is_pause) return m_sceneType;
    
    return Scene::Update(delta_time);
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
