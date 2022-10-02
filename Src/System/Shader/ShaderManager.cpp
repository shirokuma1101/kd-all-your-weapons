#include "ShaderManager.h"

void ShaderManager::Init()
{
    //============================================
    // シェーダ
    //============================================
    m_standardShader.Init();
    //m_effectShader.Init();
    //m_spriteShader.Init();

    //============================================
    // 定数バッファ
    //============================================

    // カメラ
    m_cb7_Camera.Create();
    DirectX11System::Instance().GetCtx()->VSSetConstantBuffers(7, 1, m_cb7_Camera.GetBufferAddress());
    DirectX11System::Instance().GetCtx()->PSSetConstantBuffers(7, 1, m_cb7_Camera.GetBufferAddress());

    // ライト
    m_cb8_Light.Create();
    m_cb8_Light.Get()->DirLight_Dir.Normalize();
    m_cb8_Light.Write();
    DirectX11System::Instance().GetCtx()->VSSetConstantBuffers(8, 1, m_cb8_Light.GetBufferAddress());
    DirectX11System::Instance().GetCtx()->PSSetConstantBuffers(8, 1, m_cb8_Light.GetBufferAddress());

    //============================================
    // パイプラインステート関係
    //============================================

    //深度ステンシルステート作成
    m_ds_ZEnable_ZWriteEnable   = directx11_helper::CreateDepthStencilState(DirectX11System::Instance().GetDev().Get(), true, true);
    m_ds_ZEnable_ZWriteDisable  = directx11_helper::CreateDepthStencilState(DirectX11System::Instance().GetDev().Get(), true, false);
    m_ds_ZDisable_ZWriteDisable = directx11_helper::CreateDepthStencilState(DirectX11System::Instance().GetDev().Get(), false, false);

    DirectX11System::Instance().GetCtx()->OMSetDepthStencilState(m_ds_ZEnable_ZWriteEnable, 0);

    // ラスタライザステート作成
    m_rs_CullBack = directx11_helper::CreateRasterizerState(DirectX11System::Instance().GetDev().Get(), D3D11_FILL_SOLID, D3D11_CULL_BACK, true, false);
    m_rs_CullNone = directx11_helper::CreateRasterizerState(DirectX11System::Instance().GetDev().Get(), D3D11_FILL_SOLID, D3D11_CULL_NONE, true, false);

    DirectX11System::Instance().GetCtx()->RSSetState(m_rs_CullBack);

    // ブレンドステート作成
    m_bs_Alpha = directx11_helper::CreateBlendState(DirectX11System::Instance().GetDev().Get(), directx11_helper::BlendMode::Alpha);
    m_bs_Add   = directx11_helper::CreateBlendState(DirectX11System::Instance().GetDev().Get(), directx11_helper::BlendMode::Add);

    DirectX11System::Instance().GetCtx()->OMSetBlendState(m_bs_Alpha, Math::Color(0, 0, 0, 0), 0xFFFFFFFF);

    // サンプラーステート作成
    m_ss_Anisotropic_Wrap  = directx11_helper::CreateSamplerState(DirectX11System::Instance().GetDev().Get(), directx11_helper::SamplerFilterMode::Anisotropic, 4, directx11_helper::SamplerAddressMode::Wrap, false);
    m_ss_Anisotropic_Clamp = directx11_helper::CreateSamplerState(DirectX11System::Instance().GetDev().Get(), directx11_helper::SamplerFilterMode::Anisotropic, 4, directx11_helper::SamplerAddressMode::Clamp, false);
    m_ss_Linear_Clamp      = directx11_helper::CreateSamplerState(DirectX11System::Instance().GetDev().Get(), directx11_helper::SamplerFilterMode::Linear, 0, directx11_helper::SamplerAddressMode::Clamp, false);
    m_ss_Linear_Clamp_Cmp  = directx11_helper::CreateSamplerState(DirectX11System::Instance().GetDev().Get(), directx11_helper::SamplerFilterMode::Linear, 0, directx11_helper::SamplerAddressMode::Clamp, true);
    m_ss_Point_Wrap        = directx11_helper::CreateSamplerState(DirectX11System::Instance().GetDev().Get(), directx11_helper::SamplerFilterMode::Point, 0, directx11_helper::SamplerAddressMode::Wrap, false);
    
    DirectX11System::Instance().GetCtx()->PSSetSamplers(0, 1, &m_ss_Anisotropic_Wrap);
}

