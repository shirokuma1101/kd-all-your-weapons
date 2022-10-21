#pragma once

#include "Application/Game/Scene/Scene.h"

class TitleScene : public Scene
{
public:
    
    TitleScene()
        : Scene(SceneType::Title)
    {}
    virtual ~TitleScene() override {}

    void Init() override;

};
