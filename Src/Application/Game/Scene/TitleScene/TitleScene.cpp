﻿#include "TitleScene.h"

#include "Application/Game/GameObject/UIObject/TitleUI/TitleUI.h"

void TitleScene::Init()
{
    auto title_ui = std::make_shared<TitleUI>();
    AddGameObject(title_ui);

    m_nextSceneType = SceneType::Game;
    m_wpDeletionDecisionObject = title_ui;
}
