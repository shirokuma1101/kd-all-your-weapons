#include "GameSystem.h"

#include "Math/Convert.h"
#include "Application/Game/Scene/TitleScene/TitleScene.h"
#include "Application/Game/Scene/GameScene/GameScene.h"
#include "Application/Game/Scene/ResultScene/ResultScene.h"

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
    m_upAudioMgr->SetMasterVolume(m_masterVolume);
    m_upAudioMgr->SetAudio("aki_bgm", "Asset/Sound/aki_bgm.wav");
    m_upAudioMgr->SetAudio("charge_se", "Asset/Sound/charge_se.wav");
    m_upAudioMgr->Play("aki_bgm", AudioManager::PLAYFLAGS_LOOP);

    /* Effekseer */
    m_upEffekseerMgr = std::make_unique<EffekseerManager>();
    m_upEffekseerMgr->Init(DirectX11System::Instance().GetDev().Get(), DirectX11System::Instance().GetCtx().Get());
    m_upEffekseerMgr->SetEffect("exp", "Asset/Effect/exp/exp.efk");
    m_upEffekseerMgr->SetEffect("charge", "Asset/Effect/charge/blue_laser_edited_long.efk");

    /* Input */
    m_upInputMgr = std::make_unique<InputManager>(Application::Instance().GetWindow().GetWindowHandle());
    m_upKeyConfigMgr = std::make_unique<KeyConfigManager<KeyType>>();
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
    * settings (ref:"https://www.flightsimulator.blog/2022/10/21/best-graphics-settings-guide/")
    **************************************************/

    //! 初回Borderless時にウィンドウサイズになるので、最初はBorderlessで初期化
    ApplyGraphicsSettings(DisplayMode::BORDERLESS, m_resolutionType, m_frameRateLimit, m_renderScaling, m_anisotropicFiltering, m_shadowMaps);

    /* Graphics */
    m_displayMode          = DisplayMode::WINDOWED;
    m_resolutionType       = ResolutionType::HD;
    m_frameRateLimit       = FrameRateLimit::_60;
    m_renderScaling        = 100.f;
    m_anisotropicFiltering = AnisotropicFiltering::ANISOTROPICx1;
    m_shadowMaps           = ShadowMaps::x512;
    /* Sound */
    m_masterVolume         = 0.f;
    /* Controls */
    m_upKeyConfigMgr->AddKeyConfig(KeyType::MoveForward,  'W');
    m_upKeyConfigMgr->AddKeyConfig(KeyType::MoveBackward, 'S');
    m_upKeyConfigMgr->AddKeyConfig(KeyType::StrafeLeft,   'A');
    m_upKeyConfigMgr->AddKeyConfig(KeyType::StrafeRight,  'D');
    m_upKeyConfigMgr->AddKeyConfig(KeyType::Sprint,       VK_SHIFT);
    m_upKeyConfigMgr->AddKeyConfig(KeyType::Shoot,        VK_LBUTTON);
    m_upKeyConfigMgr->AddKeyConfig(KeyType::Aim,          VK_RBUTTON);
    m_upKeyConfigMgr->AddKeyConfig(KeyType::Interact,     'F');

    m_upAudioMgr->SetMasterVolume(m_masterVolume);
    ApplyGraphicsSettings(m_displayMode, m_resolutionType, m_frameRateLimit, m_renderScaling, m_anisotropicFiltering, m_shadowMaps);


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
    constexpr double delta_time_threshold = 1.0;
    const double     delta_time           = Application::Instance().GetDeltaTime();
    if (delta_time > delta_time_threshold || delta_time <= 0.0) return;

    /* Shader */
    switch (m_anisotropicFiltering) {
    case AnisotropicFiltering::ANISOTROPICx1:
        DirectX11System::WorkInstance().GetCtx()->PSSetSamplers(0, 1, &DirectX11System::Instance().GetShaderManager()->m_pSSAnisotropicWrap);
        break;
    case AnisotropicFiltering::ANISOTROPICx2:
        DirectX11System::WorkInstance().GetCtx()->PSSetSamplers(0, 1, &DirectX11System::Instance().GetShaderManager()->m_pSSAnisotropic2xWrap);
        break;
    case AnisotropicFiltering::ANISOTROPICx4:
        DirectX11System::WorkInstance().GetCtx()->PSSetSamplers(0, 1, &DirectX11System::Instance().GetShaderManager()->m_pSSAnisotropic4xWrap);
        break;
    case AnisotropicFiltering::ANISOTROPICx8:
        DirectX11System::WorkInstance().GetCtx()->PSSetSamplers(0, 1, &DirectX11System::Instance().GetShaderManager()->m_pSSAnisotropic8xWrap);
        break;
    case AnisotropicFiltering::ANISOTROPICx16:
        DirectX11System::WorkInstance().GetCtx()->PSSetSamplers(0, 1, &DirectX11System::Instance().GetShaderManager()->m_pSSAnisotropic16xWrap);
        break;
    }

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
    if (m_upInputMgr->GetKeyManager()->GetState(VK_F2, KeyManager::KEYSTATE_PRESS)) {
        m_upAssetMgr->Load(AssetManager::AssetType::Json);
    }
}

