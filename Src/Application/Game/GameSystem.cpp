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
    //const Window::Size window_size                      = Window::HD;
    const float        rendering_resolution_percentage  = 100.f;
    const bool         is_full_screen                   = false;
    const bool         unlimited_fps                    = false;
    const double       fps                              = 144.0;
    const float        master_volume                    = 0.f;
    
    Application::WorkInstance().SetWindowSettings(0, rendering_resolution_percentage, is_full_screen);
    m_isUnlimitedFps = unlimited_fps;
    m_fpsLimit       = fps;
    m_masterVolume   = master_volume;
    

    /**************************************************
    * Manager
    **************************************************/

    /* ImGui */
    imgui_helper::Init(Application::Instance().GetWindow().GetWindowHandle(), DirectX11System::Instance().GetDev().Get(), DirectX11System::Instance().GetCtx().Get());

    /* Audio */
    m_upAudioMgr = std::make_unique<AudioManager>();
    m_upAudioMgr->Init();
    m_upAudioMgr->SetMasterVolume(m_masterVolume);
    m_upAudioMgr->SetAudio("aki_bgm", "Asset/Sound/aki_bgm.wav");
    m_upAudioMgr->SetAudio("charge_se", "Asset/Sound/charge_se.wav");
    m_upAudioMgr->Play("aki_bgm", AudioManager::PlayFlags::Loop);

    /* Effekseer */
    m_upEffekseerMgr = std::make_unique<EffekseerManager>();
    m_upEffekseerMgr->Init(DirectX11System::Instance().GetDev().Get(), DirectX11System::Instance().GetCtx().Get());
    m_upEffekseerMgr->SetEffect("exp", "Asset/Effect/exp/exp.efk");
    m_upEffekseerMgr->SetEffect("charge", "Asset/Effect/charge/blue_laser_edited_long.efk");

    /* Input */
    m_upInputMgr = std::make_unique<InputManager>(Application::Instance().GetWindow().GetWindowHandle());
    m_upKeyConfigMgr = std::make_unique<KeyConfigManager<KeyType>>();
    m_upKeyConfigMgr->AddKeyConfig(KeyType::MoveForward,  'W');
    m_upKeyConfigMgr->AddKeyConfig(KeyType::MoveBackward, 'S');
    m_upKeyConfigMgr->AddKeyConfig(KeyType::StrafeLeft,   'A');
    m_upKeyConfigMgr->AddKeyConfig(KeyType::StrageRight,  'D');
    m_upKeyConfigMgr->AddKeyConfig(KeyType::Sprint,       VK_SHIFT);
    m_upKeyConfigMgr->AddKeyConfig(KeyType::Shoot,        VK_LBUTTON);
    m_upKeyConfigMgr->AddKeyConfig(KeyType::Aim,          VK_RBUTTON);
    m_upKeyConfigMgr->AddKeyConfig(KeyType::Interact,     'F');
    input_helper::CursorData::ShowCursor(false);

    /* PhysX */
    m_upPhysxMgr = std::make_unique<PhysXManager>();
    m_upPhysxMgr->Init();

    /* Asset */
    m_upAssetMgr = std::make_unique<AssetManager>();
    m_upAssetMgr->Register(AssetManager::AssetType::Json, "Asset/Json/json_list.json");
    m_upAssetMgr->Load(AssetManager::AssetType::Json);
    m_upAssetMgr->Register(AssetManager::AssetType::Model, m_upAssetMgr->GetJsonMgr()->GetAssets(), { "model" });

    /* Font */
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteFont().AddFont("genkai", "Asset/Font/GenkaiMincho.dat");
    
    
    /**************************************************
    * Scene
    **************************************************/

    m_spScene = std::make_shared<TitleScene>();
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
    m_upKeyConfigMgr->Update();
    m_upPhysxMgr->Update(delta_time);

    /* Scene */
    ChangeScene(m_spScene->Update(static_cast<float>(delta_time)));
    
    // Set in place of camera
    auto camera = DirectX11System::Instance().GetShaderManager()->GetCameraCB().Get();
    auto normalized_dir = camera->view.Backward();
    normalized_dir.Normalize();

    m_upAudioMgr->Update(camera->position, normalized_dir);
    // Set camera to Effekseer
    m_upEffekseerMgr->SetCamera(
        DirectX11System::Instance().GetShaderManager()->GetCameraCB().Get()->projection,
        DirectX11System::Instance().GetShaderManager()->GetCameraCB().Get()->view
    );

    /* Debug */
    if (m_upInputMgr->GetKeyManager()->GetState(VK_F2, KeyManager::KeyState::Press)) {
        m_upAssetMgr->Load(AssetManager::AssetType::Json);
    }
}

