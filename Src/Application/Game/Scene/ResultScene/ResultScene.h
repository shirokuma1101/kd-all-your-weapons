#pragma once

#include "Application/Game/Scene/Scene.h"

class ResultScene : public Scene
{
public:
    
    ResultScene()
        : Scene(SceneType::Result)
    {}
    virtual ~ResultScene() override {}

    void Init() override;
    
};
