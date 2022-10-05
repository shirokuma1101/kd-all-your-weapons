#include "GameSystem.h"

#include "Math/Convert.h"
#include "Application/Game/GameObject/CameraObject/CameraObject.h"
#include "Application/Game/GameObject/ModelObject/ModelObject.h"
#include "Application/Game/GameObject/ModelObject/CharacterObject/Player/Player.h"
#include "Application/Game/GameObject/ModelObject/CharacterObject/Enemy/Enemy.h"
#include "Application/Game/GameObject/ModelObject/DynamicObject/DynamicObject.h"

void GameSystem::Init()
{
    /**************************************************
    * Manager
    **************************************************/

    /* ImGui */
    imgui_helper::Init(Application::Instance().GetWindow().GetWindowHandle(), DirectX11System::Instance().GetDev().Get(), DirectX11System::Instance().GetCtx().Get());

    /* Audio */
    m_upAudioMgr = std::make_unique<AudioManager>();
    m_upAudioMgr->Init();
    m_upAudioMgr->SetAudio("test", "D:/OneDrive/SystemFolderForSync/Music/VOCALOID/VOCALOID/マジカルミライ/wav/マジカルミライ2013_1.wav");

    /* Effekseer */
    m_upEffekseerMgr = std::make_unique<EffekseerManager>();
    m_upEffekseerMgr->Init(DirectX11System::Instance().GetDev().Get(), DirectX11System::Instance().GetCtx().Get());
    m_upEffekseerMgr->SetEffect("exp", "Asset/Effect/exp/exp.efk");
    m_upEffekseerMgr->SetEffect("charge", "Asset/Effect/charge/blue_laser_edited_long.efk");

    /* Input */
    m_upInputMgr = std::make_unique<InputManager>(Application::Instance().GetWindow().GetWindowHandle());

    /* PhysX */
    m_upPhysxMgr = std::make_unique<PhysXManager>();
    m_upPhysxMgr->Init();

    /* Asset */
    m_upAssetMgr = std::make_unique<AssetManager>();
    m_upAssetMgr->Register(AssetManager::AssetType::Json, "Asset/Json/json_list.json");
    m_upAssetMgr->Load(AssetManager::AssetType::Json);
    m_upAssetMgr->Register(AssetManager::AssetType::Model, m_upAssetMgr->GetJsonMgr()->GetAssets(), { "model" });


    /* Shader */
    DirectX11System::Instance().GetShaderManager()->ChangeRasterizerState(DirectX11System::Instance().GetShaderManager()->m_rs_CullNone);
    //auto rs = directx11_helper::CreateRasterizerState(DirectX11System::Instance().GetDev().Get(), D3D11_FILL_MODE::D3D11_FILL_WIREFRAME, D3D11_CULL_MODE::D3D11_CULL_NONE, true, false);
    //DirectX11System::Instance().GetCtx()->RSSetState(rs);
    //memory::SafeRelease(&rs);

    DirectX11System::Instance().GetShaderManager()->m_cb8_Light.Get()->AmbientLight = { 0.9f, 0.8f, 0.8f, 1.f };
    DirectX11System::Instance().GetShaderManager()->m_cb8_Light.Get()->DirLight_Color = { 1.f, 1.f, 1.f };
    DirectX11System::Instance().GetShaderManager()->m_cb8_Light.Write();

    /* GameObject */
    auto player = std::make_shared<Player>("player");
    player->SetEquipWeightLimit(2.f);
    AddGameObject(player);

    //auto dummy_character = std::make_shared<ModelObject>("dummy_character");
    //AddGameObject(dummy_character);

    for (const auto& name : {
        "environment_sphere",
        "abandoned_abandon_hospital2",
        "abandoned_building_construction",
        "abandoned_favela",
        "abandoned_post_apocalyptic_building",
        "abandoned_post_apocalyptic_building_with_props",
        //"post-apocalyptic_building_1",
        //"post-apocalyptic_building_2",
        //"post-apocalyptic_building_3",
        //"post-apocalyptic_building_4",
        //"post-apocalyptic_building_5",
        //"post-apocalyptic_building_6",
        //"abandoned_ruin_building",
        "terrain_road_01",
        "terrain_road_02",
        "terrain_road_03",
        "terrain_road_04",
        "terrain_road_05",
        "terrain_road_06",
        }) {
        AddGameObject(std::make_shared<ModelObject>(name));
    }
    for (const auto& name : {
        "props_01",
        "props_02"
        }) {
        AddDynamicObject(std::make_shared<DynamicObject>(name));
    }

    auto camera = std::make_shared<CameraObject>(Math::Matrix::Identity, 60.f, Window::ToAspectRatio(Application::Instance().GetWindow().GetSize()));
    camera->SetFollowingTarget(player, Transform({ 0.5f, -0.1f, -0.8f }));
    AddGameObject(camera);

    player->SetFollowerTarget(camera);
}

