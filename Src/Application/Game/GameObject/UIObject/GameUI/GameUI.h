#pragma once

#include "Application/Game/GameObject/UIObject/UIObject.h"

class GameUI : public UIObject
{
public:

    void Init() override;

    void Update(float delta_time) override;

    void DrawSprite();

private:

    bool m_isLoading = true;
    float m_loadTextureAngle = 0.f;

    DirectX11TextureSystem m_loadTexture;
    DirectX11TextureSystem m_backgroundTexture;
    DirectX11TextureSystem m_reticleTexture;
    
};
