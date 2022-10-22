#include "SpriteShader.hlsli"

// テクスチャ
Texture2D Tex : register(t0);
// サンプラ
SamplerState Sampler : register(s0);

float4 main(PSIn ps_in) : SV_Target0
{
    // テクスチャ色取得
    float4 tex_color = Tex.Sample(Sampler, ps_in.UV);
    //アルファテスト
    if (tex_color.a < 0.1f)
    {
        discard;
    }
    
    // テクスチャ色 * 指定色
    return tex_color * Color;
}
