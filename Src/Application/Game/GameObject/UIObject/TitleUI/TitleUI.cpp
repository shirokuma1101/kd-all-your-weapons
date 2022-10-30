#include "TitleUI.h"

void TitleUI::Init()
{
    m_titleTexture.Load("Asset/Texture/kari/title_1.png");
    m_rt.Create({ 640, 360 });
}

void TitleUI::Update(float delta_time)
{
    auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();
    
    if (km->GetState(VK_RETURN, KeyManager::KeyState::Hold)) {
        m_isObjectAlive = false;
    }

    m_cursorAlpha += delta_time;
    if (m_cursorAlpha > 1.f) {
        m_cursorAlpha = 0.f;
    }
}

void TitleUI::DrawSprite()
{
    m_rtc.Change(&m_rt);
    //DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().DrawTex(m_titleTexture, Math::Matrix::CreateScale(GetSizePercentageForHDWidthBased()));
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().DrawTex(m_titleTexture);
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().DrawTriangle(
        Math::Matrix::CreateScale(70.f) * Math::Matrix::CreateTranslation(180.f, -30.f, 0.f),
        { 0.f, 0.f }, { -1.f, 0.6f }, { -1.f, -0.6f }, { 0.f, 0.f, 0.f, m_cursorAlpha }
    );
    m_rtc.Undo();
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().DrawTex(*m_rt.GetBackBuffer());
}

void TitleUI::DrawFont()
{
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteFont().Draw("genkai", "Start", GetPositionFromPercentage(10.f, 10.f), true, directx11_helper::white, GetSizePercentageForHDWidthBased());
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteFont().Draw("genkai", "Settings", GetPositionFromPercentage(10.f, 15.f), true, directx11_helper::white, GetSizePercentageForHDWidthBased());
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteFont().Draw("genkai", "Exit", GetPositionFromPercentage(10.f, 20.f), true, directx11_helper::white, GetSizePercentageForHDWidthBased());
}
