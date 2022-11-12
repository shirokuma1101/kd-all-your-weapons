#include "SpriteShader.h"

bool SpriteShader::Init()
{
    auto dev = DirectX11System::Instance().GetDev();

    {
        #include "SpriteShaderVS.inc"
        if (FAILED(dev->CreateVertexShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_pVS))) {
            assert::ShowError(ASSERT_FILE_LINE, "頂点シェーダー作成失敗");
            Release();
            return false;
        }

        // １頂点の詳細な情報
        std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        // 頂点インプットレイアウト作成
        if (FAILED(dev->CreateInputLayout(
            &layout[0],
            static_cast<UINT>(layout.size()),
            compiledBuffer,
            sizeof(compiledBuffer),
            &m_pInputLayout
        ))) {
            assert::ShowError(ASSERT_FILE_LINE, "頂点インプットレイアウト作成失敗");
            Release();
            return false;
        }
    }
    {
        #include "SpriteShaderPS.inc"
        if (FAILED(dev->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_pPS))) {
            assert::ShowError(ASSERT_FILE_LINE, "ピクセルシェーダー作成失敗");
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
    auto v = DirectX11System::Instance().GetViewport();
    auto ctx = DirectX11System::Instance().GetCtx();

    m_projectionCB.Get()->projection = DirectX::XMMatrixOrthographicLH(v.Width, v.Height, 0, 1);
    m_projectionCB.Write();

    ctx->VSSetShader(m_pVS, 0, 0);
    ctx->PSSetShader(m_pPS, 0, 0);
    ctx->IASetInputLayout(m_pInputLayout);

    ctx->VSSetConstantBuffers(2, 1, m_spriteCB.GetBufferAddress());
    ctx->PSSetConstantBuffers(2, 1, m_spriteCB.GetBufferAddress());
    ctx->VSSetConstantBuffers(3, 1, m_projectionCB.GetBufferAddress());
    ctx->PSSetConstantBuffers(3, 1, m_projectionCB.GetBufferAddress());
}

void SpriteShader::DrawTex(const DirectX11Texture& tex, const Math::Matrix& world, const Math::Vector2& rect, const Math::Vector2& uv_min, const Math::Vector2& uv_max, const Math::Color& color, const Math::Vector2& pivot)
{
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, tex.GetSrvAddress());

    m_spriteCB.Get()->transform = world;
    m_spriteCB.Get()->color = color;
    m_spriteCB.Write();

    // 頂点作成、基準点(Pivot)分ずらす
    float x1 = -(pivot.x * rect.x);
    float y1 = -(pivot.y * rect.y);
    float x2 = rect.x - pivot.x * rect.x;
    float y2 = rect.y - pivot.y * rect.y;

    Vertex vertex[] = {
        { {x1, y1, 0}, {uv_min.x, uv_max.y} },
        { {x1, y2, 0}, {uv_min.x, uv_min.y} },
        { {x2, y1, 0}, {uv_max.x, uv_max.y} },
        { {x2, y2, 0}, {uv_max.x, uv_min.y} },
    };
    DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, 4, sizeof(Vertex), vertex);

    // セットしたテクスチャを解除しておく
    ID3D11ShaderResourceView* srv = nullptr;
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, &srv);
}

void SpriteShader::DrawPoint(const Math::Matrix& world, const Math::Color& color)
{
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, DirectX11System::Instance().GetWhiteTexture()->GetSrvAddress());

    m_spriteCB.Get()->transform = world;
    m_spriteCB.Get()->color = color;
    m_spriteCB.Write();
    
    Vertex vertex[] = {
        { {0, 0, 0},{0, 0} },
    };
    DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_POINTLIST, 1, sizeof(Vertex), vertex);
}

void SpriteShader::DrawLine(const Math::Matrix& world, const Math::Color& color)
{
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, DirectX11System::Instance().GetWhiteTexture()->GetSrvAddress());

    m_spriteCB.Get()->transform = world;
    m_spriteCB.Get()->color = color;
    m_spriteCB.Write();
    
    Vertex vertex[] = {
        { {-0.5f, 0, 0}, {0, 0} },
        { {0.5f, 0, 0}, {1, 0} },
    };
    DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, 2, sizeof(Vertex), vertex);
}

void SpriteShader::DrawTriangle(const Math::Matrix& world, const Math::Vector2& pos1, const Math::Vector2& pos2, const Math::Vector2& pos3, const Math::Color& color, bool fill)
{
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, DirectX11System::Instance().GetWhiteTexture()->GetSrvAddress());

    m_spriteCB.Get()->transform = world;
    m_spriteCB.Get()->color = color;
    m_spriteCB.Write();

    Vertex vertex[] = {
        { {pos1.x, pos1.y, 0}, {0, 0} },
        { {pos2.x, pos2.y, 0}, {1, 0} },
        { {pos3.x, pos3.y, 0}, {0, 0} },
        { {pos1.x, pos1.y, 0}, {1, 0} },
    };
    DirectX11System::Instance().DrawVertices(fill ? D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP : D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, 4, sizeof(Vertex), vertex);
}

