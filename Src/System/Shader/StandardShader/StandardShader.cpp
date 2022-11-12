#include "StandardShader.h"

bool StandardShader::Init()
{
    auto dev = DirectX11System::Instance().GetDev();

    {
        #include "StandardShaderVS.inc"
        if (FAILED(dev->CreateVertexShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_pVS))) {
            assert::ShowError(ASSERT_FILE_LINE, "頂点シェーダー作成失敗");
            Release();
            return false;
        }
        
        std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
            { "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,       0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",      0, DXGI_FORMAT_R8G8B8A8_UNORM,     0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TANGENT",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "SKININDEX",	0, DXGI_FORMAT_R16G16B16A16_UINT,  0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "SKINWEIGHT",	0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };
        
        if (FAILED(dev->CreateInputLayout(
            &layout[0],
            static_cast<UINT>(layout.size()),
            &compiledBuffer[0],
            sizeof(compiledBuffer),
            &m_pInputLayout
        ))) {
            assert::ShowError(ASSERT_FILE_LINE, "頂点入力レイアウト作成失敗");
            Release();
            return false;
        }
    }
    {
        #include "StandardShaderPS.inc"
        if (FAILED(dev->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_pPS))) {
            assert::ShowError(ASSERT_FILE_LINE, "ピクセルシェーダー作成失敗");
            Release();
            return false;
        }
    }
    {
        #include "StandardShaderShadowVS.inc"
        if (FAILED(dev->CreateVertexShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_pShadowVS))) {
            assert::ShowError(ASSERT_FILE_LINE, "頂点シェーダー作成失敗");
            Release();
            return false;
        }
    }
    {
        #include "StandardShaderShadowPS.inc"
        if (FAILED(dev->CreatePixelShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_pShadowPS))) {
            assert::ShowError(ASSERT_FILE_LINE, "ピクセルシェーダー作成失敗");
            Release();
            return false;
        }
    }
    
    m_objectCB.Create();
    m_meshCB.Create();
    m_materialCB.Create();
    m_bonesCB.Create();
    m_rimLightCB.Create();

    return true;
}

void StandardShader::SetToDevice()
{
    auto ctx = DirectX11System::Instance().GetCtx();

    ctx->IASetInputLayout(m_pInputLayout);
    
    ctx->VSSetConstantBuffers(2, 1, m_objectCB.GetBufferAddress());
    ctx->PSSetConstantBuffers(2, 1, m_objectCB.GetBufferAddress());
    ctx->VSSetConstantBuffers(3, 1, m_meshCB.GetBufferAddress());
    ctx->PSSetConstantBuffers(4, 1, m_materialCB.GetBufferAddress());
    ctx->VSSetConstantBuffers(5, 1, m_bonesCB.GetBufferAddress());
    ctx->PSSetConstantBuffers(6, 1, m_rimLightCB.GetBufferAddress());

    ctx->PSSetSamplers(1, 1, &DirectX11System::Instance().GetShaderManager()->m_pSSLinearClampComp);
}

void StandardShader::CreateShadow(const std::pair<int32_t, int32_t>& size)
{
    m_upShadowRT = std::make_unique<DirectX11RenderTargetSystem>();
    m_upShadowRT->Create(size, true, DXGI_FORMAT_R32_FLOAT);
}

bool StandardShader::BeginShadow()
{
    if (!m_upShadowRT) {
        return false;
    }

    auto ctx = DirectX11System::Instance().GetCtx();

    m_upShadowRT->Clear();
    m_shadowRtc.Change(m_upShadowRT.get());

    ctx->VSSetShader(m_pShadowVS, 0, 0);
    ctx->PSSetShader(m_pShadowPS, 0, 0);

    return true;
}

void StandardShader::EndShadow()
{
    if (!m_upShadowRT) return;

    m_shadowRtc.Undo();
}

void StandardShader::ClearShadow()
{
    m_upShadowRT.reset();
    m_shadowRtc.Undo(true);
}

void StandardShader::BeginStandard()
{
    auto ctx = DirectX11System::Instance().GetCtx();

    ctx->VSSetShader(m_pVS, 0, 0);
    ctx->PSSetShader(m_pPS, 0, 0);

    if (m_upShadowRT) {
        ctx->PSSetShaderResources(10, 1, m_upShadowRT->GetBackBuffer()->GetSrvAddress());
    }
}

void StandardShader::EndStandard()
{
    ID3D11ShaderResourceView* srv = nullptr;
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(10, 1, &srv); // 影のテクスチャをクリア
}

