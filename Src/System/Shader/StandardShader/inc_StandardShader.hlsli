//===========================================
//
// 定数バッファ
//  ゲームプログラム側から持ってくるデータ
//
//===========================================

// 定数バッファ(オブジェクト単位更新)
cbuffer cbObject : register(b0)
{
	// UV操作
	float2  g_UVOffset;
	float2  g_UVTiling;

	// ライト有効
	int     g_LightEnable;
	
	// フォグ有効
	int     g_FogEnable;
};

// 定数バッファ(メッシュ単位更新)
cbuffer cbObject : register(b1)
{
	row_major float4x4 g_mWorld; // ワールド変換行列
};

// 定数バッファ(マテリアル)
cbuffer cbMaterial : register(b2)
{
	float4 g_BaseColor;	// ベース色
	float3 g_Emissive;	// 自己発光色
	float  g_Metallic;	// 金属度
	float  g_Roughness;	// 粗さ
};

// ボーン行列配列
cbuffer cbBones : register(b3)
{
	row_major float4x4 g_mBones[300];
};

// 定数バッファ(リムライト用)
cbuffer cbRimLight : register(b4)
{
    float3 g_RimColor;
    float g_RimPower;
};

//===========================================
//
// 頂点シェーダから出力するデータ
//
//===========================================
struct VSOutput
{
	float4 Pos : SV_Position;   // 射影座標
	float2 UV : TEXCOORD0;      // UV座標
	float3 wN : TEXCOORD1;      // ワールド法線
	float4 Color : TEXCOORD2;   // 色
	float3 wPos : TEXCOORD3;    // ワールド3D座標
};