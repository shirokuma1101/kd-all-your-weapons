#include "TitleScene.h"

#include "Application/Game/GameObject/UIObject/TitleUI/TitleUI.h"

void TitleScene::Init()
{
    input_helper::CursorData::ShowCursor(true);

    auto title_ui = std::make_shared<TitleUI>();
    AddGameObject(title_ui);

    auto& am = Application::Instance().GetGameSystem()->GetAudioManager();
    if (!am->GetSoundInstance("aki_bgm")) {
        auto bgm_inst = am->Play("aki_bgm", AudioManager::PLAYFLAGS_LOOP);
    }

    m_nextSceneType = SceneType::Game;
    m_wpDeletionDecisionObject = title_ui;
}