void GameSystem::Draw()
{
    DirectX11System::Instance().GetCtx()->ClearRenderTargetView(DirectX11System::Instance().GetBackBuffer()->GetRtv(), directx11_helper::white);
    DirectX11System::Instance().GetCtx()->ClearDepthStencilView(DirectX11System::Instance().GetZBuffer()->GetDsv(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    /* Scene */
    m_spScene->Draw();

    /* Effekseer */
    m_upEffekseerMgr->Draw();
}

void GameSystem::ImGuiUpdate()
{
    static bool  is_show        = false;
    static bool  unlimited_fps  = m_isUnlimitedFps;
    static float fps_limit      = m_fpsLimit;
    static int   resolution     = 0;
    static bool  is_full_screen = false;
    static float rendering_resolution_percentage = 100.f;
    static float master_volume  = m_masterVolume;

    imgui_helper::Begin();
    
    if (m_upInputMgr->GetKeyManager()->GetState(VK_F1, KeyManager::KeyState::Press)) {
        if (!is_show) {
            is_show = true;
            input_helper::CursorData::ShowCursor(true);
        }
        else {
            is_show = false;
            input_helper::CursorData::ShowCursor(false);
        }
    }
    if (is_show) {
        ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
        ImGui::SetNextWindowSize(ImVec2(300.f, 250.f));
        if (ImGui::Begin("GameSystem")) {
            /* FPS */
            ImGui::Text("FPS: %lf", m_fps);
            ImGui::Checkbox("Unlimited FPS", &unlimited_fps);
            ImGui::SliderFloat("FPS Limit", &fps_limit, 24, 240);
            /* Resolution */
            ImGui::RadioButton("HD", &resolution, 0);
            ImGui::SameLine();
            ImGui::RadioButton("FHD", &resolution, 1);
            ImGui::SameLine();
            ImGui::RadioButton("QHD", &resolution, 2);
            ImGui::SameLine();
            ImGui::RadioButton("UHD", &resolution, 3);
            /* Rendering Resolution Percentage */
            ImGui::SliderFloat("RenderingResolution", &rendering_resolution_percentage, 50.f, 200.f);
            const auto& size = Application::Instance().GetWindow().GetSize();
            ImGui::Text("%d x %d", static_cast<Window::Size::first_type>(size.first * (rendering_resolution_percentage / 100.f)), static_cast<Window::Size::second_type>(size.second * (rendering_resolution_percentage / 100.f)));
            /* Full Screen */
            ImGui::Checkbox("Full screen", &is_full_screen);
            /* Audio */
            ImGui::SliderFloat("Master Volume", &master_volume, 0.f, 1.f);
            /* Scene */
            ImGui::Text("ID: %d", static_cast<int>(m_spScene->GetSceneType()));
            
        }
        if (ImGui::Button("Apply")) {
            Application::WorkInstance().SetWindowSettings(resolution, rendering_resolution_percentage, is_full_screen);
            
            m_isUnlimitedFps = unlimited_fps;
            m_fpsLimit       = fps_limit;
            m_masterVolume   = master_volume;
            m_upAudioMgr->SetMasterVolume(m_masterVolume);

            //ImGui::GetStyle().ScaleAllSizes(convert::ToUndoPercent(rendering_resolution_percentage));
        }
        ImGui::End();
        
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

    static const double interval_sec = 0.5; // FPS取得更新間隔
    static double       elapsed_sec  = 0.0; // FPS取得用経過時間
    static int          fps_counter  = 0;   // FPS取得用カウンタ
    static Timer        timer;              // FPS制御用タイマー

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
        timer.Start();
        while (true) {
            timer.End();
            if (sleep < timer.Duration<Timer::MS>()) break;
        }
        //Sleep(static_cast<DWORD>(sleep));
    };
}
