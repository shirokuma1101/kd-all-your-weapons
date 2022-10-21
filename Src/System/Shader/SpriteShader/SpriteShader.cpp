#include "SpriteShader.h"

bool SpriteShader::Init()
{
    //-------------------------------------
    // 頂点シェーダ
    //-------------------------------------
    {
        // コンパイル済みのシェーダーヘッダーファイルをインクルード
        #include "SpriteShader_VS.inc"

        // 頂点シェーダー作成
        if (FAILED(DirectX11System::Instance().GetDev()->CreateVertexShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_VS))) {
            assert(0 && "頂点シェーダー作成失敗");
            Release();
            return false;
        }

        // １頂点の詳細な情報
        std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        // 頂点インプットレイアウト作成
        if (FAILED(DirectX11System::Instance().GetDev()->CreateInputLayout(
            &layout[0],
            layout.size(),
            compiledBuffer,
            sizeof(compiledBuffer),
            &m_VLayout))
        ){
            assert(0 && "CreateInputLayout失敗");
            Release();
            return false;
        }
    }

    //-------------------------------------
    // ピクセルシェーダ
    //-------------------------------------
    {
        // コンパイル済みのシェーダーヘッダーファイルをインクルード
        #include "SpriteShader_PS.inc"

        if (FAILED(DirectX11System::Instance().GetDev()->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_PS))) {
            assert(0 && "ピクセルシェーダー作成失敗");
            Release();
            return false;
        }
    }

    //-------------------------------------
    // 定数バッファ作成
    //-------------------------------------
    m_cb0.Create();
    m_cb1.Create();

    //-------------------------------------
    // ステート作成
    //-------------------------------------
    m_smp0_Point = directx11_helper::CreateSamplerState(
        DirectX11System::Instance().GetDev().Get(), directx11_helper::SamplerFilterMode::Point, 0, directx11_helper::SamplerAddressMode::Wrap, false
    );
    m_smp0_Linear = directx11_helper::CreateSamplerState(
        DirectX11System::Instance().GetDev().Get(), directx11_helper::SamplerFilterMode::Linear, 0, directx11_helper::SamplerAddressMode::Wrap, false
    );

    m_ds = directx11_helper::CreateDepthStencilState(DirectX11System::Instance().GetDev().Get(), false, false);
    m_rs = directx11_helper::CreateRasterizerState(DirectX11System::Instance().GetDev().Get(), D3D11_FILL_SOLID, D3D11_CULL_NONE, true, false);

    return true;
}

void SpriteShader::Begin(bool linear, bool disable_zbuffer)
{
    //---------------------------------------
    // 2D用正射影行列作成
    //---------------------------------------
    UINT pNumVierports = 1;
    D3D11_VIEWPORT vp;
    DirectX11System::Instance().GetCtx()->RSGetViewports(&pNumVierports, &vp);
    m_mProj2D = DirectX::XMMatrixOrthographicLH(vp.Width, vp.Height, 0, 1);

    // 定数バッファ書き込み
    m_cb1.Get()->mProj = m_mProj2D;
    m_cb1.Write();

    //---------------------------------------
    // 現在のステート記憶
    //---------------------------------------
    // DepthStencilState記憶
    DirectX11System::Instance().GetCtx()->OMGetDepthStencilState(&m_saveState.DS, &m_saveState.StencilRef);
    // RasterizerState記憶
    DirectX11System::Instance().GetCtx()->RSGetState(&m_saveState.RS);
    // SamplerState記億
    DirectX11System::Instance().GetCtx()->PSGetSamplers(0, 1, &m_saveState.Smp0);

    //---------------------------------------
    // 使用するステートをセット
    //---------------------------------------
    // Z判定、Z書き込み無効のステートをセット
    if (disable_zbuffer) {
        DirectX11System::Instance().GetCtx()->OMSetDepthStencilState(m_ds, 0);
    }
    // Samplerステートをセット
    if (linear) {
        DirectX11System::Instance().GetCtx()->PSSetSamplers(0, 1, &m_smp0_Linear);
    }
    else {
        DirectX11System::Instance().GetCtx()->PSSetSamplers(0, 1, &m_smp0_Point);
    }
    // Rasterizerステートをセット
    DirectX11System::Instance().GetCtx()->RSSetState(m_rs);

    //---------------------------------------
    // シェーダ
    //---------------------------------------

    // シェーダをセット
    DirectX11System::Instance().GetCtx()->VSSetShader(m_VS, 0, 0);
    DirectX11System::Instance().GetCtx()->PSSetShader(m_PS, 0, 0);

    // 頂点レイアウトセット
    DirectX11System::Instance().GetCtx()->IASetInputLayout(m_VLayout);

    // 定数バッファセット
    DirectX11System::Instance().GetCtx()->VSSetConstantBuffers(2, 1, m_cb0.GetBufferAddress());
    DirectX11System::Instance().GetCtx()->PSSetConstantBuffers(2, 1, m_cb0.GetBufferAddress());
    DirectX11System::Instance().GetCtx()->VSSetConstantBuffers(3, 1, m_cb1.GetBufferAddress());
    DirectX11System::Instance().GetCtx()->PSSetConstantBuffers(3, 1, m_cb1.GetBufferAddress());
}

