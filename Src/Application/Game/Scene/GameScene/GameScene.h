#pragma once

#include "Application/Game/Scene/Scene.h"

class CharacterObject;
class Enemy;

class GameScene : public Scene
{
public:

    GameScene()
        : Scene(Scene::SceneType::Game)
    {}
    virtual ~GameScene() override {}

    void Init() override;

    Scene::SceneType Update(float delta_time) override;

    std::list<std::weak_ptr<Enemy>>& GetEnemyObjects() noexcept {
        return m_wpEnemyObjects;
    }
    const std::list<std::weak_ptr<Enemy>>& GetEnemyObjects() const noexcept {
        return m_wpEnemyObjects;
    }
    std::list<std::weak_ptr<DynamicObject>>& GetDynamicObjects() noexcept {
        return m_wpDynamicObjects;
    }
    const std::list<std::weak_ptr<DynamicObject>>& GetDynamicObjects() const noexcept {
        return m_wpDynamicObjects;
    }
    
    void AddEnemyObject(const std::shared_ptr<Enemy>& game_object, bool init = true);
    void AddDynamicObject(const std::shared_ptr<DynamicObject>& game_object, bool init = true);
    
private:
    
    std::list<std::weak_ptr<Enemy>> m_wpEnemyObjects;
    std::list<std::weak_ptr<DynamicObject>> m_wpDynamicObjects;

};
