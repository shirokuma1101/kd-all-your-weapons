#include "ShaderManager.h"

void ShaderManager::Init()
{
    auto dev = DirectX11System::Instance().GetDev().Get();
    auto ctx = DirectX11System::Instance().GetCtx().Get();
    
    /**************************************************
    * シェーダー
    **************************************************/
    
    m_standardShader.Init();
    m_spriteShader.Init();
    m_spriteFont.Init();


    /**************************************************
    * 定数バッファ
    **************************************************/

    /* カメラ */
    m_cameraCB.Create();
    
    /* ライト */
    m_lightCB.Create();

    
    /**************************************************
    * パイプラインステート
    **************************************************/
    
    /* 深度ステンシルステート作成 */
    m_pDssEnableDepthEnableWriteDepth   = directx11_helper::CreateDepthStencilState(dev, true, true);
    m_pDssEnableDepthDisableWriteDepth  = directx11_helper::CreateDepthStencilState(dev, true, false);
    m_pDssDisableDepthDisableWriteDepth = directx11_helper::CreateDepthStencilState(dev, false, false);
    ctx->OMSetDepthStencilState(m_pDssEnableDepthEnableWriteDepth, 0);

    /* ブレンドステート作成 */
    m_pBSAlpha                          = directx11_helper::CreateBlendState(dev, directx11_helper::BlendMode::Alpha);
    m_pBSAdd                            = directx11_helper::CreateBlendState(dev, directx11_helper::BlendMode::Add);
    ctx->OMSetBlendState(m_pBSAlpha, Math::Color(0, 0, 0, 0), 0xFFFFFFFF);

    /* ラスタライズステート作成 */
    m_pRSCullNone                       = directx11_helper::CreateRasterizerState(dev, D3D11_FILL_SOLID, D3D11_CULL_NONE, true, false);
    m_pRSCullBack                       = directx11_helper::CreateRasterizerState(dev, D3D11_FILL_SOLID, D3D11_CULL_BACK, true, false);
    ctx->RSSetState(m_pRSCullBack);

    /* サンプラーステート作成 */
    m_pSSPointWrap                      = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::Point, 0, directx11_helper::SamplerAddressMode::Wrap, false);
    m_pSSLinearClamp                    = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::Linear, 0, directx11_helper::SamplerAddressMode::Clamp, false);
    m_pSSLinearClampComp                = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::Linear, 0, directx11_helper::SamplerAddressMode::Clamp, true);
    m_pSSAnisotropicWrap                = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::Anisotropic, 4, directx11_helper::SamplerAddressMode::Wrap, false);
    m_pSSAnisotropicClamp               = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::Anisotropic, 4, directx11_helper::SamplerAddressMode::Clamp, false);
    ctx->PSSetSamplers(0, 1, &m_pSSAnisotropicWrap);
}

void ShaderManager::Release()
{
    m_standardShader.Release();
    m_spriteShader.Release();
    m_spriteFont.Release();

    m_cameraCB.Release();
    m_lightCB.Release();

    /* 深度ステンシルステート解放 */
    memory::SafeRelease(&m_pDssEnableDepthEnableWriteDepth);
    memory::SafeRelease(&m_pDssEnableDepthDisableWriteDepth);
    memory::SafeRelease(&m_pDssDisableDepthDisableWriteDepth);
    m_pDssUndo = nullptr;

    /* ブレンドステート解放 */
    memory::SafeRelease(&m_pBSAlpha);
    memory::SafeRelease(&m_pBSAdd);
    m_pBSUndo = nullptr;

    /* ラスタライズステート解放 */
    memory::SafeRelease(&m_pRSCullNone);
    memory::SafeRelease(&m_pRSCullBack);
    m_pRSUndo = nullptr;

    /* サンプラーステート解放 */
    memory::SafeRelease(&m_pSSPointWrap);
    memory::SafeRelease(&m_pSSLinearClamp);
    memory::SafeRelease(&m_pSSLinearClampComp);
    memory::SafeRelease(&m_pSSAnisotropicWrap);
    memory::SafeRelease(&m_pSSAnisotropicClamp);
    m_pSSUndo = nullptr;
}

void ShaderManager::SetToDevice()
{
    auto camera = m_cameraCB.Get();
    auto light = m_lightCB.Get();
    light->directionalLightDirection.Normalize();
    light->directionalLightVP = DirectX::XMMatrixLookAtLH(camera->position - light->directionalLightDirection * 40, camera->position, Math::Vector3::Up) * DirectX::XMMatrixOrthographicLH(50, 50, 0, 100);
    m_lightCB.Write();
    
    DirectX11System::Instance().GetCtx().Get()->VSSetConstantBuffers(0, 1, m_cameraCB.GetBufferAddress());
    DirectX11System::Instance().GetCtx().Get()->PSSetConstantBuffers(0, 1, m_cameraCB.GetBufferAddress());
    DirectX11System::Instance().GetCtx().Get()->VSSetConstantBuffers(1, 1, m_lightCB.GetBufferAddress());
    DirectX11System::Instance().GetCtx().Get()->PSSetConstantBuffers(1, 1, m_lightCB.GetBufferAddress());
}

