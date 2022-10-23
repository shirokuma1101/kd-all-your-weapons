#include "GameUI.h"

void GameUI::Init()
{
    m_backgroundTexture.Load("Asset/Texture/kari/title.png");
    m_loadTexture.Load("Asset/Texture/kari/load-icon-png-7952.png");
    m_reticleTexture.Load("Asset/Texture/kari/reticle.png");
}

void GameUI::Update(float delta_time)
{
    auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();

    if (km->GetState(VK_F4, KeyManager::KeyState::Hold)) {
        m_isObjectAlive = false;
    }
    static float time = 0.f;
    if (m_isLoading) {
        time += delta_time;
        m_loadTextureAngle += 360.f * delta_time;
        if (time > 15.f) { // 20sec
            m_isLoading = false;
            time = 0.f;
        }
    }
}

void GameUI::DrawSprite()
{
    if (m_isLoading) {
        DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().SetMatrix(Math::Matrix::Identity);
        //DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().DrawTex(m_backgroundTexture, { 0, 0 }, { 1280, 720 });
        DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().SetMatrix(
            Math::Matrix::CreateScale(0.25f) * Math::Matrix::CreateRotationZ(convert::ToRadians(m_loadTextureAngle)) * Math::Matrix::CreateTranslation(450.f, -250.f, 0)
        );
        DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().DrawTex(m_loadTexture, { 0, 0 }, { 512, 512 });
    }
    else {
        DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().SetMatrix(Math::Matrix::CreateScale(0.025f));
        DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().DrawTex(m_reticleTexture, { 0, 0 }, { 500, 500 }, nullptr, { 0.f, 0.6f, 0.8f, 1.f });
    }
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().SetMatrix(Math::Matrix::Identity);
}

void GameUI::DrawFont()
{
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteFont().Draw("genkai", "genkai", GetPositionFromPercentage(10.f, 5.f), true, directx11_helper::white, GetSizePercentageForHDWidthBased());
}
