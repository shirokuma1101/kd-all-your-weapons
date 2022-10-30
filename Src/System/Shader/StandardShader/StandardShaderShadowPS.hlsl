#include "../Common.hlsli"
#include "StandardShader.hlsli"

// テクスチャ
Texture2D Tex : register(t0);

// 特殊処理用テクスチャ
Texture2D DissolveTex : register(t11); // ディゾルブテクスチャ

// サンプラーステート
SamplerState Sampler : register(s0);

float4 main(ShadowPSIn ps_in) : SV_Target0
{
    // ディゾルブによる描画スキップ
    //float dissolve = DissolveTex.Sample(SS, In.UV).r;
    //if (dissolve <= DissolveThreshold)
    //{
    //    discard;
    //}
    
    // テクスチャ色取得
    float4 tex_color = Tex.Sample(Sampler, ps_in.UV);
    //アルファテスト
    if (tex_color.a * ps_in.Color.a < 0.05f)
    {
        discard;
    }

    return float4(ps_in.WorldPos.z / ps_in.WorldPos.w, 0.0f, 0.0f, 1.0f);
}