void SpriteShader::End()
{
    //---------------------------------------
    // 記憶してたステートに戻す
    //---------------------------------------
    DirectX11System::Instance().GetCtx()->OMSetDepthStencilState(m_saveState.DS, m_saveState.StencilRef);
    DirectX11System::Instance().GetCtx()->RSSetState(m_saveState.RS);
    DirectX11System::Instance().GetCtx()->PSSetSamplers(0, 1, &m_saveState.Smp0);
    memory::SafeRelease(&m_saveState.DS);
    memory::SafeRelease(&m_saveState.RS);
    memory::SafeRelease(&m_saveState.Smp0);
}

void SpriteShader::DrawTex(const DirectX11Texture& tex, const Math::Vector2& pos, const Math::Vector2& rect, const Math::Rectangle* src_rect, const Math::Color& color, const Math::Vector2& pivot)
{
    // テクスチャ(ShaderResourceView)セット
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, tex.GetSrvAddress());

    // 色
    m_cb0.Get()->Color = color;
    m_cb0.Write();

    // UV
    Math::Vector2 uvMin = { 0, 0 };
    Math::Vector2 uvMax = { 1, 1 };
    if (src_rect) {
        uvMin.x = src_rect->x / (float)tex.GetTextureDesc().Width;
        uvMin.y = src_rect->y / (float)tex.GetTextureDesc().Height;

        uvMax.x = (src_rect->x + src_rect->width) / (float)tex.GetTextureDesc().Width;
        uvMax.y = (src_rect->y + src_rect->height) / (float)tex.GetTextureDesc().Height;
    }

    // 頂点作成
    float x1 = pos.x;
    float y1 = pos.y;
    float x2 = (float)(pos.x + rect.x);
    float y2 = (float)(pos.y + rect.y);

    // 基準点(Pivot)ぶんずらす
    x1 -= pivot.x * rect.x;
    x2 -= pivot.x * rect.x;
    y1 -= pivot.y * rect.y;
    y2 -= pivot.y * rect.y;

    Vertex vertex[] = {
        { {x1, y1, 0},	{uvMin.x, uvMax.y} },
        { {x1, y2, 0},	{uvMin.x, uvMin.y} },
        { {x2, y1, 0},	{uvMax.x, uvMax.y} },
        { {x2, y2, 0},	{uvMax.x, uvMin.y} }

    };

    // 描画
    DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, sizeof(Vertex), vertex);

    // セットしたテクスチャを解除しておく
    ID3D11ShaderResourceView* srv = nullptr;
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, &srv);
}

void SpriteShader::DrawPoint(const Math::Vector2& pos, const Math::Color& color)
{
    // 白テクスチャ
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, DirectX11System::Instance().GetWhiteTexture()->GetSrvAddress());

    // 色
    m_cb0.Get()->Color = color;
    m_cb0.Write();
    
    // 描画
    Vertex vertex[] = {
        { {pos.x, pos.y, 0}, {0, 0} },
    };
    DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_POINTLIST, 1, sizeof(Vertex), vertex);
}

void SpriteShader::DrawLine(const Math::Vector2& pos1, const Math::Vector2& pos2, const Math::Color& color)
{
    // 白テクスチャ
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, DirectX11System::Instance().GetWhiteTexture()->GetSrvAddress());

    // 色
    m_cb0.Get()->Color = color;
    m_cb0.Write();
    
    // 描画
    Vertex vertex[] = {
        { {pos1.x, pos1.y, 0}, {0, 0} },
        { {pos2.x, pos2.y, 0}, {1, 0} },
    };
    DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, 2, sizeof(Vertex), vertex);
}

void SpriteShader::DrawTriangle(const Math::Vector2& pos1, const Math::Vector2& pos2, const Math::Vector2& pos3, const Math::Color& color, bool fill)
{
    // 白テクスチャ
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, DirectX11System::Instance().GetWhiteTexture()->GetSrvAddress());

    // 色
    m_cb0.Get()->Color = color;
    m_cb0.Write();
    
    // 描画
    Vertex vertex[] = {
        { {pos1.x, pos1.y, 0}, {0, 0} },
        { {pos2.x, pos2.y, 0}, {1, 0} },
        { {pos3.x, pos3.y, 0}, {0, 0} },
        { {pos1.x, pos1.y, 0}, {1, 0} },
    };
    DirectX11System::Instance().DrawVertices(
        fill ? D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP : D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,
        4, sizeof(Vertex), vertex);
}

