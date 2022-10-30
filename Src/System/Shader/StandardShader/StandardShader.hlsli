/**************************************************
*
* StandardShader
*
**************************************************/

struct VSIn
{
    float4 Pos        : POSITION;   // 頂点座標
    float2 UV         : TEXCOORD0;  // テクスチャUV座標
    float3 Normal     : NORMAL;     // 法線ベクトル
    float4 Color      : COLOR;      // 頂点色
    float3 Tangent    : TANGENT;    // 接線
    uint4  SkinIndex  : SKININDEX;  // スキンメッシュのボーンインデックス
    float4 SkinWeight : SKINWEIGHT; // ボーンの影響度
};

struct PSIn
{
    float4 Pos           : SV_POSITION; // 射影座標
    float2 UV            : TEXCOORD0;   // UV座標
    float4 Color         : TEXCOORD1;   // 色
    float3 WorldPos      : TEXCOORD2;   // ワールド3D座標
    float3 WorldNormal   : TEXCOORD3;   // ワールド法線
    float3 WorldTangent  : TEXCOORD4;   // ワールド接線
    float3 WorldBinormal : TEXCOORD5;   // ワールド従法線
};

struct ShadowPSIn
{
    float4 Pos         : SV_POSITION; // 射影座標
    float2 UV          : TEXCOORD0;   // UV座標
    float4 Color       : TEXCOORD1;   // 色
    float4 WorldPos    : TEXCOORD2;   // ワールド3D座標
};

/* Object */
cbuffer ObjectCB : register(b2)
{
    // UV
    float2 UVOffset; // UVオフセット
    float2 UVTiling; // UVスケーリング
    
    int    LightEnable;       // ライト有効
    
    int    FogEnable;         // フォグ有効

    float  DissolveThreshold; // ディゾルブ閾値
};

/* Mesh */
cbuffer MeshCB : register(b3)
{
    row_major float4x4 World; // ワールド変換行列
};

/* Material */
cbuffer MaterialCB : register(b4)
{
    float4 BaseColor; // ベース色
    float3 Emissive;  // 自己発光色
    float  Metallic;  // 金属度
    float  Roughness; // 粗さ
};

/* Bones */
cbuffer BonesCB : register(b5)
{
    int                BonesEnable; // ボーン有効
    row_major float4x4 Bones[128];
};

/* RimLight */
cbuffer RimLightCB : register(b6)
{
    float3 RimColor;
    float  RimPower;
};
