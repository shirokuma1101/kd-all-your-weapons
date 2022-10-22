#include "SpriteShader.h"

bool SpriteShader::Init()
{
    {
        #include "SpriteShaderVS.inc"
        if (FAILED(DirectX11System::Instance().GetDev()->CreateVertexShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_pVS))) {
            assert::RaiseAssert(ASSERT_FILE_LINE, "頂点シェーダー作成失敗");
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
            static_cast<UINT>(layout.size()),
            compiledBuffer,
            sizeof(compiledBuffer),
            &m_pInputLayout
        ))) {
            assert::RaiseAssert(ASSERT_FILE_LINE, "頂点インプットレイアウト作成失敗");
            Release();
            return false;
        }
    }
    {
        #include "SpriteShaderPS.inc"
        if (FAILED(DirectX11System::Instance().GetDev()->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_pPS))) {
            assert::RaiseAssert(ASSERT_FILE_LINE, "ピクセルシェーダー作成失敗");
            Release();
            return false;
        }
    }

    m_spriteCB.Create();
    m_projectionCB.Create();
    
    return true;
}

void SpriteShader::Release()
{
    m_spriteCB.Release();
    m_projectionCB.Release();
    
    memory::SafeRelease(&m_pVS);
    memory::SafeRelease(&m_pInputLayout);
    memory::SafeRelease(&m_pPS);
}

void SpriteShader::SetToDevice()
{
    UINT viewport = 1;
    D3D11_VIEWPORT vp{};
    DirectX11System::Instance().GetCtx()->RSGetViewports(&viewport, &vp);
    
    m_projectionCB.Get()->projection = DirectX::XMMatrixOrthographicLH(vp.Width, vp.Height, 0, 1);
    m_projectionCB.Write();

    DirectX11System::Instance().GetCtx()->VSSetShader(m_pVS, nullptr, 0);
    DirectX11System::Instance().GetCtx()->PSSetShader(m_pPS, nullptr, 0);
    DirectX11System::Instance().GetCtx()->IASetInputLayout(m_pInputLayout);

    DirectX11System::Instance().GetCtx()->VSSetConstantBuffers(2, 1, m_spriteCB.GetBufferAddress());
    DirectX11System::Instance().GetCtx()->PSSetConstantBuffers(2, 1, m_spriteCB.GetBufferAddress());
    DirectX11System::Instance().GetCtx()->VSSetConstantBuffers(3, 1, m_projectionCB.GetBufferAddress());
    DirectX11System::Instance().GetCtx()->PSSetConstantBuffers(3, 1, m_projectionCB.GetBufferAddress());
}

void SpriteShader::DrawTex(const DirectX11Texture& tex, const Math::Vector2& pos, const Math::Vector2& rect, const Math::Rectangle* src_rect, const Math::Color& color, const Math::Vector2& pivot)
{
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, tex.GetSrvAddress());
    
    m_spriteCB.Get()->color = color;
    m_spriteCB.Write();
    
    Math::Vector2 uv_min = { 0, 0 };
    Math::Vector2 uv_max = { 1, 1 };
    if (src_rect) {
        uv_min.x = src_rect->x / (float)tex.GetTextureDesc().Width;
        uv_min.y = src_rect->y / (float)tex.GetTextureDesc().Height;
        uv_max.x = (src_rect->x + src_rect->width) / (float)tex.GetTextureDesc().Width;
        uv_max.y = (src_rect->y + src_rect->height) / (float)tex.GetTextureDesc().Height;
    }

    // 頂点作成
    float x1 = pos.x;
    float y1 = pos.y;
    float x2 = (float)(pos.x + rect.x);
    float y2 = (float)(pos.y + rect.y);

    // 基準点(Pivot)分ずらす
    x1 -= pivot.x * rect.x;
    x2 -= pivot.x * rect.x;
    y1 -= pivot.y * rect.y;
    y2 -= pivot.y * rect.y;

    Vertex vertex[] = {
        { {x1, y1, 0},	{uv_min.x, uv_max.y} },
        { {x1, y2, 0},	{uv_min.x, uv_min.y} },
        { {x2, y1, 0},	{uv_max.x, uv_max.y} },
        { {x2, y2, 0},	{uv_max.x, uv_min.y} },
    };
    DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, sizeof(vertex), vertex);

    // セットしたテクスチャを解除しておく
    ID3D11ShaderResourceView* srv = nullptr;
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, &srv);
}

