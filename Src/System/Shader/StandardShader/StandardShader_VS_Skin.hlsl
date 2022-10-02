#include "../inc_Common.hlsli"
#include "inc_StandardShader.hlsli"

VSOutput main(
	float4 pos : POSITION,	// 頂点座標
	float2 uv : TEXCOORD0,	// テクスチャUV座標
	float3 normal : NORMAL,	// 法線ベクトル
	float4 col : COLOR,		// 頂点色
	uint4 skinIndex : SKININDEX,	// スキンメッシュのボーンインデックス
	float4 skinWeight : SKINWEIGHT	// ボーンの影響度
)
{
	// スキニング---------------->
	// 行列を合成
	row_major float4x4 mBones = 0; // 行列を0埋め
	[unroll]
	for (int i = 0; i < 4; i++)
	{
		mBones += g_mBones[skinIndex[i]] * skinWeight[i];
	}
	
	// 座標と法線に適用
	pos = mul(pos, mBones);
	normal = mul(normal, (float3x3)mBones);
	
	// <----------------スキニング
	
	VSOutput Out;
	
	// キャラクターの座標変換 : ローカル座標系 -> ワールド座標系へ変換
	Out.Pos = mul(pos, g_mWorld);
	Out.wPos = Out.Pos.xyz;
	
	// カメラの逆向きに変換 : ワールド座標系 -> ビュー座標系へ変換
	Out.Pos = mul(Out.Pos, g_mView);
	
	// 射影空間に変換 : ビュー座標系 -> 射影座標系へ変換
	Out.Pos = mul(Out.Pos, g_mProj);
	
	// UVと頂点色はそのまま
	Out.UV = uv;
	Out.Color = col;
	
	// 法線
	Out.wN = mul(normal, (float3x3) g_mWorld);
	return Out;
}
