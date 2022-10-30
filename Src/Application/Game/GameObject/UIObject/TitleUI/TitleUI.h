#pragma once

#include "Application/Game/GameObject/UIObject/UIObject.h"

class TitleUI : public UIObject
{
public:

    void Init() override;

    void Update(float delta_time) override;

    void DrawSprite() override;

    void DrawFont() override;

private:

    float m_cursorAlpha = 0.f;
    DirectX11TextureSystem m_titleTexture;

    DirectX11RenderTargetSystem m_rt;
    DirectX11RenderTargetChangerSystem m_rtc;
    
};
