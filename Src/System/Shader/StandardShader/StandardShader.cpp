#include "StandardShader.h"

void StandardShader::SetToDevice()
{
    DirectX11System::Instance().GetCtx()->PSSetShader(m_PS, 0, 0);

    // 定数バッファをセット
    DirectX11System::Instance().GetCtx()->VSSetConstantBuffers(0, 1, m_cb0.GetBufferAddress());
    DirectX11System::Instance().GetCtx()->PSSetConstantBuffers(0, 1, m_cb0.GetBufferAddress());

    DirectX11System::Instance().GetCtx()->VSSetConstantBuffers(1, 1, m_cb1_Mesh.GetBufferAddress());
    DirectX11System::Instance().GetCtx()->PSSetConstantBuffers(2, 1, m_cb2_Material.GetBufferAddress());
    DirectX11System::Instance().GetCtx()->VSSetConstantBuffers(3, 1, m_cb3_Bones.GetBufferAddress());
    DirectX11System::Instance().GetCtx()->PSSetConstantBuffers(4, 1, m_cb4_RimLight.GetBufferAddress());
}

void StandardShader::DrawMesh(const KdMesh* mesh, const Math::Matrix& mWorld, const std::vector<KdMaterial>& materials)
{
    if (mesh == nullptr) { return; }

    // 行列セット
    SetWorldMatrix(mWorld);

    m_cb1_Mesh.Write();

    // StaticMeshとSkinMeshによって頂点シェーダーと入力レイアウトを切り替える
    if (mesh->IsSkinMesh()) {
        // SkinMesh用の頂点シェーダーをセット
        if (DirectX11System::Instance().GetShaderManager()->SetVertexShader(m_VS_Skin)) {
            // 頂点レイアウトをセット
            DirectX11System::Instance().GetCtx()->IASetInputLayout(m_inputLayout_Skin);
        }
    }
    else {
        // StaticMesh用の頂点シェーダーをセット
        if (DirectX11System::Instance().GetShaderManager()->SetVertexShader(m_VS)) {
            // 頂点レイアウトをセット
            DirectX11System::Instance().GetCtx()->IASetInputLayout(m_inputLayout);
        }
    }

    // メッシュ情報をセット
    mesh->SetToDevice();

    // 全サブセット
    for (UINT subi = 0; subi < mesh->GetSubsets().size(); subi++) {
        // 面が１枚も無い場合はスキップ
        if (mesh->GetSubsets()[subi].FaceCount == 0) { continue; }

        // マテリアルセット
        const KdMaterial& material = materials[mesh->GetSubsets()[subi].MaterialNo];

        SetMaterial(material);

        m_cb2_Material.Write();

        //-----------------------
        // サブセット描画
        //-----------------------
        mesh->DrawSubset(subi);
    }
}

void StandardShader::DrawModel(const KdModelWork& rModel, const Math::Matrix& mWorld, std::initializer_list<std::string_view> invisible_nodes)
{
    // 有効じゃないときはスキップ
    if (!rModel.IsEnable()) { return; }

    const std::shared_ptr<KdModelData>& data = rModel.GetData();

    // モデルがないときはスキップ
    if (data == nullptr) { return; }

    auto& workNodes = rModel.GetNodes();
    auto& dataNodes = data->GetOriginalNodes();

    // スキンメッシュモデルの場合：ボーン情報を書き込み
    if (data->IsSkinMesh()) {
        // ノード内からボーン情報を取得
        for (auto&& nodeIdx : data->GetBoneNodeIndices()) {
            if (nodeIdx >= StandardShader::maxBoneBufferSize) { assert(0 && "転送できるボーンの上限数を超えました"); return; }

            auto& dataNode = dataNodes[nodeIdx];
            auto& workNode = workNodes[nodeIdx];

            // ボーン情報からGPUに渡す行列の計算
            BoneCB().Get()->mBones[dataNode.m_boneIndex] = dataNode.m_boneInverseWorldMatrix * workNode.m_worldTransform;

            BoneCB().Write();
        }
    }

    // 全メッシュノードを描画
    for (auto& nodeIdx : data->GetMeshNodeIndices()) {
        auto& rWorkNode = workNodes[nodeIdx];
        bool is_find = false;

        for (const auto& e : invisible_nodes) {
            if (rWorkNode.m_name == e) {
                is_find = true;
                break;
            }
        }

        if (is_find) continue;

        const std::shared_ptr<KdMesh>& spMesh = rModel.GetMesh(nodeIdx);

        // 描画
        DrawMesh(spMesh.get(), rWorkNode.m_worldTransform * mWorld, data->GetMaterials());
    }
}

