#pragma once

#include "ExternalDependencies/Audio/AudioManager.h"
#include "ExternalDependencies/Effekseer/EffekseerManager.h"
#include "ExternalDependencies/Input/InputManager.h"
#include "ExternalDependencies/PhysX/PhysXManager.h"
#include "System/Asset/AssetManager/AssetManager.h"
#include "Application/Game/Scene/Scene.h"

class DynamicObject;

class GameSystem
{
public:

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
    const std::unique_ptr<PhysXManager>& GetPhysXManager() const noexcept {
        return m_upPhysxMgr;
    }
    const std::unique_ptr<AssetManager>& GetAssetManager() const noexcept {
        return m_upAssetMgr;
    }
    
    std::shared_ptr<Scene>& GetScene() noexcept {
        return m_spScene;
    }
    const std::shared_ptr<Scene>& GetScene() const noexcept {
        return m_spScene;
    }
    
    void ChangeScene(Scene::SceneType scene_type);

    void CalcFps();
    
    /*
    auto& am = Application::Instance().GetGameSystem()->GetAudioManager();
    auto& em = Application::Instance().GetGameSystem()->GetEffekseerManager();
    auto& im = Application::Instance().GetGameSystem()->GetInputManager();
    auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();
    auto& cm = Application::Instance().GetGameSystem()->GetInputManager()->GetCursorManager();
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

    std::unique_ptr<AudioManager>     m_upAudioMgr;
    std::unique_ptr<EffekseerManager> m_upEffekseerMgr;
    std::unique_ptr<InputManager>     m_upInputMgr;
    std::unique_ptr<PhysXManager>     m_upPhysxMgr;
    std::unique_ptr<AssetManager>     m_upAssetMgr;
    std::shared_ptr<Scene>            m_spScene;

    double m_fps            = 0;
    bool   m_isUnlimitedFps = false;
    float  m_fpsLimit       = 0;
    
};
