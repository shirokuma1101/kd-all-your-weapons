#include "GameUI.h"

#include "Application/Game/Scene/GameScene/GameScene.h"

void GameUI::Init()
{
    m_backgroundTexture.Load("Asset/Texture/ui/title/title.png");
    m_loadTexture.Load("Asset/Texture/ui/game/load-icon.png");
    m_reticleTexture.Load("Asset/Texture/ui/game/reticle.png");
}

void GameUI::Update(float delta_time)
{
    static float time = 0.f;
    if (m_isLoading) {
        time += delta_time;
        m_loadTextureAngle += 360.f * delta_time;
        if (time > 22.f) { // 22sec
            m_isLoading = false;
            time = 0.f;
        }
    }

    // SceneがGameの場合はGameSceneにキャストする
    if (Application::Instance().GetGameSystem()->GetScene()->GetSceneType() != Scene::SceneType::Game) return;
    // GameSceneと確定しているのでstatic
    auto game_scene = std::static_pointer_cast<GameScene>(Application::WorkInstance().GetGameSystem()->GetScene());

    if (!game_scene->GetEnemyObjects().size()) {
        m_isObjectAlive = false;
    }
}

void GameUI::DrawSprite()
{
    if (m_isLoading) {
        auto p = GetPositionFromPercentage(85, 80);
        DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().DrawTex(m_backgroundTexture, Math::Matrix::CreateScale(GetSizePercentageForHDWidthBased()), { 1280, 720 });
        DirectX11System::WorkInstance().GetShaderManager()->GetSpriteShader().DrawTex(
            m_loadTexture,
            Math::Matrix::CreateScale(0.25f * GetSizePercentageForHDWidthBased())
            * Math::Matrix::CreateRotationZ(convert::ToRadians(m_loadTextureAngle))
            * Math::Matrix::CreateTranslation(p.x, p.y, 0)
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
    if (m_isLoading) return;

    auto sp_player = m_wpPlayer.lock();

    float weight_limit = sp_player->GetEquipWeightLimit();
    DirectX11System::WorkInstance().GetShaderManager()->GetSpriteFont().Draw(
        "cyberspace",
        fmt::format("Weight Limit: {}", weight_limit),
        GetPositionFromPercentage(0, 10),
        false,
        directx11_helper::white,
        0.5f * GetSizePercentageForHDWidthBased()
    );
    if (float weight = sp_player->GetSelectedObjectWeight(); weight) {
        DirectX11System::WorkInstance().GetShaderManager()->GetSpriteFont().Draw(
            "cyberspace",
            fmt::format("{}", weight),
            GetPositionFromPercentage(50, 10),
            true,
            weight_limit >= weight ? directx11_helper::green : directx11_helper::red,
            GetSizePercentageForHDWidthBased()
        );
    }
}
