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
    m_pDss                   = directx11_helper::CreateDepthStencilState(dev, false, false);
    m_pDssDepth              = directx11_helper::CreateDepthStencilState(dev, true,  false);
    m_pDssDepthWriteDepth    = directx11_helper::CreateDepthStencilState(dev, true,  true);
    ctx->OMSetDepthStencilState(m_pDssDepthWriteDepth, 0);

    /* ブレンドステート作成 */
    m_pBSAlpha               = directx11_helper::CreateBlendState(dev, directx11_helper::BlendMode::ALPHA);
    m_pBSAdd                 = directx11_helper::CreateBlendState(dev, directx11_helper::BlendMode::ADD);
    ctx->OMSetBlendState(m_pBSAlpha, directx11_helper::alpha, 0xFFFFFFFF);

    /* ラスタライズステート作成 */
    m_pRSSolidNone           = directx11_helper::CreateRasterizerState(dev, directx11_helper::FillMode::SOLID,     directx11_helper::CullMode::NONE, true, false);
    m_pRSSolidBack           = directx11_helper::CreateRasterizerState(dev, directx11_helper::FillMode::SOLID,     directx11_helper::CullMode::BACK, true, false);
    m_pRSWireframeNone       = directx11_helper::CreateRasterizerState(dev, directx11_helper::FillMode::WIREFRAME, directx11_helper::CullMode::NONE, true, false);
    m_pRSWireframeBack       = directx11_helper::CreateRasterizerState(dev, directx11_helper::FillMode::WIREFRAME, directx11_helper::CullMode::BACK, true, false);
    ctx->RSSetState(m_pRSSolidBack);

    /* サンプラーステート作成 */
    m_pSSPointWrap           = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::POINT,        0, directx11_helper::SamplerAddressMode::WRAP,  false);
    m_pSSPointClamp          = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::POINT,        0, directx11_helper::SamplerAddressMode::CLAMP, false);
    m_pSSLinearWrap          = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::LINEAR,       0, directx11_helper::SamplerAddressMode::WRAP,  false);
    m_pSSLinearClamp         = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::LINEAR,       0, directx11_helper::SamplerAddressMode::CLAMP, false);
    m_pSSLinearClampComp     = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::LINEAR,       0, directx11_helper::SamplerAddressMode::CLAMP, true);
    m_pSSAnisotropicWrap     = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::ANISOTROPIC,  1, directx11_helper::SamplerAddressMode::WRAP,  false);
    m_pSSAnisotropic2xWrap   = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::ANISOTROPIC,  2, directx11_helper::SamplerAddressMode::WRAP,  false);
    m_pSSAnisotropic4xWrap   = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::ANISOTROPIC,  4, directx11_helper::SamplerAddressMode::WRAP,  false);
    m_pSSAnisotropic8xWrap   = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::ANISOTROPIC,  8, directx11_helper::SamplerAddressMode::WRAP,  false);
    m_pSSAnisotropic16xWrap  = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::ANISOTROPIC, 16, directx11_helper::SamplerAddressMode::WRAP,  false);
    m_pSSAnisotropicClamp    = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::ANISOTROPIC,  1, directx11_helper::SamplerAddressMode::CLAMP, false);
    m_pSSAnisotropic2xClamp  = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::ANISOTROPIC,  2, directx11_helper::SamplerAddressMode::CLAMP, false);
    m_pSSAnisotropic4xClamp  = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::ANISOTROPIC,  4, directx11_helper::SamplerAddressMode::CLAMP, false);
    m_pSSAnisotropic8xClamp  = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::ANISOTROPIC,  8, directx11_helper::SamplerAddressMode::CLAMP, false);
    m_pSSAnisotropic16xClamp = directx11_helper::CreateSamplerState(dev, directx11_helper::SamplerFilterMode::ANISOTROPIC, 16, directx11_helper::SamplerAddressMode::CLAMP, false);

    ctx->VSSetSamplers(0, 1, &m_pSSAnisotropicWrap);
    ctx->PSSetSamplers(0, 1, &m_pSSAnisotropicWrap);
    ctx->GSSetSamplers(0, 1, &m_pSSAnisotropicWrap);
    ctx->CSSetSamplers(0, 1, &m_pSSAnisotropicWrap);
}