bool ShaderManager::SetVertexShader(ID3D11VertexShader* vs)
{
    if (!vs) {
        return false;
    }

    ID3D11VertexShader* now_vs = nullptr;
    DirectX11System::Instance().GetCtx()->VSGetShader(&now_vs, nullptr, nullptr);
    
    if (now_vs) {
        now_vs->Release();
    }

    if (now_vs != vs) {
        DirectX11System::Instance().GetCtx()->VSSetShader(vs, nullptr, 0);
        return true;
    }

    return false;
}

bool ShaderManager::SetPixelShader(ID3D11PixelShader* ps)
{
    if (!ps) {
        return false;
    }

    ID3D11PixelShader* now_ps = nullptr;
    DirectX11System::Instance().GetCtx()->PSGetShader(&now_ps, nullptr, nullptr);

    if (now_ps) {
        now_ps->Release();
    }

    if (now_ps != ps) {
        DirectX11System::Instance().GetCtx()->PSSetShader(ps, nullptr, 0);
        return true;
    }

    return false;
}

void ShaderManager::ChangeDepthStencilState(ID3D11DepthStencilState* dss)
{
    if (!dss) return;

    ID3D11DepthStencilState* now_dss = nullptr;
    DirectX11System::Instance().GetCtx()->OMGetDepthStencilState(&now_dss, 0);

    if (now_dss != dss) {
        m_pDssUndo = now_dss;

        DirectX11System::Instance().GetCtx()->OMSetDepthStencilState(dss, 0);
    }

    if (now_dss) {
        now_dss->Release();
    }
}

void ShaderManager::UndoDepthStencilState()
{
    if (!m_pDssUndo) return;

    DirectX11System::Instance().GetCtx()->OMSetDepthStencilState(m_pDssUndo, 0);

    m_pDssUndo = nullptr;
}

void ShaderManager::ChangeBlendState(ID3D11BlendState* bs)
{
    if (!bs) return;

    ID3D11BlendState* now_bs = nullptr;
    DirectX11System::Instance().GetCtx()->OMGetBlendState(&now_bs, nullptr, nullptr);

    if (now_bs != bs) {
        m_pBSUndo = now_bs;

        DirectX11System::Instance().GetCtx()->OMSetBlendState(bs, Math::Color(0, 0, 0, 0), 0xFFFFFFFF);
    }

    if (now_bs) {
        now_bs->Release();
    }
}

void ShaderManager::UndoBlendState()
{
    if (!m_pBSUndo) return;

    DirectX11System::Instance().GetCtx()->OMSetBlendState(m_pBSUndo, Math::Color(0, 0, 0, 0), 0xFFFFFFFF);

    m_pBSUndo = nullptr;
}

void ShaderManager::ChangeRasterizerState(ID3D11RasterizerState* rs)
{
    if (!rs) return;

    ID3D11RasterizerState* now_rs = nullptr;
    DirectX11System::Instance().GetCtx()->RSGetState(&now_rs);

    if (now_rs != rs) {
        m_pRSUndo = now_rs;

        DirectX11System::Instance().GetCtx()->RSSetState(rs);
    }

    if (now_rs) {
        now_rs->Release();
    }
}

void ShaderManager::UndoRasterizerState()
{
    if (!m_pRSUndo) return;

    DirectX11System::Instance().GetCtx()->RSSetState(m_pRSUndo);

    m_pRSUndo = nullptr;
}

void ShaderManager::ChangeSamplerState(int slot, ID3D11SamplerState* ss)
{
    if (!ss) return;

    ID3D11SamplerState* now_ss = nullptr;
    DirectX11System::Instance().GetCtx()->PSGetSamplers(slot, 1, &now_ss);

    if (now_ss != ss) {
        m_pSSUndo = now_ss;

        DirectX11System::Instance().GetCtx()->PSSetSamplers(slot, 1, &ss);
    }

    if (now_ss) {
        now_ss->Release();
    }
}

void ShaderManager::UndoSamplerState()
{
    if (!m_pSSUndo) return;

    DirectX11System::Instance().GetCtx()->PSSetSamplers(0, 1, &m_pSSUndo);

    m_pSSUndo = nullptr;
}
