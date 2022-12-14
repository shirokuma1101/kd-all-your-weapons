#pragma once

#include "Application/Game/GameObject/UIObject/UIObject.h"
#include "Application/Game/GameObject/ModelObject/CharacterObject/Player/Player.h"

class GameUI : public UIObject
{
public:

    void Init() override;
    void Update(float delta_time) override;
    void DrawSprite() override;
    void DrawFont() override;

    void SetPlayer(std::shared_ptr<Player>& player) {
        m_wpPlayer = player;
    }

private:

    bool  m_isLoading        = true;
    float m_loadTextureAngle = 0.f;

    std::weak_ptr<Player> m_wpPlayer;

    DirectX11TextureSystem m_loadTexture;
    DirectX11TextureSystem m_backgroundTexture;
    DirectX11TextureSystem m_reticleTexture;
    
};
