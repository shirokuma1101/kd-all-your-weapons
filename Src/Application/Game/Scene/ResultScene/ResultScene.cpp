#include "ResultScene.h"

#include "Application/Game/GameObject/UIObject/ResultUI/ResultUI.h"

void ResultScene::Init()
{
    input_helper::CursorData::ShowCursor(true);
    
    auto result_ui = std::make_shared<ResultUI>();
    AddGameObject(result_ui);

    m_nextSceneType = SceneType::Title;
    m_wpDeletionDecisionObject = result_ui;
}
