#include "TitleUI.h"

void TitleUI::Init()
{
    m_titleTexture.Load("Asset/Texture/ui/title/title.png");
}

void TitleUI::Update(float delta_time)
{
    auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();
    auto& cm = Application::Instance().GetGameSystem()->GetInputManager()->GetCursorManager();

    auto cursor_pos = cm->GetPositionFromCenter();
    cursor_pos.y *= -1;

    //MN: 座標

    auto start_left_top = GetPositionFromPercentage(50, 45);
    auto start_right_bottom = GetPositionFromPercentage(95, 55);
    if (cursor_pos.x > start_left_top.x && cursor_pos.x < start_right_bottom.x &&
        cursor_pos.y < start_left_top.y && cursor_pos.y > start_right_bottom.y) {
        m_selectMenu = SelectMenu::Start;
        if (km->GetState(VK_LBUTTON, KeyManager::KEYSTATE_PRESS)) {
            m_isObjectAlive = false;
        }
    }

    //auto settings_left_top = GetPositionFromPercentage(50, 65);
    //auto settings_right_bottom = GetPositionFromPercentage(95, 75);
    //if (cursor_pos.x > settings_left_top.x && cursor_pos.x < settings_right_bottom.x &&
    //    cursor_pos.y < settings_left_top.y && cursor_pos.y > settings_right_bottom.y) {
    //    m_selectMenu = SelectMenu::Settings;
    //    if (km->GetState(VK_LBUTTON, KeyManager::KEYSTATE_PRESS)) {
    //    }
    //}

    auto exit_left_top = GetPositionFromPercentage(50, 85);
    auto exit_right_bottom = GetPositionFromPercentage(95, 95);
    if (cursor_pos.x > exit_left_top.x && cursor_pos.x < exit_right_bottom.x &&
        cursor_pos.y < exit_left_top.y && cursor_pos.y > exit_right_bottom.y) {
        m_selectMenu = SelectMenu::Exit;
        if (km->GetState(VK_LBUTTON, KeyManager::KEYSTATE_PRESS)) {
            SendMessage(Application::Instance().GetWindow().GetWindowHandle(), WM_CLOSE, 0, 0);
        }
    }

    m_cursorAlpha += delta_time;
    if (m_cursorAlpha > 1.f) {
        m_cursorAlpha = 0.f;
    }

    //Application::Instance().GetGameSystem()->GetGameSettings();
}

void TitleUI::DrawSprite()
{
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().DrawTex(m_titleTexture, Math::Matrix::CreateScale(GetSizePercentageForHDWidthBased()));

    Math::Matrix cursorMatrix;
    switch (m_selectMenu) {
    case SelectMenu::Start:
        {
            auto p = GetPositionFromPercentage(50, 50);
            cursorMatrix = Math::Matrix::CreateTranslation(p.x, p.y, 0.f);
        }
        break;
    case SelectMenu::Settings:
        {
            auto p = GetPositionFromPercentage(50, 70);
            cursorMatrix = Math::Matrix::CreateTranslation(p.x, p.y, 0.f);
        }
        break;
    case SelectMenu::Exit:
        {
            auto p = GetPositionFromPercentage(50, 90);
            cursorMatrix = Math::Matrix::CreateTranslation(p.x, p.y, 0.f);
        }
        break;
    }
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().DrawTriangle(
        Math::Matrix::CreateScale(70.f * GetSizePercentageForHDWidthBased()) * cursorMatrix,
        { 0.f, 0.f }, { -1.f, 0.6f }, { -1.f, -0.6f }, { 0.f, 0.f, 0.f, m_cursorAlpha }
    );
}

void TitleUI::DrawFont()
{
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteFont().Draw("cyberspace", "Start", GetPositionFromPercentage(75.f, 50.f), true, directx11_helper::white, GetSizePercentageForHDWidthBased());
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteFont().Draw("cyberspace", "Settings", GetPositionFromPercentage(75.f, 70.f), true, { 0.5f, 0.5f, 0.5f, 1.f }, GetSizePercentageForHDWidthBased());
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteFont().Draw("cyberspace", "Exit", GetPositionFromPercentage(75.f, 90.f), true, directx11_helper::white, GetSizePercentageForHDWidthBased());
}
