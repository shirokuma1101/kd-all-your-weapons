#pragma once

#include "Application/Game/GameObject/UIObject/UIObject.h"

class TitleUI : public UIObject
{
public:

    enum class SelectMenu {
        Start,
        Settings,
        Exit,
    };

    void Init() override;
    void Update(float delta_time) override;
    void DrawSprite() override;
    void DrawFont() override;

private:

    float      m_cursorAlpha = 0.f;
    SelectMenu m_selectMenu  = SelectMenu::Start;

    DirectX11TextureSystem m_titleTexture;
    
};
