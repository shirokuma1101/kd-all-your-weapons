#pragma once

//============================================================
//
// 基本シェーダ
//
//============================================================
class StandardShader
{
    struct cbObject;
    struct cbMaterial;
    struct cbBone;
    struct cbRimLight;
public:

    StandardShader() {}
    ~StandardShader() {
        Release();
    }

    //================================================
    // 設定・取得
    //================================================

    // Object用定数バッファをそのまま返す
    auto& ObjectCB() {
        return m_cb0;
    }
    // material用定数バッファをそのまま返す
    auto& MaterialCB() {
        return m_cb2_Material;
    }
    // bone用定数バッファをそのまま返す
    auto& BoneCB() {
        return m_cb3_Bones;
    }
    // ライムライト用定数バッファをそのまま返す
    auto& RimLightCB() {
        return m_cb4_RimLight;
    }

    // UVタイリング設定
    void SetUVTiling(const Math::Vector2& tiling) {
        m_cb0.Get()->UVTiling = tiling;
    }
    // UVオフセット設定
    void SetUVOffset(const Math::Vector2& offset) {
        m_cb0.Get()->UVOffset = offset;
    }
    // フォグ有効/無効
    void SetFogEnable(bool enable) {
        m_cb0.Get()->FogEnable = enable;
    }

    // ワールド行列セット
    void SetWorldMatrix(const Math::Matrix& m) {
        m_cb1_Mesh.Get()->mW = m;
    }

    //================================================
    // 描画
    //================================================

    // このシェーダをデバイスへセット
    void SetToDevice();

    // メッシュ描画
    void DrawMesh(const KdMesh* mesh, const Math::Matrix& mWorld, const std::vector<KdMaterial>& materials);

    // モデル描画（静的モデル）
    void DrawModel(const KdModelData& model, const Math::Matrix& mWorld);

    // モデル描画（アニメーション可モデル）
    void DrawModel(const KdModelWork& model, const Math::Matrix& mWorld, std::initializer_list<std::string_view> invisible_nodes = {});

    // 頂点群から成るポリゴン群描画
    void DrawPolygon(const KdPolygon& poly, const Math::Matrix& mWorld);

    //================================================
    // 初期化・解放
    //================================================

    // 初期化
    bool Init();
    // 解放
    void Release();
    
    static const int maxBoneBufferSize = 300;
    
private:

    void SetMaterial(const KdMaterial& material);

    // 3Dモデル用シェーダ
    ID3D11VertexShader* m_VS = nullptr;      // 頂点シェーダー
    ID3D11VertexShader* m_VS_Skin = nullptr; // 頂点シェーダー(スキンメッシュ

    ID3D11InputLayout*  m_inputLayout = nullptr;      // 頂点入力レイアウト
    ID3D11InputLayout*  m_inputLayout_Skin = nullptr; // 頂点入力レイアウト(スキンメッシュ
    
    ID3D11PixelShader*  m_PS = nullptr;               // ピクセルシェーダー

    // 定数バッファ
    //  ※定数バッファは、パッキング規則というものを厳守しなければならない
    //  <パッキング規則> 参考：https://docs.microsoft.com/ja-jp/windows/win32/direct3dhlsl/dx-graphics-hlsl-packing-rules
    //  ・構造体のサイズは16バイトの倍数にする。
    //  ・各項目(変数)は、16バイト境界をまたぐような場合、次のベクトルに配置される。

    // 定数バッファ(オブジェクト単位更新)
    struct cbObject
    {
        // UV操作
        Math::Vector2		UVOffset = { 0,0 };
        Math::Vector2		UVTiling = { 1,1 };

        // フォグ有効
        int					FogEnable = 1;

        float				_blank[3] = { 0.0f, 0.0f, 0.0f };
    };
    DirectX11ConstantBufferSystem<cbObject> m_cb0;

    // 定数バッファ(メッシュ単位更新)
    struct cbMesh
    {
        Math::Matrix		mW;		// ワールド行列　行列は16バイトx4バイトの64バイトなのでピッタリ。
    };
    DirectX11ConstantBufferSystem<cbMesh>	m_cb1_Mesh;

    // 定数バッファ(マテリアル単位更新)
    struct cbMaterial
    {
        Math::Vector4	BaseColor;
        Math::Vector3	Emissive;
        float			Metallic = 0.0f;
        float			Roughness = 1.0f;

        float			_blank[3] = { 0.0f, 0.0f, 0.0f };
    };
    DirectX11ConstantBufferSystem<cbMaterial> m_cb2_Material;

    // 定数バッファ(ボーン単位更新)
    struct cbBone {
        Math::Matrix	mBones[300];
    };
    DirectX11ConstantBufferSystem<cbBone>	m_cb3_Bones;

    // 定数バッファ(リムライト用)
    struct cbRimLight {
        Math::Vector3	RimColor = { 1.f, 1.f, 1.f };
        float           RimPower = 0.0f;
    };
    DirectX11ConstantBufferSystem<cbRimLight> m_cb4_RimLight;

};
