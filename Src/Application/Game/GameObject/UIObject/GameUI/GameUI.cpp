#include "GameUI.h"

void GameUI::Init()
{
    m_backgroundTexture.Load("Asset/Texture/ui/title/title_loading.png");
    m_loadTexture.Load("Asset/Texture/ui/game/load-icon.png");
    m_reticleTexture.Load("Asset/Texture/ui/game/reticle.png");
}

void GameUI::Update(float delta_time)
{
    auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();

    if (km->GetState(VK_F4, KeyManager::KEYSTATE_HOLD)) {
        m_isObjectAlive = false;
    }
    static float time = 0.f;
    if (m_isLoading) {
        time += delta_time;
        m_loadTextureAngle += 360.f * delta_time;
        if (time > 15.f) { // 15sec
            m_isLoading = false;
            time = 0.f;
        }
    }
}

void GameUI::DrawSprite()
{
    if (m_isLoading) {
        auto p = GetPositionFromPercentage(85, 80);
        DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().DrawTex(m_backgroundTexture, Math::Matrix::CreateScale(GetSizePercentageForHDWidthBased()), {1280, 720});
        DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().DrawTex(
            m_loadTexture,
            Math::Matrix::CreateScale(0.25f)
            * Math::Matrix::CreateRotationZ(convert::ToRadians(m_loadTextureAngle))
            * Math::Matrix::CreateTranslation(p.x, p.y, 0), { 512, 512 }
        );
    }
    else {
        DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().DrawTex(
            m_reticleTexture,
            Math::Matrix::CreateScale(0.025f), { 0.f, 0.6f, 0.8f, 1.f }
        );
    }
}

void GameUI::DrawFont()
{
    //DirectX11System::WorkInstance().GetShaderManager()->GetSpriteFont().Draw("genkai", "Weight Limit", GetPositionFromPercentage(15.f, 5.f), true, directx11_helper::white, GetSizePercentageForHDWidthBased());
    //DirectX11System::WorkInstance().GetShaderManager()->GetSpriteFont().Draw("genkai", "Selected Weight", GetPositionFromPercentage(15.f, 10.f), true, directx11_helper::white, GetSizePercentageForHDWidthBased());
}
