#pragma once

#include "Application/Game/GameObject/UIObject/UIObject.h"

class ResultUI : public UIObject
{
public:

    void Init() override;
    void Update(float delta_time) override;
    void DrawSprite() override;

private:
    
    DirectX11TextureSystem m_resultTexture;

};
