#include "TitleUI.h"

void TitleUI::Init()
{
    m_titleTexture.Load("Asset/Texture/kari/title_1.png");
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
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().SetMatrix(Math::Matrix::Identity);
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().DrawTex(m_titleTexture, { 0, 0 }, { 1280, 720 });
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().SetMatrix(
        Math::Matrix::CreateScale(70.f) * Math::Matrix::CreateTranslation(180.f, -30.f, 0.f)
    );
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().DrawTriangle(
        { 0.f, 0.f }, { -1.f, 0.6f }, { -1.f, -0.6f }, { 0.f, 0.f, 0.f, m_cursorAlpha }
    );
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().SetMatrix(Math::Matrix::Identity);
}