void GameSystem::Update()
{
    /* Time */
    static const double delta_time_threshold = 1.0;
    const double        delta_time = Application::Instance().GetDeltaTime();
    const float         fdelta_time = static_cast<float>(delta_time);
    if (delta_time > delta_time_threshold || delta_time <= 0.0) return;

    /* Manager */
    m_upEffekseerMgr->Update(delta_time);
    m_upInputMgr->Update();
    m_upPhysxMgr->Update(delta_time);

    /* GameObject */
    for (const auto& obj : m_spGameObjects) {
        obj->PreUpdate();
    }
    for (auto iter = m_spGameObjects.begin(); iter != m_spGameObjects.end();) {
        (*iter)->Update(fdelta_time);
        if ((*iter)->IsObjectAlive()) {
            ++iter;
        }
        else {
            iter = m_spGameObjects.erase(iter);
        }
    }
    
    // Set in place of camera
    m_upAudioMgr->Update(DirectX11System::Instance().GetShaderManager()->m_cb7_Camera.Get()->CamPos, {});
    // Set camera to Effekseer
    m_upEffekseerMgr->SetCamera(
        DirectX11System::Instance().GetShaderManager()->m_cb7_Camera.Get()->mProj,
        DirectX11System::Instance().GetShaderManager()->m_cb7_Camera.Get()->mView
    );

    if (m_upInputMgr->GetKeyManager()->GetState(VK_F1, KeyManager::KeyState::Press)) {
        m_upAssetMgr->Load(AssetManager::AssetType::Json);
    }
}

void GameSystem::Draw()
{
    DirectX11System::Instance().GetCtx()->ClearRenderTargetView(DirectX11System::Instance().GetBackBuffer()->GetRtv(), directx11_helper::blue);
    DirectX11System::Instance().GetCtx()->ClearDepthStencilView(DirectX11System::Instance().GetZBuffer()->GetDsv(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    DirectX11System::Instance().GetShaderManager()->m_standardShader.SetToDevice();
    for (const auto& e : m_spGameObjects) {
        e->DrawTransparent();
    }
    for (const auto& e : m_spGameObjects) {
        e->DrawOpaque();
    }

    m_upEffekseerMgr->Draw();
}

void GameSystem::ImGuiUpdate()
{
    static bool is_push = true;
    if (m_upInputMgr->GetKeyManager()->GetState(VK_F2, KeyManager::KeyState::Press)) {
        if (!is_push)is_push = true;
        else is_push = false;
    }
    if (is_push) return;
    
    imgui_helper::Begin();
    
    double delta_time = Application::Instance().GetDeltaTime();

    static const double interval_sec = 1.0; // FPS取得更新間隔
    static double       elapsed_sec  = 0.0; // FPS取得用経過時間
    static int          fps_counter  = 0;   // FPS取得用カウンタ

    static double       now_fps         = 0.0;  // 現在のFPS
    static float        fps_limit_float = 60.f; // FPS制限

    elapsed_sec += delta_time;
    ++fps_counter;
    if (elapsed_sec > interval_sec) {
        now_fps = static_cast<float>(fps_counter / interval_sec);
        elapsed_sec = 0.0;
        fps_counter = 0;
    }

    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGui::SetNextWindowSize(ImVec2(200.f, 200.f));
    if (ImGui::Begin("SceneManagement")) {
        ImGui::Text("FPS: %lf", now_fps);
        ImGui::SliderFloat("Limit", &fps_limit_float, 5, 144);
    }
    ImGui::End();

    double fps_limit = 1000.0 / static_cast<double>(fps_limit_float);
    double delta_time_ms = convert::SToMS(delta_time);
    fps_limit -= delta_time_ms;
    //Sleep(static_cast<DWORD>(std::round(fps_limit > 0.0 ? fps_limit : 0.0)));

    for (const auto& e : m_spGameObjects) {
        e->ImGuiUpdate();
    }

    imgui_helper::Draw();
}

void GameSystem::AddDynamicObject(const std::shared_ptr<DynamicObject>& obj, bool init)
{
    if (init) {
        obj->Init();
    }
    m_wpDynamicObjects.push_back(obj);
    m_spGameObjects.push_back(obj);
}
