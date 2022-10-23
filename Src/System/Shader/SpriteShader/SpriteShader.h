#pragma once

#include "Math/Convert.h"

class SpriteShader
{
public:

    struct Vertex {
        Math::Vector3 pos;
        Math::Vector2 uv;
    };

    SpriteShader() {}
    ~SpriteShader() {
        Release();
    }

    bool Init();

    void Release();

    void SetMatrix(const Math::Matrix& matrix) {
        m_spriteCB.Get()->transform = matrix;
    }

    void SetToDevice();

    void DrawTex(
        const DirectX11Texture& tex,
        const Math::Vector2&    pos,
        const Math::Vector2&    rect,
        const Math::Rectangle*  src_rect = nullptr,
        const Math::Color&      color    = directx11_helper::white,
        const Math::Vector2&    pivot    = { 0.5, 0.5f }
    );

    void DrawTex(
        const DirectX11Texture& tex,
        const Math::Vector2&    pos,
        const Math::Rectangle*  src_rect = nullptr,
        const Math::Color&      color    = directx11_helper::white,
        const Math::Vector2&    pivot    = { 0.5, 0.5f }
    ) {
        DrawTex(tex, pos, { static_cast<float>(tex.GetTextureDesc().Width), static_cast<float>(tex.GetTextureDesc().Height) }, src_rect, color, pivot);
    }

    void DrawPoint(const Math::Vector2& pos, const Math::Color& color = directx11_helper::white);

    void DrawLine(const Math::Vector2& pos1, const Math::Vector2& pos2, const Math::Color& color = directx11_helper::white);

    void DrawTriangle(const Math::Vector2& pos1, const Math::Vector2& pos2, const Math::Vector2& pos3, const Math::Color& color = directx11_helper::white, bool fill = true);

    void DrawBox(const Math::Vector2& pos, const Math::Vector2& extent, const Math::Color& color = directx11_helper::white, bool fill = true);

    void DrawCircle(const Math::Vector2& pos, float radius, const Math::Color& color = directx11_helper::white, bool fill = true);

private:

    struct SpriteCB {
        Math::Matrix  transform;                      // 64byte: ワールドビュープロジェクション行列
        Math::Vector4 color = { 1.f, 1.f, 1.f, 1.f }; // 16byte: 色
    }; // 80byte

    struct ProjectionCB {
        Math::Matrix projection; // 64byte: プロジェクション行列
    }; // 64byte

    DirectX11ConstantBufferSystem<SpriteCB>     m_spriteCB;
    DirectX11ConstantBufferSystem<ProjectionCB> m_projectionCB;

    ID3D11VertexShader* m_pVS          = nullptr;
    ID3D11InputLayout*  m_pInputLayout = nullptr;
    ID3D11PixelShader*  m_pPS          = nullptr;
    
};

class SpriteFont
{
public:

    SpriteFont() {}
    ~SpriteFont() {
        Release();
    }

    bool Init();

    void Draw(std::string_view font_name, std::string_view text, const Math::Vector2& pos = { 0.f, 0.f }, bool align_center = true, const Math::Color& color = directx11_helper::white, float scale = 1.f);

    void Release() {}

    void Begin();

    void End();

    void AddFont(std::string_view font_name, std::string_view file_path) {
        auto wtext = sjis_to_wide(file_path.data());
        m_upSpriteFonts.emplace(font_name, std::make_unique<DirectX::SpriteFont>(DirectX11System::Instance().GetDev().Get(), wtext.data()));
    }

    Math::Vector2 MeasureString(std::string_view font_name, std::string_view text) {
        // フォントデータを取得
        if (auto iter = m_upSpriteFonts.find(font_name.data()); iter != m_upSpriteFonts.end()) {
            // 文字列を作成
            auto wtext = sjis_to_wide(text.data());
            // 中心座標を計算
            return iter->second->MeasureString(wtext.data());
        }
        return Math::Vector2::Zero;
    }
    
private:

    std::unique_ptr<DirectX::SpriteBatch>                                 m_upSpriteBatch; // フォント表示用
    std::unordered_map<std::string, std::unique_ptr<DirectX::SpriteFont>> m_upSpriteFonts; // フォントデータ

    struct SaveState {
        ID3D11BlendState*        pBlendState        = nullptr;
        float                    factor[4]          = {};
        UINT                     mask               = 0;
        ID3D11SamplerState*      pSamplerState      = nullptr;
        ID3D11DepthStencilState* pDepthStencilState = nullptr;
        UINT                     stencilRef         = 0;
        ID3D11RasterizerState*   pRasterizerState   = nullptr;
    } m_saveState;
    
};
