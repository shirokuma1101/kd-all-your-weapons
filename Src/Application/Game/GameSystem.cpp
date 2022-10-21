#include "GameSystem.h"

#include "Math/Convert.h"
#include "Application/Game/Scene/TitleScene/TitleScene.h"
#include "Application/Game/Scene/GameScene/GameScene.h"
#include "Application/Game/Scene/ResultScene/ResultScene.h"

void GameSystem::Init()
{
    /**************************************************
    * ini
    **************************************************/
    const Window::Size window_size = Window::FHD;
    const float        rendering_resolution_percent = 100.f;
    const bool         is_full_screen = false;
    const double       fps = 144.0;
    Application::WorkInstance().SetWindowSettings(window_size, rendering_resolution_percent, is_full_screen);
    m_fpsLimit = fps;
    

    /**************************************************
    * Manager
    **************************************************/

    /* ImGui */
    imgui_helper::Init(Application::Instance().GetWindow().GetWindowHandle(), DirectX11System::Instance().GetDev().Get(), DirectX11System::Instance().GetCtx().Get());

    /* Audio */
    m_upAudioMgr = std::make_unique<AudioManager>();
    m_upAudioMgr->Init();
    m_upAudioMgr->SetAudio("aki_bgm", "Asset/Sound/aki_bgm.wav");
    m_upAudioMgr->SetAudio("charge_se", "Asset/Sound/charge_se.wav");
    //m_upAudioMgr->Play("aki_bgm", AudioManager::PlayFlags::Loop)->SetVolume(0.5f);

    /* Effekseer */
    m_upEffekseerMgr = std::make_unique<EffekseerManager>();
    m_upEffekseerMgr->Init(DirectX11System::Instance().GetDev().Get(), DirectX11System::Instance().GetCtx().Get());
    m_upEffekseerMgr->SetEffect("exp", "Asset/Effect/exp/exp.efk");
    m_upEffekseerMgr->SetEffect("charge", "Asset/Effect/charge/blue_laser_edited_long.efk");

    /* Input */
    m_upInputMgr = std::make_unique<InputManager>(Application::Instance().GetWindow().GetWindowHandle());
    //input_helper::CursorData::ShowCursor(false);

    /* PhysX */
    m_upPhysxMgr = std::make_unique<PhysXManager>();
    m_upPhysxMgr->Init();

    /* Asset */
    m_upAssetMgr = std::make_unique<AssetManager>();
    m_upAssetMgr->Register(AssetManager::AssetType::Json, "Asset/Json/json_list.json");
    m_upAssetMgr->Load(AssetManager::AssetType::Json);
    m_upAssetMgr->Register(AssetManager::AssetType::Model, m_upAssetMgr->GetJsonMgr()->GetAssets(), { "model" });
    
    
    /**************************************************
    * Scene
    **************************************************/

    m_spScene = std::make_shared<GameScene>();
    m_spScene->Init();
}

void GameSystem::Update()
{
    /* Time */
    CalcFps();
    static const double delta_time_threshold = 1.0;
    const double        delta_time           = Application::Instance().GetDeltaTime();
    if (delta_time > delta_time_threshold || delta_time <= 0.0) return;

    /* Manager */
    m_upEffekseerMgr->Update(delta_time);
    m_upInputMgr->Update();
    m_upPhysxMgr->Update(delta_time);

    /* Scene */
    ChangeScene(m_spScene->Update(static_cast<float>(delta_time)));
    
    // Set in place of camera
    m_upAudioMgr->Update(DirectX11System::Instance().GetShaderManager()->GetCameraCB().Get()->position, {});
    // Set camera to Effekseer
    m_upEffekseerMgr->SetCamera(
        DirectX11System::Instance().GetShaderManager()->GetCameraCB().Get()->projection,
        DirectX11System::Instance().GetShaderManager()->GetCameraCB().Get()->view
    );

    if (m_upInputMgr->GetKeyManager()->GetState(VK_F1, KeyManager::KeyState::Press)) {
        m_upAssetMgr->Load(AssetManager::AssetType::Json);
    }
}