// 頂点シェーダのセット（既にセットされているシェーダーの場合はキャンセル）
bool ShaderManager::SetVertexShader(ID3D11VertexShader* pSetVS)
{
    if (!pSetVS) {
        return false;
    }

    ID3D11VertexShader* pNowVS = nullptr;
    DirectX11System::Instance().GetCtx()->VSGetShader(&pNowVS, nullptr, nullptr);

    bool needChange = pNowVS != pSetVS;

    if (pNowVS) {
        pNowVS->Release();
    }
    
    if (needChange) {
        DirectX11System::Instance().GetCtx()->VSSetShader(pSetVS, nullptr, 0);
    }

    return needChange;
}

// ピクセルシェーダのセット（既にセットされているシェーダーの場合はキャンセル）
bool ShaderManager::SetPixelShader(ID3D11PixelShader* pSetPS)
{
    if (!pSetPS) {
        return false;
    }

    ID3D11PixelShader* pNowPS = nullptr;
    DirectX11System::Instance().GetCtx()->PSGetShader(&pNowPS, nullptr, nullptr);

    bool needChange = pNowPS != pSetPS;

    if (pNowPS) {
        pNowPS->Release();
    }

    if (needChange) {
        DirectX11System::Instance().GetCtx()->PSSetShader(pSetPS, nullptr, 0);
    }

    return needChange;
}

bool ShaderManager::SetInputLayout(ID3D11InputLayout* pSetLayout)
{
    if (!pSetLayout) {
        return false;
    }

    ID3D11InputLayout* pNowLayout = nullptr;

    DirectX11System::Instance().GetCtx()->IAGetInputLayout(&pNowLayout);

    bool needChange = pNowLayout != pSetLayout;

    if (pNowLayout) {
        pNowLayout->Release();
    }
    
    if (needChange) {
        DirectX11System::Instance().GetCtx()->IASetInputLayout(pSetLayout);
    }

    return needChange;
}

bool ShaderManager::SetVSConstantBuffer(int startSlot, ID3D11Buffer* const* pSetVSBuffer)
{
    if (!pSetVSBuffer) {
        return false;
    }

    ID3D11Buffer* pNowVSBuffer = nullptr;

    DirectX11System::Instance().GetCtx()->VSGetConstantBuffers(startSlot, 1, &pNowVSBuffer);

    bool needChange = pNowVSBuffer != *pSetVSBuffer;

    if (pNowVSBuffer) {
        pNowVSBuffer->Release();
    }

    if (needChange) {
        DirectX11System::Instance().GetCtx()->VSSetConstantBuffers(startSlot, 1, pSetVSBuffer);
    }

    return needChange;
}

bool ShaderManager::SetPSConstantBuffer(int startSlot, ID3D11Buffer* const* pSetPSBuffer)
{
    if (!pSetPSBuffer) {
        return false;
    }

    ID3D11Buffer* pNowPSBuffer = nullptr;

    DirectX11System::Instance().GetCtx()->PSGetConstantBuffers(startSlot, 1, &pNowPSBuffer);

    bool needChange = pNowPSBuffer != *pSetPSBuffer;

    if (pNowPSBuffer) {
        pNowPSBuffer->Release();
    }

    if (needChange) {
        DirectX11System::Instance().GetCtx()->PSSetConstantBuffers(startSlot, 1, pSetPSBuffer);
    }

    return needChange;
}

void ShaderManager::ChangeDepthStencilState(ID3D11DepthStencilState* pSetDs)
{
    if (!pSetDs) return;

    ID3D11DepthStencilState* pNowDs = nullptr;
    DirectX11System::Instance().GetCtx()->OMGetDepthStencilState(&pNowDs, 0);

    if (pNowDs != pSetDs) {
        m_ds_Undo = pNowDs;

        DirectX11System::Instance().GetCtx()->OMSetDepthStencilState(pSetDs, 0);
    }

    if (pNowDs) {
        pNowDs->Release();
    }
}

void ShaderManager::UndoDepthStencilState()
{
    if (!m_ds_Undo) return;

    DirectX11System::Instance().GetCtx()->OMSetDepthStencilState(m_ds_Undo, 0);

    m_ds_Undo = nullptr;
}

