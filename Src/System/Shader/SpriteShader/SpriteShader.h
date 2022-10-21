#pragma once

class SpriteShader
{
public:

    struct Vertex {
        Math::Vector3 Pos;
        Math::Vector2 UV;
    };

    SpriteShader() {}
    ~SpriteShader() {
        Release();
    }
    
    bool Init();
    
    // 変換行列セット
    void SetMatrix(const Math::Matrix& m) {
        m_cb0.Get()->mTransform = m;
    }
    
    void Begin(bool linear = false, bool disableZBuffer = true);

    void End();

    void DrawTex(
        const DirectX11Texture& tex,
        const Math::Vector2&    pos,
        const Math::Vector2&    rect,
        const Math::Rectangle*  src_rect = nullptr,
        const Math::Color&      color   = directx11_helper::white,
        const Math::Vector2&    pivot   = { 0.5, 0.5f }
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
    
    void DrawCircle(const Math::Vector2& pos, float radius, const Math::Color& color = directx11_helper::white, bool fill = true);

    void DrawBox(const Math::Vector2& pos, const Math::Vector2& extent, const Math::Color& color = directx11_helper::white, bool fill = true);

    void SetScissorRect(const Math::Rectangle& rect);

    void Release();
    
private:

    ID3D11VertexShader* m_VS      = nullptr; // 頂点シェーダー
    ID3D11InputLayout*  m_VLayout = nullptr; // 頂点レイアウト
    ID3D11PixelShader*  m_PS      = nullptr; // ピクセルシェーダー

    // 定数バッファ
    struct cbSprite {
        Math::Matrix mTransform;
        Math::Vector4 Color = { 1, 1, 1, 1 };
    };
    DirectX11ConstantBufferSystem<cbSprite> m_cb0;

    // 定数バッファ
    struct cbProjection {
        Math::Matrix mProj;
    };
    DirectX11ConstantBufferSystem<cbProjection> m_cb1;
    
    // 2D用正射影行列
    Math::Matrix m_mProj2D;

    // 使用するステート
    ID3D11DepthStencilState* m_ds          = nullptr;
    ID3D11RasterizerState*   m_rs          = nullptr;
    ID3D11SamplerState*      m_smp0_Point  = nullptr;
    ID3D11SamplerState*      m_smp0_Linear = nullptr;

    // ステート記憶/復元用
    struct SaveState {
        ID3D11DepthStencilState* DS         = nullptr;
        ID3D11RasterizerState*   RS         = nullptr;
        UINT                     StencilRef = 0;
        ID3D11SamplerState*      Smp0       = nullptr;
    };
    SaveState m_saveState;
    
};
