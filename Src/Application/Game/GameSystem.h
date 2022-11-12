#pragma once

#include "ExternalDependencies/Audio/AudioManager.h"
#include "ExternalDependencies/Effekseer/EffekseerManager.h"
#include "ExternalDependencies/Input/InputManager.h"
#include "ExternalDependencies/PhysX/PhysXManager.h"
#include "System/Asset/AssetManager/AssetManager.h"
#include "Application/Game/Scene/Scene.h"

class DynamicObject;
enum class KeyType {
    MoveForward,
    MoveBackward,
    StrafeLeft,
    StrafeRight,
    Sprint,
    Shoot,
    Aim,
    Interact,
    //MelleAttack,
};

class GameSystem
{
public:

    /* Graphics */
    enum class DisplayMode {
        WINDOWED,
        BORDERLESS,
        FULLSCREEN,
    };
    enum class ResolutionType {
        HD,    // { 1280,  720 }
        FHD,   // { 1920, 1080 }
        QHD,   // { 2560, 1440 }
        UHD,   // { 3840, 2160 }
        UWQHD, // { 3440, 1440 }
        UWQHDP,// { 3840, 1600 }
    };
    enum class FrameRateLimit {
        NO   = 0,
        _30  = 30,
        _60  = 60,
        _90  = 90,
        _120 = 120,
        _144 = 144,
        _240 = 240,
        _360 = 360,
    };
    enum class AnisotropicFiltering {
        ANISOTROPICx1,
        ANISOTROPICx2,
        ANISOTROPICx4,
        ANISOTROPICx8,
        ANISOTROPICx16,
    };
    enum class ShadowMaps {
        NO,
        x512,
        x1024,
        x2048,
        x4096,
        x8192,
    };

    ~GameSystem() {
        Release();
    }

    void Init();
    void Update();
    void Draw();
    void ImGuiUpdate();

    const std::unique_ptr<AudioManager>& GetAudioManager() const noexcept {
        return m_upAudioMgr;
    }
    const std::unique_ptr<EffekseerManager>& GetEffekseerManager() const noexcept {
        return m_upEffekseerMgr;
    }
    const std::unique_ptr<InputManager>& GetInputManager() const noexcept {
        return m_upInputMgr;
    }
    const std::unique_ptr<KeyConfigManager<KeyType>>& GetKeyConfigManager() const noexcept {
        return m_upKeyConfigMgr;
    }
    const std::unique_ptr<PhysXManager>& GetPhysXManager() const noexcept {
        return m_upPhysxMgr;
    }
    const std::unique_ptr<AssetManager>& GetAssetManager() const noexcept {
        return m_upAssetMgr;
    }
    
    std::shared_ptr<Scene> GetScene() noexcept {
        return m_spScene;
    }
    std::shared_ptr<const Scene> GetScene() const noexcept {
        return m_spScene;
    }
    
    void ChangeScene(Scene::SceneType scene_type);

    void CalcFps();

    void ApplyGraphicsSettings(
        DisplayMode          display_mode,
        ResolutionType       resolution_type,
        FrameRateLimit       frame_rate_limit,
        float                render_scaling,
        AnisotropicFiltering anisotropic_filtering,
        ShadowMaps           shadow_maps
    );

    /*
    auto& am = Application::Instance().GetGameSystem()->GetAudioManager();
    auto& em = Application::Instance().GetGameSystem()->GetEffekseerManager();
    auto& im = Application::Instance().GetGameSystem()->GetInputManager();
    auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();
    auto& cm = Application::Instance().GetGameSystem()->GetInputManager()->GetCursorManager();
    auto& kcm = Application::Instance().GetGameSystem()->GetKeyConfigManager();
    auto& pm = Application::Instance().GetGameSystem()->GetPhysXManager();
    auto& am = Application::Instance().GetGameSystem()->GetAssetManager();
    auto& jm = Application::Instance().GetGameSystem()->GetAssetManager()->GetJsonMgr();
    auto& tm = Application::Instance().GetGameSystem()->GetAssetManager()->GetTextureMgr();
    auto& mm = Application::Instance().GetGameSystem()->GetAssetManager()->GetModelMgr();
    */

private:

    void Release() {
        imgui_helper::Release();
    }

    std::unique_ptr<AudioManager>              m_upAudioMgr;
    std::unique_ptr<EffekseerManager>          m_upEffekseerMgr;
    std::unique_ptr<InputManager>              m_upInputMgr;
    std::unique_ptr<KeyConfigManager<KeyType>> m_upKeyConfigMgr;
    std::unique_ptr<PhysXManager>              m_upPhysxMgr;
    std::unique_ptr<AssetManager>              m_upAssetMgr;
    std::shared_ptr<Scene>                     m_spScene;

    double m_fps = 0;
    
    /* Graphics */
    DisplayMode    m_displayMode    = DisplayMode::WINDOWED;
    ResolutionType m_resolutionType = ResolutionType::HD;
    FrameRateLimit m_frameRateLimit = FrameRateLimit::_60;
    float          m_renderScaling  = 100.f;
    AnisotropicFiltering m_anisotropicFiltering = AnisotropicFiltering::ANISOTROPICx1;
    ShadowMaps     m_shadowMaps     = ShadowMaps::x512;
    /* Sound */
    float          m_masterVolume   = 0.f;
    
};
