#pragma once

class StandardShader
{
public:

    StandardShader() {}
    ~StandardShader() {
        Release();
    }

    bool Init();

    void Release();

    auto& GetObjectCB() noexcept {
        return m_objectCB;
    }
    auto& GetObjectCB() const noexcept {
        return m_objectCB;
    }
    auto& GetMeshCB() noexcept {
        return m_meshCB;
    }
    auto& GetMeshCB() const noexcept {
        return m_meshCB;
    }
    auto& GetMaterialCB() noexcept {
        return m_materialCB;
    }
    auto& GetMaterialCB() const noexcept {
        return m_materialCB;
    }
    auto& GetBonesCB() noexcept {
        return m_bonesCB;
    }
    auto& GetBonesCB() const noexcept {
        return m_bonesCB;
    }
    auto& GetRimLightCB() noexcept {
        return m_rimLightCB;
    }
    auto& GetRimLightCB() const noexcept {
        return m_rimLightCB;
    }

    void SetToDevice();
    
    void CreateShadow(const std::pair<int32_t, int32_t>& size);
    bool BeginShadow();
    void EndShadow();
    void ClearShadow();

    void BeginStandard();
    void EndStandard();

    void DrawPolygon(const KdPolygon& poly, const Math::Matrix& world);

    void DrawMesh(const KdMesh& mesh, const Math::Matrix& world, const std::vector<KdMaterial>& materials, float alpha = 1.f);

    void DrawModel(const KdModelWork& model, const Math::Matrix& world, const std::initializer_list<std::string_view>& invisible_nodes = {}, float alpha = 1.f);
    
private:

    // オブジェクト定数バッファ
    struct ObjectCB {
        Math::Vector2 uvOffset = { 0.f, 0.f }; //  8byte: UVオフセット
        Math::Vector2 uvTiling = { 1.f, 1.f }; //  8byte: UVタイリング

        int   lightEnable       = 1;           //  4byte: ライト有効
        int   fogEnable         = 1;           //  4byte: フォグ有効
        float dissolveThreshold = 0.f;         //  4byte: ディゾルブ閾値
        DIRECTX11_HELPER_PADDING_4BYTE(0);     //  4byte: パディング0
    }; // 32byte

    // メッシュ定数バッファ
    struct MeshCB {
        Math::Matrix world; // 64byte: ワールド行列
    }; // 64byte

    // マテリアル定数バッファ
    struct MaterialCB {
        Math::Vector4 baseColor;            // 16byte: ベースカラー
        Math::Vector3 emissive;             // 12byte: エミッシブ
        float         metallic  = 0.f;      //  4byte: メタリック
        float         roughness = 1.f;      //  4byte: ラフネス

        DIRECTX11_HELPER_PADDING_12BYTE(0); // 12byte: パディング0
    }; // 48byte

    // ボーン定数バッファ
    static constexpr int BONES_MAX = 128;
    struct BonesCB {
        int          bonesEnable = 0;       //     4byte: ボーン有効
        DIRECTX11_HELPER_PADDING_12BYTE(0); //    12byte: パディング0
        Math::Matrix bones[BONES_MAX];      // 19200byte: ボーン行列
    }; // 19216byte

    // リムライト定数バッファ
    struct RimLightCB {
        Math::Vector3 rimColor = { 1.f, 1.f, 1.f }; // 12byte: リムカラー
        float         rimPower = 0.f;               //  4byte: リムパワー
    }; // 16byte

    void SetMaterial(const KdMaterial& material = {}, float alpha = 1.f);
    
    DirectX11ConstantBufferSystem<ObjectCB>   m_objectCB;
    DirectX11ConstantBufferSystem<MeshCB>     m_meshCB;
    DirectX11ConstantBufferSystem<MaterialCB> m_materialCB;
    DirectX11ConstantBufferSystem<BonesCB>    m_bonesCB;
    DirectX11ConstantBufferSystem<RimLightCB> m_rimLightCB;

    ID3D11VertexShader* m_pVS              = nullptr;
    ID3D11InputLayout*  m_pInputLayout     = nullptr;
    ID3D11PixelShader*  m_pPS              = nullptr;

    ID3D11VertexShader* m_pShadowVS        = nullptr;
    ID3D11PixelShader*  m_pShadowPS        = nullptr;
    std::unique_ptr<DirectX11RenderTargetSystem> m_upShadowRT = nullptr;
    DirectX11RenderTargetChangerSystem           m_shadowRtc;
    
};
