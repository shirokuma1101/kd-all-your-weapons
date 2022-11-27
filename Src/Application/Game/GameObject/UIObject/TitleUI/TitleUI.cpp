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

    auto start_left_top = GetPositionFromPercentage(70, 50);
    auto start_right_bottom = GetPositionFromPercentage(90, 60);
    if (cursor_pos.x > start_left_top.x && cursor_pos.x < start_right_bottom.x &&
        cursor_pos.y < start_left_top.y && cursor_pos.y > start_right_bottom.y) {
        m_selectMenu = SelectMenu::Start;
        if (km->GetState(VK_LBUTTON, KeyManager::KEYSTATE_PRESS)) {
            m_isObjectAlive = false;
        }
    }

    auto exit_left_top = GetPositionFromPercentage(70, 80);;
    auto exit_right_bottom = GetPositionFromPercentage(90, 90);
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
            auto p = GetPositionFromPercentage(65, 55);
            cursorMatrix = Math::Matrix::CreateTranslation(p.x, p.y, 0.f);
        }
        break;
    case SelectMenu::Exit:
        {
            auto p = GetPositionFromPercentage(65, 80);
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
    //DirectX11System::WorkInstance().GetShaderManager()->GetSpriteFont().Draw("genkai", "Start", GetPositionFromPercentage(10.f, 10.f), true, directx11_helper::white, GetSizePercentageForHDWidthBased());
    //DirectX11System::WorkInstance().GetShaderManager()->GetSpriteFont().Draw("genkai", "Settings", GetPositionFromPercentage(10.f, 15.f), true, directx11_helper::white, GetSizePercentageForHDWidthBased());
    //DirectX11System::WorkInstance().GetShaderManager()->GetSpriteFont().Draw("genkai", "Exit", GetPositionFromPercentage(10.f, 20.f), true, directx11_helper::white, GetSizePercentageForHDWidthBased());
}
