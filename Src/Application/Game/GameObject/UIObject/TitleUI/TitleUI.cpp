#include "TitleUI.h"

void TitleUI::Init()
{
    m_titleTexture.Load("Asset/Texture/ui/title/title.png");
}

void TitleUI::Update(float delta_time)
{
    auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();

    if (km->GetState(VK_UP, KeyManager::KEYSTATE_PRESS)) {
        m_selectMenu = SelectMenu::Start;
    }
    else if (km->GetState(VK_DOWN, KeyManager::KEYSTATE_PRESS)) {
        m_selectMenu = SelectMenu::Exit;
    }

    if (km->GetState(VK_RETURN, KeyManager::KEYSTATE_HOLD)) {
        switch (m_selectMenu) {
        case SelectMenu::Start:
            m_isObjectAlive = false;
            break;
        case SelectMenu::Exit:
            SendMessage(Application::Instance().GetWindow().GetWindowHandle(), WM_CLOSE, 0, 0);
            break;
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
