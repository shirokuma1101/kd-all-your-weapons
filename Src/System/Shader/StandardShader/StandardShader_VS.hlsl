#include "../inc_Common.hlsli"
#include "inc_StandardShader.hlsli"

//================================
// 頂点シェーダ
//================================
VSOutput main( 
	float4 pos : POSITION,		// 頂点座標
	float2 uv : TEXCOORD0,		// テクスチャUV座標
	float3 normal : NORMAL,		// 法線ベクトル
	float4 color : COLOR,		// 頂点色
	float3 tangent : TANGENT	// 接線
){
	VSOutput Out;
	
	// キャラクターの座標変換 : ローカル座標系 -> ワールド座標系へ変換
	Out.Pos = mul(pos, g_mWorld);
	Out.wPos = Out.Pos.xyz;
	
	// カメラの逆向きに変換 : ワールド座標系 -> ビュー座標系へ変換
	Out.Pos = mul(Out.Pos, g_mView);
	
	// 射影空間に変換 : ビュー座標系 -> 射影座標系へ変換
	Out.Pos = mul(Out.Pos, g_mProj);

	// 法線
	Out.wN = normalize(mul(normal, (float3x3)g_mWorld));

	// 頂点色
	Out.Color = color;

	// UV座標
	Out.UV = uv * g_UVTiling + g_UVOffset;

	// 出力
	return Out;
}