void GameSystem::Draw()
{
    const FLOAT color[4] = { 0.6f, 0.6f, 0.6f, 1.f };
    DirectX11System::Instance().GetCtx()->ClearRenderTargetView(DirectX11System::Instance().GetBackBuffer()->GetRtv(), color);
    DirectX11System::Instance().GetCtx()->ClearDepthStencilView(DirectX11System::Instance().GetZBuffer()->GetDsv(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    /* Scene */
    m_spScene->Draw();

    /* Effekseer */
    m_upEffekseerMgr->Draw();
}

void GameSystem::ImGuiUpdate()
{
    imgui_helper::Begin();
    
    static bool is_show = false;
    if (m_upInputMgr->GetKeyManager()->GetState(VK_F2, KeyManager::KeyState::Press)) {
        if (!is_show) {
            is_show = true;
        }
        else {
            is_show = false;
        }
    }
    if (is_show) {
        static int  resolution = 0;
        static bool is_change_resolution = false;

        ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
        ImGui::SetNextWindowSize(ImVec2(300.f, 150.f));
        if (ImGui::Begin("GameSystem")) {
            ImGui::Text("FPS: %lf", m_fps);
            ImGui::Checkbox("Unlimited FPS", &m_isUnlimitedFps);
            ImGui::SliderFloat("FPS Limit", &m_fpsLimit, 5, 240);

            is_change_resolution |= ImGui::RadioButton("HD", &resolution, 0);
            ImGui::SameLine();
            is_change_resolution |= ImGui::RadioButton("FHD", &resolution, 1);
            ImGui::SameLine();
            is_change_resolution |= ImGui::RadioButton("QHD", &resolution, 2);
            ImGui::SameLine();
            is_change_resolution |= ImGui::RadioButton("UHD", &resolution, 3);
        }
        ImGui::End();

        if (is_change_resolution) {
            is_change_resolution = false;
            switch (resolution) {
            case 0:
                Application::WorkInstance().SetWindowSettings(Window::HD);
                break;
            case 1:
                Application::WorkInstance().SetWindowSettings(Window::FHD);
                break;
            case 2:
                Application::WorkInstance().SetWindowSettings(Window::QHD);
                break;
            case 3:
                Application::WorkInstance().SetWindowSettings(Window::UHD);
                break;
            }
        }
        m_spScene->ImGuiUpdate();
    }
    
    imgui_helper::Draw();
}

void GameSystem::ChangeScene(Scene::SceneType scene_type)
{
    if (scene_type != m_spScene->GetSceneType()) {
        switch (scene_type) {
        case Scene::SceneType::Title:
            m_spScene = std::make_shared<TitleScene>();
            break;
        case Scene::SceneType::Game:
            m_spScene = std::make_shared<GameScene>();
            break;
        case Scene::SceneType::Result:
            m_spScene = std::make_shared<ResultScene>();
            break;
        default:
            break;
        }
        m_spScene->Init();
    }
}

void GameSystem::CalcFps()
{
    const double delta_time = Application::Instance().GetDeltaTime();

    static const double interval_sec = 1.0; // FPS取得更新間隔
    static double       elapsed_sec  = 0.0; // FPS取得用経過時間
    static int          fps_counter  = 0;   // FPS取得用カウンタ

    // FPS取得
    elapsed_sec += delta_time;
    ++fps_counter;
    if (elapsed_sec > interval_sec) {
        m_fps = fps_counter / interval_sec;
        elapsed_sec = 0.0;
        fps_counter = 0;
    }

    // FPS制御
    if (m_isUnlimitedFps) return;
    double limit_ms = 1000.0 / static_cast<double>(m_fpsLimit);
    if (double sleep = limit_ms - convert::SToMS(delta_time); sleep > 0.f) {
        if (sleep > limit_ms) {
            sleep = limit_ms;
        }
        Sleep(static_cast<DWORD>(sleep));
    };
}