void SpriteShader::DrawCircle(const Math::Vector2& pos, float radius, const Math::Color& color, bool fill)
{
    if (radius <= 0.f) return;

    // 白テクスチャ
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, DirectX11System::Instance().GetWhiteTexture()->GetSrvAddress());

    // 色
    m_cb0.Get()->Color = color;
    m_cb0.Write();

    // 頂点
    if (fill) {
        int faceNum = static_cast<int>(radius) + 1;
        if (faceNum > 300) {
            faceNum = 300;
        }
        std::vector<Vertex> vertex(faceNum * 3); // 半径により頂点数を調整
        
        // 描画
        for (int i = 0; i < faceNum; ++i) {
            int idx = i * 3;
            vertex[idx].Pos.x = pos.x;
            vertex[idx].Pos.y = pos.y;

            vertex[idx + 1].Pos.x = pos.x + std::cos(DirectX::XMConvertToRadians(i * (360.0f / (faceNum - 1)))) * radius;
            vertex[idx + 1].Pos.y = pos.y + std::sin(DirectX::XMConvertToRadians(i * (360.0f / (faceNum - 1)))) * radius;
            vertex[idx + 1].Pos.z = 0;

            vertex[idx + 2].Pos.x = pos.x + std::cos(DirectX::XMConvertToRadians((i + 1) * (360.0f / (faceNum - 1)))) * radius;
            vertex[idx + 2].Pos.y = pos.y + std::sin(DirectX::XMConvertToRadians((i + 1) * (360.0f / (faceNum - 1)))) * radius;
            vertex[idx + 2].Pos.z = 0;
        }

        DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, (int)vertex.size(), sizeof(Vertex), &vertex[0]);
    }
    else {
        int numVertex = static_cast<int>(radius) + 1;
        if (numVertex > 300) {
            numVertex = 300;
        }
        std::vector<Vertex> vertex(numVertex); // 半径により頂点数を調整

        // 描画
        for (int i = 0; i < numVertex; ++i) {
            vertex[i].Pos.x = pos.x + cos(DirectX::XMConvertToRadians(i * (360.0f / (numVertex - 1)))) * radius;
            vertex[i].Pos.y = pos.y + sin(DirectX::XMConvertToRadians(i * (360.0f / (numVertex - 1)))) * radius;
            vertex[i].Pos.z = 0;
        }

        DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, numVertex, sizeof(Vertex), &vertex[0]);
    }
}

void SpriteShader::DrawBox(const Math::Vector2& pos, const Math::Vector2& extent, const Math::Color& color, bool fill)
{
    // 白テクスチャ
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, DirectX11System::Instance().GetWhiteTexture()->GetSrvAddress());

    // 色
    m_cb0.Get()->Color = color;
    m_cb0.Write();

    Math::Vector3 p1 = { pos.x - extent.x, pos.y - extent.y, 0 };
    Math::Vector3 p2 = { pos.x - extent.x, pos.y + extent.y, 0 };
    Math::Vector3 p3 = { pos.x + extent.x, pos.y + extent.y, 0 };
    Math::Vector3 p4 = { pos.x + extent.x, pos.y - extent.y, 0 };

    // 描画
    if (fill) {
        Vertex vertex[] = {
            { p1, {0, 0}},
            { p2, {0, 0}},
            { p4, {0, 0}},
            { p3, {0, 0}}
        };

        DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, sizeof(Vertex), vertex);
    }
    else {
        Vertex vertex[] = {
            { p1, {0, 0}},
            { p2, {0, 0}},
            { p3, {0, 0}},
            { p4, {0, 0}},
            { p1, {0, 0}}
        };

        DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, 5, sizeof(Vertex), vertex);
    }
}

void SpriteShader::SetScissorRect(const Math::Rectangle& rect)
{
    ID3D11RasterizerState* rs = directx11_helper::CreateRasterizerState(DirectX11System::Instance().GetDev().Get(), D3D11_FILL_SOLID, D3D11_CULL_BACK, true, true);
    DirectX11System::Instance().GetCtx()->RSSetState(rs);
    rs->Release();
    memory::SafeRelease(&rs);
    
    D3D11_RECT rc{};
    rc.left   = rect.x;
    rc.top    = rect.y;
    rc.right  = rect.x + rect.width;
    rc.bottom = rect.y + rect.height;
    DirectX11System::Instance().GetCtx()->RSSetScissorRects(1, &rc);
}

void SpriteShader::Release()
{
    memory::SafeRelease(&m_VS);
    memory::SafeRelease(&m_PS);
    memory::SafeRelease(&m_VLayout);
    m_cb0.Release();
    m_cb1.Release();

    memory::SafeRelease(&m_ds);
    memory::SafeRelease(&m_rs);
    memory::SafeRelease(&m_smp0_Point);
    memory::SafeRelease(&m_smp0_Linear);

    memory::SafeRelease(&m_saveState.DS);
    memory::SafeRelease(&m_saveState.RS);
    memory::SafeRelease(&m_saveState.Smp0);
}