void GameSystem::Draw()
{
    DirectX11System::Instance().GetCtx()->ClearRenderTargetView(DirectX11System::WorkInstance().GetBackBuffer()->GetRtv(), directx11_helper::white);
    DirectX11System::Instance().GetCtx()->ClearDepthStencilView(DirectX11System::WorkInstance().GetZBuffer()->GetDsv(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    /* Scene */
    m_spScene->Draw();

    /* Effekseer */
    m_upEffekseerMgr->Draw();
}

void GameSystem::ImGuiUpdate()
{
    using UtDisplayMode          = std::underlying_type_t<DisplayMode>;
    using UtResolutionType       = std::underlying_type_t<ResolutionType>;
    using UtFrameRateLimit       = std::underlying_type_t<FrameRateLimit>;
    using UtAnisotropicFiltering = std::underlying_type_t<AnisotropicFiltering>;
    using UtShadowMaps           = std::underlying_type_t<ShadowMaps>;

    static bool             is_show          = false;
    /* Graphics */
    static UtDisplayMode    display_mode     = static_cast<UtDisplayMode>(m_displayMode);
    static UtResolutionType resolution_type  = static_cast<UtResolutionType>(m_resolutionType);
    static UtFrameRateLimit frame_rate_limit = static_cast<UtFrameRateLimit>(m_frameRateLimit);
    static float            render_scaling   = m_renderScaling;
    static UtAnisotropicFiltering anisotropic_filtering = static_cast<UtAnisotropicFiltering>(m_anisotropicFiltering);
    static UtShadowMaps     shadow_maps      = static_cast<UtShadowMaps>(m_shadowMaps);
    /* Sound */
    static float          master_volume = 0.f;

    imgui_helper::Begin();

    if (m_upInputMgr->GetKeyManager()->GetState(VK_F1, KeyManager::KEYSTATE_PRESS)) {
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
        ImGui::SetNextWindowSize(ImVec2(350.f, 275.f));
        if (ImGui::Begin("GameSystem Settings")) {
            ImGui::Text("FPS: %lf", m_fps);
            /* Graphics */
            // Window type
            ImGui::RadioButton("Windowed", &display_mode, static_cast<UtDisplayMode>(DisplayMode::WINDOWED));
            ImGui::SameLine();
            ImGui::RadioButton("Borderless", &display_mode, static_cast<UtDisplayMode>(DisplayMode::BORDERLESS));
            ImGui::SameLine();
            ImGui::RadioButton("Fullscreen", &display_mode, static_cast<UtDisplayMode>(DisplayMode::FULLSCREEN));
            // Resolution
            ImGui::RadioButton("HD", &resolution_type, static_cast<UtResolutionType>(ResolutionType::HD));
            ImGui::SameLine();
            ImGui::RadioButton("FHD", &resolution_type, static_cast<UtResolutionType>(ResolutionType::FHD));
            ImGui::SameLine();
            ImGui::RadioButton("QHD", &resolution_type, static_cast<UtResolutionType>(ResolutionType::QHD));
            ImGui::SameLine();
            ImGui::RadioButton("UHD", &resolution_type, static_cast<UtResolutionType>(ResolutionType::UHD));
            // FPS
            ImGui::RadioButton("Unlimit", &frame_rate_limit, static_cast<UtFrameRateLimit>(FrameRateLimit::NO));
            ImGui::SameLine();
            ImGui::RadioButton("30", &frame_rate_limit, static_cast<UtFrameRateLimit>(FrameRateLimit::_30));
            ImGui::SameLine();
            ImGui::RadioButton("60", &frame_rate_limit, static_cast<UtFrameRateLimit>(FrameRateLimit::_60));
            ImGui::SameLine();
            ImGui::RadioButton("90", &frame_rate_limit, static_cast<UtFrameRateLimit>(FrameRateLimit::_90));
            ImGui::SameLine();
            ImGui::RadioButton("120", &frame_rate_limit, static_cast<UtFrameRateLimit>(FrameRateLimit::_120));
            ImGui::SameLine();
            ImGui::RadioButton("144", &frame_rate_limit, static_cast<UtFrameRateLimit>(FrameRateLimit::_144));
            ImGui::SameLine();
            ImGui::RadioButton("240", &frame_rate_limit, static_cast<UtFrameRateLimit>(FrameRateLimit::_240));
            ImGui::SameLine();
            ImGui::RadioButton("360", &frame_rate_limit, static_cast<UtFrameRateLimit>(FrameRateLimit::_360));
            ImGui::SameLine();
            // RenderScaling
            ImGui::SliderFloat("RenderScaling", &render_scaling, 50.f, 200.f);
            const auto& size = Application::Instance().GetWindow().GetSize();
            ImGui::Text(
                "%d x %d",
                static_cast<Window::Size::first_type>(size.first * (render_scaling / 100.f)),
                static_cast<Window::Size::second_type>(size.second * (render_scaling / 100.f))
            );
            // AnisotropicFiltering
            ImGui::RadioButton("Off", &anisotropic_filtering, static_cast<UtAnisotropicFiltering>(AnisotropicFiltering::ANISOTROPICx1));
            ImGui::SameLine();
            ImGui::RadioButton("x2", &anisotropic_filtering, static_cast<UtAnisotropicFiltering>(AnisotropicFiltering::ANISOTROPICx2));
            ImGui::SameLine();
            ImGui::RadioButton("x4", &anisotropic_filtering, static_cast<UtAnisotropicFiltering>(AnisotropicFiltering::ANISOTROPICx4));
            ImGui::SameLine();
            ImGui::RadioButton("x8", &anisotropic_filtering, static_cast<UtAnisotropicFiltering>(AnisotropicFiltering::ANISOTROPICx8));
            ImGui::SameLine();
            ImGui::RadioButton("x16", &anisotropic_filtering, static_cast<UtAnisotropicFiltering>(AnisotropicFiltering::ANISOTROPICx16));
            // ShadoMaps
            ImGui::RadioButton("No", &shadow_maps, static_cast<UtShadowMaps>(ShadowMaps::NO));
            ImGui::SameLine();
            ImGui::RadioButton("512", &shadow_maps, static_cast<UtShadowMaps>(ShadowMaps::x512));
            ImGui::SameLine();
            ImGui::RadioButton("1024", &shadow_maps, static_cast<UtShadowMaps>(ShadowMaps::x1024));
            ImGui::SameLine();
            ImGui::RadioButton("2048", &shadow_maps, static_cast<UtShadowMaps>(ShadowMaps::x2048));
            ImGui::SameLine();
            ImGui::RadioButton("4096", &shadow_maps, static_cast<UtShadowMaps>(ShadowMaps::x4096));
            ImGui::SameLine();
            ImGui::RadioButton("8192", &shadow_maps, static_cast<UtShadowMaps>(ShadowMaps::x8192));
            /* Sound */
            ImGui::SliderFloat("Master Volume", &master_volume, 0.f, 1.f);
            /* Scene */
            ImGui::Text("ID: %d", static_cast<int>(m_spScene->GetSceneType()));
        }
        if (ImGui::Button("Apply")) {
            m_upAudioMgr->SetMasterVolume(m_masterVolume = master_volume);
            ApplyGraphicsSettings(
                static_cast<DisplayMode>(display_mode),
                static_cast<ResolutionType>(resolution_type),
                static_cast<FrameRateLimit>(frame_rate_limit),
                render_scaling,
                static_cast<AnisotropicFiltering>(anisotropic_filtering),
                static_cast<ShadowMaps>(shadow_maps)
            );
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

    constexpr double interval_sec = 0.5; // FPS取得更新間隔
    static double    elapsed_sec  = 0.0; // FPS取得用経過時間
    static int       fps_counter  = 0;   // FPS取得用カウンタ
    static Timer     timer;              // FPS制御用タイマー

    // FPS取得
    elapsed_sec += delta_time;
    ++fps_counter;
    if (elapsed_sec > interval_sec) {
        m_fps = fps_counter / interval_sec;
        elapsed_sec = 0.0;
        fps_counter = 0;
    }

    // FPS制御
    int frame_rate_limit = static_cast<std::underlying_type_t<FrameRateLimit>>(FrameRateLimit::_360);
    if (!frame_rate_limit) return;
    double limit_ms = 1000.0 / frame_rate_limit;
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

void GameSystem::ApplyGraphicsSettings(DisplayMode display_mode, ResolutionType resolution_type, FrameRateLimit frame_rate_limit, float render_scaling, AnisotropicFiltering anisotropic_filtering, ShadowMaps shadow_maps)
{
    /* Graphics */
    switch (m_displayMode = display_mode) {
    case DisplayMode::WINDOWED:
        Application::WorkInstance().GetWindow().SetWindowStyle(WS_OVERLAPPEDWINDOW - WS_THICKFRAME);
        DirectX11System::Instance().GetSwapChain()->SetFullscreenState(FALSE, 0);
        break;
    case DisplayMode::BORDERLESS:
        Application::WorkInstance().GetWindow().SetWindowStyle(WS_POPUP);
        DirectX11System::Instance().GetSwapChain()->SetFullscreenState(FALSE, 0);
        break;
    case DisplayMode::FULLSCREEN:
        Application::WorkInstance().GetWindow().SetWindowStyle(WS_OVERLAPPEDWINDOW - WS_THICKFRAME);
        DirectX11System::Instance().GetSwapChain()->SetFullscreenState(TRUE, 0);
        break;
    }
    Window::Size resolution;
    switch (m_resolutionType = resolution_type) {
    case ResolutionType::HD:
        resolution = Window::HD;
        break;
    case ResolutionType::FHD:
        resolution = Window::FHD;
        break;
    case ResolutionType::QHD:
        resolution = Window::QHD;
        break;
    case ResolutionType::UHD:
        resolution = Window::UHD;
        break;
    }
    m_frameRateLimit = frame_rate_limit;
    m_renderScaling = render_scaling;
    m_anisotropicFiltering = anisotropic_filtering;
    std::pair<int32_t, int32_t> shadow_resolution;
    switch (m_shadowMaps = shadow_maps) {
    case ShadowMaps::NO:
        shadow_resolution = { 0, 0 };
        break;
    case ShadowMaps::x512:
        shadow_resolution = { 512, 512 };
        break;
    case ShadowMaps::x1024:
        shadow_resolution = { 1024, 1024 };
        break;
    case ShadowMaps::x2048:
        shadow_resolution = { 2048, 2048 };
        break;
    case ShadowMaps::x4096:
        shadow_resolution = { 4096, 4096 };
        break;
    case ShadowMaps::x8192:
        shadow_resolution = { 8192, 8192 };
        break;
    }

    // RenderTargetを全て解放する
    DirectX11System::WorkInstance().GetShaderManager()->GetStandardShader().ClearShadow();
    // リサイズ
    Application::WorkInstance().GetWindow().Resize(resolution);
    DirectX11System::WorkInstance().Resize({
        static_cast<Window::Size::first_type>(resolution.first * (m_renderScaling / 100.f)),
        static_cast<Window::Size::second_type>(resolution.second * (m_renderScaling / 100.f))
        });
    // RenderTargetを再生成する
    if (shadow_resolution != std::pair<int32_t, int32_t>(0, 0)) {
        DirectX11System::WorkInstance().GetShaderManager()->GetStandardShader().CreateShadow(shadow_resolution);
    }
}