void SpriteShader::DrawPoint(const Math::Vector2& pos, const Math::Color& color)
{
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, DirectX11System::Instance().GetWhiteTexture()->GetSrvAddress());

    m_spriteCB.Get()->color = color;
    m_spriteCB.Write();
    
    Vertex vertex[] = {
        { {pos.x, pos.y, 0},{0, 0} },
    };
    DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_POINTLIST, 1, sizeof(vertex), vertex);
}

void SpriteShader::DrawLine(const Math::Vector2& pos1, const Math::Vector2& pos2, const Math::Color& color)
{
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, DirectX11System::Instance().GetWhiteTexture()->GetSrvAddress());

    m_spriteCB.Get()->color = color;
    m_spriteCB.Write();
    
    Vertex vertex[] = {
        { {pos1.x, pos1.y, 0}, {0, 0} },
        { {pos2.x, pos2.y, 0}, {1, 0} },
    };
    DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, 2, sizeof(vertex), vertex);
}

void SpriteShader::DrawTriangle(const Math::Vector2& pos1, const Math::Vector2& pos2, const Math::Vector2& pos3, const Math::Color& color, bool fill)
{
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, DirectX11System::Instance().GetWhiteTexture()->GetSrvAddress());

    m_spriteCB.Get()->color = color;
    m_spriteCB.Write();

    Vertex vertex[] = {
        { {pos1.x, pos1.y, 0}, {0, 0} },
        { {pos2.x, pos2.y, 0}, {1, 0} },
        { {pos3.x, pos3.y, 0}, {0, 0} },
        { {pos1.x, pos1.y, 0}, {1, 0} },
    };
    DirectX11System::Instance().DrawVertices(fill ? D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP : D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, 4, sizeof(vertex), vertex);
}

void SpriteShader::DrawBox(const Math::Vector2& pos, const Math::Vector2& extent, const Math::Color& color, bool fill)
{
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, DirectX11System::Instance().GetWhiteTexture()->GetSrvAddress());
    
    m_spriteCB.Get()->color = color;
    m_spriteCB.Write();

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
        DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, sizeof(vertex), vertex);
    }
    else {
        Vertex vertex[] = {
            { p1, {0, 0}},
            { p2, {0, 0}},
            { p3, {0, 0}},
            { p4, {0, 0}},
            { p1, {0, 0}}
        };
        DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, 5, sizeof(vertex), vertex);
    }
}

void SpriteShader::DrawCircle(const Math::Vector2& pos, float radius, const Math::Color& color, bool fill)
{
    if (radius <= 0.f) return;

    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, DirectX11System::Instance().GetWhiteTexture()->GetSrvAddress());
    
    m_spriteCB.Get()->color = color;
    m_spriteCB.Write();

    if (fill) {
        int face_count = static_cast<int>(radius) + 1;
        if (face_count > 300) {
            face_count = 300;
        }
        std::vector<Vertex> vertex(face_count * 3);
        
        for (int i = 0; i < face_count; ++i) {
            int idx = i * 3;
            vertex[idx].pos.x = pos.x;
            vertex[idx].pos.y = pos.y;
            
            vertex[idx + 1].pos.x = pos.x + std::cos(DirectX::XMConvertToRadians(i * (360.0f / (face_count - 1)))) * radius;
            vertex[idx + 1].pos.y = pos.y + std::sin(DirectX::XMConvertToRadians(i * (360.0f / (face_count - 1)))) * radius;
            vertex[idx + 1].pos.z = 0;
            
            vertex[idx + 2].pos.x = pos.x + std::cos(DirectX::XMConvertToRadians((i + 1) * (360.0f / (face_count - 1)))) * radius;
            vertex[idx + 2].pos.y = pos.y + std::sin(DirectX::XMConvertToRadians((i + 1) * (360.0f / (face_count - 1)))) * radius;
            vertex[idx + 2].pos.z = 0;
        }
        DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, (int)vertex.size(), sizeof(vertex), &vertex[0]);
    }
    else {
        int vertex_count = static_cast<int>(radius) + 1;
        if (vertex_count > 300) {
            vertex_count = 300;
        }
        std::vector<Vertex> vertex(vertex_count); // 半径により頂点数を調整
        
        for (int i = 0; i < vertex_count; ++i) {
            vertex[i].pos.x = pos.x + cos(DirectX::XMConvertToRadians(i * (360.0f / (vertex_count - 1)))) * radius;
            vertex[i].pos.y = pos.y + sin(DirectX::XMConvertToRadians(i * (360.0f / (vertex_count - 1)))) * radius;
            vertex[i].pos.z = 0;
        }
        DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, vertex_count, sizeof(vertex), &vertex[0]);
    }
}
