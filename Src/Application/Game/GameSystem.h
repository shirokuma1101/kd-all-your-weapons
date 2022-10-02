#pragma once

#include "System/GameObject/GameObject.h"

#include "ExternalDependencies/ImGui/ImGuiHelper.h"

#include "ExternalDependencies/Audio/AudioManager.h"
#include "ExternalDependencies/Effekseer/EffekseerManager.h"
#include "ExternalDependencies/Input/InputManager.h"
#include "ExternalDependencies/PhysX/PhysXManager.h"
#include "System/Asset/AssetManager/AssetManager.h"

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

    std::list<std::shared_ptr<GameObject>>& GetGameObjects() noexcept {
        return m_spGameObjects;
    }
    const std::list<std::shared_ptr<GameObject>>& GetGameObjects() const noexcept {
        return m_spGameObjects;
    }
    std::list<std::weak_ptr<DynamicObject>>& GetDynamicObjects() noexcept {
        return m_wpDynamicObjects;
    }
    const std::list<std::weak_ptr<DynamicObject>>& GetDynamicObjects() const noexcept {
        return m_wpDynamicObjects;
    }

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

    void AddGameObject(const std::shared_ptr<GameObject>& obj, bool init = true) {
        if (init) {
            obj->Init();
        }
        m_spGameObjects.push_back(obj);
    }
    void AddDynamicObject(const std::shared_ptr<DynamicObject>& obj, bool init = true);
    
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

    std::unique_ptr<AudioManager>           m_upAudioMgr;
    std::unique_ptr<EffekseerManager>       m_upEffekseerMgr;
    std::unique_ptr<InputManager>           m_upInputMgr;
    std::unique_ptr<PhysXManager>           m_upPhysxMgr;
    std::unique_ptr<AssetManager>           m_upAssetMgr;
    
    std::list<std::shared_ptr<GameObject>>  m_spGameObjects;
    std::list<std::weak_ptr<DynamicObject>> m_wpDynamicObjects;

};