void StandardShader::DrawPolygon(const KdPolygon& poly, const Math::Matrix& world)
{
    if (!poly.IsEnable()) return;

    m_materialCB.Write();
    
    std::vector<KdPolygon::Vertex> vertices;

    // ポリゴン描画用の頂点生成
    poly.GenerateVertices(vertices);

    // 頂点数が3より少なければポリゴンが形成できないので描画不能
    if (vertices.size() < 3) return;

    m_meshCB.Get()->world = world;
    m_meshCB.Write();

    if (auto material = poly.GetMaterial(); material) {
        SetMaterial(*material);
    }
    else {
        SetMaterial();
    }

    DirectX11System::WorkInstance().GetShaderManager()->ChangeSamplerState(0, DirectX11System::Instance().GetShaderManager()->m_pSSAnisotropicClamp);
    
    DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, static_cast<UINT>(vertices.size()), sizeof(KdPolygon::Vertex), &vertices[0]);

    DirectX11System::WorkInstance().GetShaderManager()->UndoSamplerState();
}

void StandardShader::DrawMesh(const KdMesh& mesh, const Math::Matrix& world, const std::vector<KdMaterial>& materials)
{
    m_meshCB.Get()->world = world;
    m_meshCB.Write();
    
    // メッシュ情報をセット
    mesh.SetToDevice();

    // 全サブセット
    for (UINT i = 0; i < mesh.GetSubsets().size(); ++i) {
        // 面が１枚も無い場合はスキップ
        if (mesh.GetSubsets()[i].FaceCount == 0) continue;

        // マテリアルセット
        const KdMaterial& material = materials[mesh.GetSubsets()[i].MaterialNo];
        SetMaterial(material);
        
        // サブセット描画
        mesh.DrawSubset(i);
    }
}

void StandardShader::DrawModel(const KdModelWork& model, const Math::Matrix& world, const std::initializer_list<std::string_view>& invisible_nodes)
{
    if (!model.IsEnable()) return;

    const std::shared_ptr<KdModelData>& data = model.GetData();
    
    if (!data) return;

    auto& work_nodes = model.GetNodes();
    auto& data_nodes = data->GetOriginalNodes();

    // スキンメッシュモデルの場合：ボーン情報を書き込み
    if (data->IsSkinMesh()) {
        // ノード内からボーン情報を取得
        for (const auto& i : data->GetBoneNodeIndices()) {
            if (i >= BONES_MAX) {
                assert::ShowError(ASSERT_FILE_LINE, "転送できるボーンの上限数を超えました");
                return;
            }

            const auto& data_node = data_nodes[i];
            const auto& work_node = work_nodes[i];

            // ボーン情報からGPUに渡す行列の計算
            m_bonesCB.Get()->bonesEnable = true;
            m_bonesCB.Get()->bones[data_node.m_boneIndex] = data_node.m_boneInverseWorldMatrix * work_node.m_worldTransform;
            m_bonesCB.Write();
        }
    }
    else {
        m_bonesCB.Get()->bonesEnable = false;
        m_bonesCB.Write();
    }

    // 全メッシュノードを描画
    for (const auto& i : data->GetMeshNodeIndices()) {
        const auto& work_node = work_nodes[i];
        bool is_find = false;

        for (const auto& e : invisible_nodes) {
            if (work_node.m_name == e) {
                is_find = true;
                break;
            }
        }

        if (is_find) continue;

        if (const auto& mesh = model.GetMesh(i); mesh) {
            DrawMesh(*mesh, work_node.m_worldTransform * world, data->GetMaterials());
        }
    }
}

void StandardShader::SetMaterial(const KdMaterial& material)
{
    // マテリアルセット
    m_materialCB.Get()->baseColor = material.BaseColor;
    m_materialCB.Get()->emissive  = material.Emissive;
    m_materialCB.Get()->metallic  = material.Metallic;
    m_materialCB.Get()->roughness = material.Roughness;
    // テクスチャセット
    ID3D11ShaderResourceView* srvs[] = {
        material.BaseColorTex->GetSrv(),
        material.EmissiveTex->GetSrv(),
        material.MetallicRoughnessTex->GetSrv(),
        material.NormalTex->GetSrv(),
    };
    // セット
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, 4, srvs);
    m_materialCB.Write();
}

void StandardShader::Release()
{
    m_objectCB.Release();
    m_meshCB.Release();
    m_materialCB.Release();
    m_bonesCB.Release();
    m_rimLightCB.Release();

    memory::SafeRelease(&m_pVS);
    memory::SafeRelease(&m_pInputLayout);
    memory::SafeRelease(&m_pPS);
    memory::SafeRelease(&m_pShadowVS);
    memory::SafeRelease(&m_pShadowPS);

    m_upShadowRT.reset();
    m_shadowRtc.Release();
}
