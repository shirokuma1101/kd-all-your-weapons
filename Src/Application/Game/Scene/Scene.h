#pragma once

#include "System/GameObject/GameObject.h"

class Scene
{
public:
    
    enum class SceneType {
        None,
        Title,
        Game,
        Result,
    };

    Scene(SceneType scene_type)
        : m_sceneType(scene_type)
    {}
    virtual ~Scene() {}

    virtual void Init() {
        for (const auto& e : m_spGameObjects) {
            e->Init();
        }
    }
    virtual SceneType Update(float delta_time) {
        for (const auto& obj : m_spGameObjects) {
            obj->PreUpdate();
        }
        for (auto iter = m_spGameObjects.begin(); iter != m_spGameObjects.end();) {
            (*iter)->Update(delta_time);
            if ((*iter)->IsObjectAlive()) {
                ++iter;
            }
            else {
                iter = m_spGameObjects.erase(iter);
            }
        }
        if (m_wpDeletionDecisionObject.expired()) {
            return m_nextSceneType;
        }
        return m_sceneType;
    }
    virtual void Draw() {
        DirectX11System::WorkInstance().GetShaderManager()->SetToDevice();
        
        auto& shader = DirectX11System::WorkInstance().GetShaderManager()->GetStandardShader();
        shader.SetToDevice();
        if (shader.BeginShadow()) {
            for (const auto& e : m_spGameObjects) {
                e->DrawOpaque();
            }
            shader.EndShadow();
        }
        shader.BeginStandard();
        for (const auto& e : m_spGameObjects) {
            e->DrawOpaque();
        }
        shader.EndStandard();
        
        DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().SetToDevice();
        for (const auto& e : m_spGameObjects) {
            e->DrawSprite();
        }
        
        DirectX11System::WorkInstance().GetShaderManager()->GetSpriteFont().Begin();
        for (const auto& e : m_spGameObjects) {
            e->DrawFont();
        }
        DirectX11System::WorkInstance().GetShaderManager()->GetSpriteFont().End();
    }
    virtual void ImGuiUpdate() {
        for (const auto& e : m_spGameObjects) {
            e->ImGuiUpdate();
        }
    }

    virtual SceneType GetSceneType() const noexcept final {
        return m_sceneType;
    }
    virtual std::list<std::shared_ptr<GameObject>> GetGameObjects() noexcept final {
        return m_spGameObjects;
    }
    virtual const std::list<std::shared_ptr<GameObject>>& GetGameObjects() const noexcept final {
        return m_spGameObjects;
    }

    void AddGameObject(std::shared_ptr<GameObject> game_object, bool init = true) {
        if (init) {
            game_object->Init();
        }
        m_spGameObjects.push_back(game_object);
    }

protected:

    const SceneType                        m_sceneType;
    SceneType                              m_nextSceneType = SceneType::Title;
    std::list<std::shared_ptr<GameObject>> m_spGameObjects;
    std::weak_ptr<GameObject>              m_wpDeletionDecisionObject;
    
};
