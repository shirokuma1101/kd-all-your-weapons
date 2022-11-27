#include "ResultUI.h"

void ResultUI::Init()
{
    m_resultTexture.Load("Asset/Texture/ui/result/result.png");
}

void ResultUI::Update(float)
{
    auto& km = Application::Instance().GetGameSystem()->GetInputManager()->GetKeyManager();

    if (km->GetState(VK_LBUTTON, KeyManager::KEYSTATE_PRESS)) {
        m_isObjectAlive = false;
    }
}

void ResultUI::DrawSprite()
{
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().DrawTex(m_resultTexture, Math::Matrix::CreateScale(GetSizePercentageForHDWidthBased()));
}
