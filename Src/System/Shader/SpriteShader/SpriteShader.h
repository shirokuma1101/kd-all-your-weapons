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

    bool Init() {
        m_upSpriteBatch = std::make_unique<DirectX::SpriteBatch>(DirectX11System::Instance().GetCtx().Get());
    }

    void Draw(
        std::string_view     font_name,
        std::string_view     text,
        const Math::Vector2& pos,
        bool                 align_center = false,
        const Math::Color&   color        = directx11_helper::white,
        float                scale        = 1.f
    ) {
        // フォントデータを取得
        const auto& font = m_upSpriteFonts.at(font_name.data());
        // 文字列を作成
        auto wtext = sjis_to_wide(text.data());
        // 中心座標を計算
        Math::Vector2 origin{};
        if (align_center) {
            origin = font->MeasureString(wtext.data());
            convert::ToHalf(origin);
        }

        // Viewportから補正値を計算
        D3D11_VIEWPORT vp{};
        UINT num_viewport = 1;
        DirectX11System::Instance().GetCtx()->RSGetViewports(&num_viewport, &vp);
        Math::Vector2 offset_pos {
            pos.x + convert::ToHalf(vp.Width),
            -pos.y + convert::ToHalf(vp.Height),
        };

        font->DrawString(m_upSpriteBatch.get(), wtext.data(), offset_pos, color, 0.f, origin, scale);
    }

    void Release() {}

    void Begin() {
        DirectX11System::Instance().GetCtx()->OMGetBlendState(&m_pOldBlendState, m_oldFactor, &m_oldMask);
        m_upSpriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_pOldBlendState);
    }

    void End() {
        m_upSpriteBatch->End();
        DirectX11System::Instance().GetCtx()->OMSetBlendState(m_pOldBlendState, m_oldFactor, m_oldMask);
        memory::SafeRelease(&m_pOldBlendState);
    }

    void AddFont(std::string_view font_name, std::string_view file_path) {
        auto wtext = sjis_to_wide(file_path.data());
        m_upSpriteFonts.emplace(font_name, std::make_unique<DirectX::SpriteFont>(DirectX11System::Instance().GetDev().Get(), wtext.data()));
    }

    Math::Vector2 MeasureString(std::string_view font_name, std::string_view text) {
        // フォントデータを取得
        DirectX::SpriteFont* font = m_upSpriteFonts.at(font_name.data()).get();
        // 文字列を作成
        auto wtext = sjis_to_wide(text.data());
        // 中心座標を計算
        return font->MeasureString(wtext.data());
    }
    
private:

    std::unique_ptr<DirectX::SpriteBatch>                                 m_upSpriteBatch; // フォント表示用
    std::unordered_map<std::string, std::unique_ptr<DirectX::SpriteFont>> m_upSpriteFonts; // フォントデータ

    ID3D11BlendState* m_pOldBlendState = nullptr;
    FLOAT             m_oldFactor[4]   = {};
    UINT              m_oldMask        = 0;
    
};