void ShaderManager::ChangeRasterizerState(ID3D11RasterizerState* pSetRs)
{
    if (!pSetRs) return;

    ID3D11RasterizerState* pNowRs = nullptr;
    DirectX11System::Instance().GetCtx()->RSGetState(&pNowRs);

    if (pNowRs != pSetRs) {
        m_rs_Undo = pNowRs;

        DirectX11System::Instance().GetCtx()->RSSetState(pSetRs);
    }

    if (pNowRs) {
        pNowRs->Release();
    }
}

void ShaderManager::UndoRasterizerState()
{
    if (!m_rs_Undo) return;

    DirectX11System::Instance().GetCtx()->RSSetState(m_rs_Undo);

    m_rs_Undo = nullptr;
}

void ShaderManager::ChangeBlendState(ID3D11BlendState* pSetBs)
{
    if (!pSetBs) return;

    ID3D11BlendState* pNowBs = nullptr;
    DirectX11System::Instance().GetCtx()->OMGetBlendState(&pNowBs, nullptr, nullptr);

    if (pNowBs != pSetBs) {
        m_bs_Undo = pNowBs;

        DirectX11System::Instance().GetCtx()->OMSetBlendState(pSetBs, Math::Color(0, 0, 0, 0), 0xFFFFFFFF);
    }

    if (pNowBs) {
        pNowBs->Release();
    }
}

void ShaderManager::UndoBlendState()
{
    if (!m_bs_Undo) return;

    DirectX11System::Instance().GetCtx()->OMSetBlendState(m_bs_Undo, Math::Color(0, 0, 0, 0), 0xFFFFFFFF);

    m_bs_Undo = nullptr;
}

void ShaderManager::ChangeSamplerState(int slot, ID3D11SamplerState* pSetSs)
{
    if (!pSetSs) return;

    ID3D11SamplerState* pNowSs = nullptr;
    DirectX11System::Instance().GetCtx()->PSGetSamplers(slot, 1, &pNowSs);

    if (pNowSs != pSetSs) {
        m_ss_Undo = pNowSs;

        DirectX11System::Instance().GetCtx()->PSSetSamplers(slot, 1, &pSetSs);
    }

    if (pNowSs) {
        pNowSs->Release();
    }
}

void ShaderManager::UndoSamplerState()
{
    if (!m_ss_Undo) { return; }

    DirectX11System::Instance().GetCtx()->PSSetSamplers(0, 1, &m_ss_Undo);

    m_ss_Undo = nullptr;
}

void ShaderManager::AddPointLight(const Math::Vector3& pos, const Math::Vector3& color, float radius, bool isBright)
{
    ShaderManager::cbLight* cbLight = m_cb8_Light.Get();

    ShaderManager::cbLight::PointLight& modifyPointLight = cbLight->PointLights[cbLight->PointLight_Num];

    modifyPointLight.Pos = pos;
    modifyPointLight.Color = color;
    modifyPointLight.Radius = radius;
    modifyPointLight.IsBright = isBright;

    ++cbLight->PointLight_Num;
}

void ShaderManager::Release()
{
    m_standardShader.Release();
    //m_effectShader.Release();
    //m_spriteShader.Release();

    m_cb7_Camera.Release();
    m_cb8_Light.Release();

    //深度ステンシルステート開放
    memory::SafeRelease(&m_ds_ZEnable_ZWriteEnable);
    memory::SafeRelease(&m_ds_ZEnable_ZWriteDisable);
    memory::SafeRelease(&m_ds_ZDisable_ZWriteDisable);
    m_ds_Undo = nullptr;

    // ラスタライザステート解放
    memory::SafeRelease(&m_rs_CullBack);
    memory::SafeRelease(&m_rs_CullNone);
    m_rs_Undo = nullptr;

    // ブレンドステート解放
    memory::SafeRelease(&m_bs_Alpha);
    memory::SafeRelease(&m_bs_Add);
    m_bs_Undo = nullptr;

    // サンプラーステート解放
    memory::SafeRelease(&m_ss_Anisotropic_Wrap);
    memory::SafeRelease(&m_ss_Anisotropic_Clamp);
    memory::SafeRelease(&m_ss_Linear_Clamp);
    memory::SafeRelease(&m_ss_Linear_Clamp_Cmp);
    memory::SafeRelease(&m_ss_Point_Wrap);
    m_ss_Undo = nullptr;
}