void ShaderManager::Release()
{
    m_standardShader.Release();
    m_spriteShader.Release();
    m_spriteFont.Release();

    m_cameraCB.Release();
    m_lightCB.Release();
    

    /* 深度ステンシルステート解放 */
    memory::SafeRelease(&m_pDss);
    memory::SafeRelease(&m_pDssDepth);
    memory::SafeRelease(&m_pDssDepthWriteDepth);
    m_pDssUndo = nullptr;

    /* ブレンドステート解放 */
    memory::SafeRelease(&m_pBSAlpha);
    memory::SafeRelease(&m_pBSAdd);
    m_pBSUndo = nullptr;

    /* ラスタライズステート解放 */
    memory::SafeRelease(&m_pRSSolidNone);
    memory::SafeRelease(&m_pRSSolidBack);
    memory::SafeRelease(&m_pRSWireframeNone);
    memory::SafeRelease(&m_pRSWireframeBack);
    m_pRSUndo = nullptr;

    /* サンプラーステート解放 */
    memory::SafeRelease(&m_pSSPointWrap);
    memory::SafeRelease(&m_pSSPointClamp);
    memory::SafeRelease(&m_pSSLinearWrap);
    memory::SafeRelease(&m_pSSLinearClamp);
    memory::SafeRelease(&m_pSSLinearClampComp);
    memory::SafeRelease(&m_pSSAnisotropicWrap);
    memory::SafeRelease(&m_pSSAnisotropic2xWrap);
    memory::SafeRelease(&m_pSSAnisotropic4xWrap);
    memory::SafeRelease(&m_pSSAnisotropic8xWrap);
    memory::SafeRelease(&m_pSSAnisotropic16xWrap);
    memory::SafeRelease(&m_pSSAnisotropicClamp);
    memory::SafeRelease(&m_pSSAnisotropic2xClamp);
    memory::SafeRelease(&m_pSSAnisotropic4xClamp);
    memory::SafeRelease(&m_pSSAnisotropic8xClamp);
    memory::SafeRelease(&m_pSSAnisotropic16xClamp);
    m_pSSUndo = nullptr;
}

void ShaderManager::SetToDevice()
{
    const auto camera = m_cameraCB.Get();
    auto light = m_lightCB.Get();
    light->directionalLightDirection.Normalize();
    light->directionalLightVP
        = DirectX::XMMatrixLookAtLH(camera->position - light->directionalLightDirection * 50.f, camera->position, Math::Vector3::Up)
        * DirectX::XMMatrixOrthographicLH(50, 50, 0, 100);
    m_lightCB.Write();

    auto ctx = DirectX11System::Instance().GetCtx();
    ctx->VSSetConstantBuffers(0, 1, m_cameraCB.GetBufferAddress());
    ctx->PSSetConstantBuffers(0, 1, m_cameraCB.GetBufferAddress());
    ctx->VSSetConstantBuffers(1, 1, m_lightCB.GetBufferAddress());
    ctx->PSSetConstantBuffers(1, 1, m_lightCB.GetBufferAddress());
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

void ShaderManager::AddPointLight(const Math::Vector3& position, const Math::Vector3& color, const Math::Vector3& attenuation)
{
    auto light = m_lightCB.Get();
    light->pointLight[light->pointLightCount].position = position;
    light->pointLight[light->pointLightCount].color = color;
    light->pointLight[light->pointLightCount].attenuation = attenuation;
    ++light->pointLightCount;
    m_lightCB.Write();
}

void ShaderManager::ClearPointLight()
{
    m_lightCB.Get()->pointLightCount = 0;
    m_lightCB.Write();
}
