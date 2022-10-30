#pragma once

#include "StandardShader/StandardShader.h"
#include "SpriteShader/SpriteShader.h"

class ShaderManager
{
public:

    ShaderManager() {}
    ~ShaderManager() {
        Release();
    }

    void Init();

    void Release();

    auto& GetStandardShader() noexcept {
        return m_standardShader;
    }
    auto& GetStandardShader() const noexcept {
        return m_standardShader;
    }
    auto& GetSpriteShader() noexcept {
        return m_spriteShader;
    }
    auto& GetSpriteShader() const noexcept {
        return m_spriteShader;
    }
    auto& GetSpriteFont() noexcept {
        return m_spriteFont;
    }
    auto& GetSpriteFont() const noexcept {
        return m_spriteFont;
    }
    auto& GetCameraCB() noexcept {
        return m_cameraCB;
    }
    auto& GetCameraCB() const noexcept {
        return m_cameraCB;
    }
    auto& GetLightCB() noexcept {
        return m_lightCB;
    }
    auto& GetLightCB() const noexcept {
        return m_lightCB;
    }

    void SetToDevice();

    bool SetVertexShader(ID3D11VertexShader* vs);
    bool SetPixelShader(ID3D11PixelShader* ps);

    /* 深度ステンシル */
    void ChangeDepthStencilState(ID3D11DepthStencilState* dss);
    void UndoDepthStencilState();

    /* ブレンドステート */
    void ChangeBlendState(ID3D11BlendState* bs);
    void UndoBlendState();

    /* ラスタライザステート */
    void ChangeRasterizerState(ID3D11RasterizerState* rs);
    void UndoRasterizerState();

    /* サンプラーステート */
    void ChangeSamplerState(int slot, ID3D11SamplerState* ss);
    void UndoSamplerState();

private:

    // カメラ定数バッファ
    struct CameraCB {
        Math::Matrix  view;       // 64byte: ビュー行列
        Math::Matrix  projection; // 64byte: 射影行列
        Math::Vector3 position;   // 12byte: カメラのワールド座標

        int           distanceFogEnable = 0;                 //  4byte: 距離フォグ有効
        Math::Vector3 distanceFogColor  = { 1.f, 1.f, 1.f }; // 12byte: 距離フォグ色
        float         distanceFogStart  = 0.f;               //  4byte: 距離フォグ開始距離
        float         distanceFogEnd    = 0.f;               //  4byte: 距離フォグ終了距離
        DIRECTX11_HELPER_PADDING_12BYTE(0);                  // 12byte: パディング0
    }; // 164byte

    // ライト定数バッファ
    static constexpr int POINT_LIGHT_MAX = 8;
    struct LightCB {
        Math::Vector3 ambientLight              = { 0.3f, 0.3f, 0.3f };  // 12byte: 環境光
        DIRECTX11_HELPER_PADDING_4BYTE(0);                               //  4byte: パディング0

        Math::Vector3 directionalLightDirection = { -1.f, -1.f, 1.f };   // 12byte: 平行光の方向
        DIRECTX11_HELPER_PADDING_4BYTE(1);                               //  4byte: パディング1
        Math::Vector3 directionalLightColor     = { 1.f, 1.f, 1.f };     // 12byte: 平行光の色
        DIRECTX11_HELPER_PADDING_4BYTE(2);                               //  4byte: パディング2
        Math::Matrix  directionalLightVP;                                // 64byte: 平行光のビュー射影行列

        int pointLightCount = 0;                                         //  4byte: ポイントライト数
        DIRECTX11_HELPER_PADDING_12BYTE(3);                              // 12byte: パディング3
        struct PointLight {
            Math::Vector3 position;                                      // 12byte: 点光源のワールド座標
            DIRECTX11_HELPER_PADDING_4BYTE(4);                           //  4byte: パディング4
            Math::Vector3 color;                                         // 12byte: 点光源の色
            DIRECTX11_HELPER_PADDING_4BYTE(5);                           //  4byte: パディング5
            Math::Vector3 attenuation = { 0.f, 1.f, 0.f };               // 12byte: 点光源の減衰率 (一定減衰、線形減衰、2次減衰)
            DIRECTX11_HELPER_PADDING_4BYTE(6);                           //  4byte: パディング6
        }; // 48byte
        std::array<PointLight, POINT_LIGHT_MAX> pointLight;              // 384byte: 点光源
    }; // 512byte

    StandardShader m_standardShader;
    SpriteShader   m_spriteShader;
    SpriteFont     m_spriteFont; // シェーダーではない

    DirectX11ConstantBufferSystem<CameraCB> m_cameraCB;
    DirectX11ConstantBufferSystem<LightCB>  m_lightCB;

public:

    /* 深度ステンシルステート */
    ID3D11DepthStencilState* m_pDssEnableDepthEnableWriteDepth   = nullptr; // 深度バッファの利用ON、 深度に書き込みON
    ID3D11DepthStencilState* m_pDssEnableDepthDisableWriteDepth  = nullptr; // 深度バッファの利用ON、 深度に書き込みOFF
    ID3D11DepthStencilState* m_pDssDisableDepthDisableWriteDepth = nullptr; // 深度バッファの利用OFF、深度に書き込みOFF
    ID3D11DepthStencilState* m_pDssUndo                          = nullptr; // 元に戻す

    /* ブレンドステート */
    ID3D11BlendState*        m_pBSAlpha                          = nullptr; // 透明度による色合成
    ID3D11BlendState*        m_pBSAdd                            = nullptr; // 加算による色合成
    ID3D11BlendState*        m_pBSUndo                           = nullptr; // 元に戻す

    /* ラスタライズステート */
    ID3D11RasterizerState*   m_pRSCullNone                       = nullptr; // 描画カリングモード：描画省略なし
    ID3D11RasterizerState*   m_pRSCullBack                       = nullptr; // 描画カリングモード：裏面の描画省略
    ID3D11RasterizerState*   m_pRSUndo                           = nullptr; // 元に戻す

    /* サンプラーステート */
    ID3D11SamplerState*      m_pSSPointWrap                      = nullptr; // ポイントサンプリング   : テクスチャの端を固定する
    ID3D11SamplerState*      m_pSSLinearClamp                    = nullptr; // バイリニアサンプリング : テクスチャの端を繋げて繰り返す
    ID3D11SamplerState*      m_pSSLinearClampComp                = nullptr; // バイリニアサンプリング : テクスチャの端を繋げて繰り返す・比較機能ON
    ID3D11SamplerState*      m_pSSAnisotropicWrap                = nullptr; // 異方性フィルタリング   : テクスチャの端を固定する
    ID3D11SamplerState*      m_pSSAnisotropicClamp               = nullptr; // 異方性フィルタリング   : テクスチャの端を繋げて繰り返す
    ID3D11SamplerState*      m_pSSUndo                           = nullptr; // 元に戻す

};