void StandardShader::DrawPolygon(const KdPolygon& poly, const Math::Matrix& mWorld)
{
    // 有効じゃないときはスキップ
    if (!poly.IsEnable()) { return; }

    m_cb0.Write();

    std::vector<KdPolygon::Vertex> vertices;

    // ポリゴン描画用の頂点生成
    poly.GenerateVertices(vertices);

    // 頂点数が3より少なければポリゴンが形成できないので描画不能
    if (vertices.size() < 3) { return; }

    // StaticMesh用の頂点シェーダーをセット
    if (DirectX11System::Instance().GetShaderManager()->SetVertexShader(m_VS)) {
        // 頂点レイアウトをセット
        DirectX11System::Instance().GetCtx()->IASetInputLayout(m_inputLayout);
    }

    SetWorldMatrix(mWorld);

    m_cb1_Mesh.Write();

    if (poly.GetMaterial())
    {
        SetMaterial(*poly.GetMaterial());
    }
    else
    {
        SetMaterial(KdMaterial());
    }

    m_cb2_Material.Write();

    DirectX11System::Instance().GetShaderManager()->ChangeSamplerState(0, DirectX11System::Instance().GetShaderManager()->m_ss_Anisotropic_Clamp);

    // 指定した頂点配列を描画する関数
    DirectX11System::Instance().DrawVertices(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, vertices.size(), sizeof(KdPolygon::Vertex), &vertices[0]);

    DirectX11System::Instance().GetShaderManager()->UndoSamplerState();
}

bool StandardShader::Init()
{
    //-------------------------------------
    // 頂点シェーダ
    //-------------------------------------
    {
        // コンパイル済みのシェーダーヘッダーファイルをインクルード
#include "StandardShader_VS.inc"

// 頂点シェーダー作成
        if (FAILED(DirectX11System::Instance().GetDev()->CreateVertexShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_VS))) {
            assert(0 && "頂点シェーダー作成失敗");
            Release();
            return false;
        }

        // １頂点の詳細な情報
        std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,		0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        // 頂点入力レイアウト作成
        if (FAILED(DirectX11System::Instance().GetDev()->CreateInputLayout(
            &layout[0],				// 入力エレメント先頭アドレス
            layout.size(),			// 入力エレメント数
            &compiledBuffer[0],		// 頂点バッファのバイナリデータ
            sizeof(compiledBuffer),	// 上記のバッファサイズ
            &m_inputLayout))
            ) {
            assert(0 && "CreateInputLayout失敗");
            Release();
            return false;
        }
    }
    //-------------------------------------
    // 頂点シェーダ
    //-------------------------------------
    {
        // コンパイル済みのシェーダーヘッダーファイルをインクルード
#include "StandardShader_VS_Skin.inc"

// 頂点シェーダー作成
        if (FAILED(DirectX11System::Instance().GetDev()->CreateVertexShader(compiledBuffer, sizeof(compiledBuffer), nullptr, &m_VS_Skin))) {
            assert(0 && "頂点シェーダー作成失敗");
            Release();
            return false;
        }

        // １頂点の詳細な情報
        std::vector<D3D11_INPUT_ELEMENT_DESC> layout = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 20,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,		0, 32,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 36,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "SKININDEX",	0, DXGI_FORMAT_R16G16B16A16_UINT,	0, 48,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "SKINWEIGHT",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 56,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        // 頂点入力レイアウト作成
        if (FAILED(DirectX11System::Instance().GetDev()->CreateInputLayout(
            &layout[0],				// 入力エレメント先頭アドレス
            layout.size(),			// 入力エレメント数
            &compiledBuffer[0],		// 頂点バッファのバイナリデータ
            sizeof(compiledBuffer),	// 上記のバッファサイズ
            &m_inputLayout_Skin))
            ) {
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
#include "StandardShader_PS.inc"

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
    m_cb1_Mesh.Create();
    m_cb2_Material.Create();
    m_cb3_Bones.Create();
    m_cb4_RimLight.Create();

    return true;
}

void StandardShader::Release()
{
    memory::SafeRelease(&m_VS);
    memory::SafeRelease(&m_VS_Skin);
    memory::SafeRelease(&m_inputLayout);
    memory::SafeRelease(&m_inputLayout_Skin);
    memory::SafeRelease(&m_PS);

    m_cb0.Release();
    m_cb1_Mesh.Release();
    m_cb2_Material.Release();
    m_cb3_Bones.Release();
    m_cb4_RimLight.Release();
}

void StandardShader::SetMaterial(const KdMaterial& material)
{
    //-----------------------
    // マテリアル情報を定数バッファへ書き込む
    //-----------------------
    m_cb2_Material.Get()->BaseColor = material.BaseColor;
    m_cb2_Material.Get()->Emissive  = material.Emissive;
    m_cb2_Material.Get()->Metallic  = material.Metallic;
    m_cb2_Material.Get()->Roughness = material.Roughness;

    //-----------------------
    // テクスチャセット
    //-----------------------
    ID3D11ShaderResourceView* srvs[3] = {};

    // BaseColor
    srvs[0] = material.BaseColorTex->GetSrv();
    // Emissive
    srvs[1] = material.EmissiveTex->GetSrv();
    // Metallic Roughness
    srvs[2] = material.MetallicRoughnessTex->GetSrv();

    // セット
    DirectX11System::Instance().GetCtx()->PSSetShaderResources(0, _countof(srvs), srvs);
}