void SpriteShader::DrawBox(const Math::Matrix& world, const Math::Vector2& half_extent, const Math::Color& color, bool fill)
{
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, DirectX11System::Instance().GetWhiteTexture()->GetSrvAddress());

    m_spriteCB.Get()->transform = world;
    m_spriteCB.Get()->color = color;
    m_spriteCB.Write();

    Math::Vector3 p1 = { -half_extent.x, -half_extent.y, 0 };
    Math::Vector3 p2 = { -half_extent.x, +half_extent.y, 0 };
    Math::Vector3 p3 = { +half_extent.x, +half_extent.y, 0 };
    Math::Vector3 p4 = { +half_extent.x, -half_extent.y, 0 };

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

void SpriteShader::DrawCircle(const Math::Matrix& world, float radius, const Math::Color& color, bool fill)
{
    if (radius <= 0.f) return;

    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 1, DirectX11System::Instance().GetWhiteTexture()->GetSrvAddress());

    m_spriteCB.Get()->transform = world;
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
            vertex[idx].pos.x = 0;
            vertex[idx].pos.y = 0;
            
            vertex[idx + 1].pos.x = std::cos(DirectX::XMConvertToRadians(i * (360.0f / (face_count - 1)))) * radius;
            vertex[idx + 1].pos.y = std::sin(DirectX::XMConvertToRadians(i * (360.0f / (face_count - 1)))) * radius;
            vertex[idx + 1].pos.z = 0;
            
            vertex[idx + 2].pos.x = std::cos(DirectX::XMConvertToRadians((i + 1) * (360.0f / (face_count - 1)))) * radius;
            vertex[idx + 2].pos.y = std::sin(DirectX::XMConvertToRadians((i + 1) * (360.0f / (face_count - 1)))) * radius;
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
            vertex[i].pos.x = cos(DirectX::XMConvertToRadians(i * (360.0f / (vertex_count - 1)))) * radius;
            vertex[i].pos.y = sin(DirectX::XMConvertToRadians(i * (360.0f / (vertex_count - 1)))) * radius;
            vertex[i].pos.z = 0;
        }
        DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_LINESTRIP, vertex_count, sizeof(Vertex), &vertex[0]);
    }
}

bool SpriteFont::Init()
{
    m_upSpriteBatch = std::make_unique<DirectX::SpriteBatch>(DirectX11System::Instance().GetCtx().Get());
    return true;
}

void SpriteFont::Draw(std::string_view font_name, std::string_view text, const Math::Vector2& pos, bool align_center, const Math::Color& color, float scale)
{
    // フォントデータを取得
    if (auto iter = m_upSpriteFonts.find(font_name.data()); iter != m_upSpriteFonts.end()) {
        // 文字列を作成
        auto wtext = sjis_to_wide(text.data());
        // 中心座標を計算
        Math::Vector2 origin;
        if (align_center) {
            origin = convert::ToHalf(Math::Vector2(iter->second->MeasureString(wtext.data())));
        }
        // 画面中央を(0, 0)とするためにViewportから補正値を計算
        auto v = DirectX11System::Instance().GetViewport();
        Math::Vector2 offset_pos(pos.x + convert::ToHalf(v.Width), -pos.y + convert::ToHalf(v.Height));

        // 描画
        iter->second->DrawString(m_upSpriteBatch.get(), wtext.data(), offset_pos, color, 0.f, origin, scale);
    }
}

void SpriteFont::Begin()
{
    auto ctx = DirectX11System::Instance().GetCtx();

    ctx->OMGetBlendState(&m_saveState.pBlendState, m_saveState.factor, &m_saveState.mask);
    ctx->CSGetSamplers(0, 1, &m_saveState.pSamplerState);
    ctx->OMGetDepthStencilState(&m_saveState.pDepthStencilState, &m_saveState.stencilRef);
    ctx->RSGetState(&m_saveState.pRasterizerState);
    m_upSpriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_saveState.pBlendState, m_saveState.pSamplerState, m_saveState.pDepthStencilState, m_saveState.pRasterizerState);
}

void SpriteFont::End()
{
    m_upSpriteBatch->End();
    memory::SafeRelease(&m_saveState.pBlendState);
    memory::SafeRelease(&m_saveState.pSamplerState);
    memory::SafeRelease(&m_saveState.pDepthStencilState);
    memory::SafeRelease(&m_saveState.pRasterizerState);
}
