#include "../Common.hlsli"
#include "StandardShader.hlsli"

ShadowPSIn main(VSIn vs_in)
{
    if (BonesEnable)
    {
        /* スキニング */
        // 行列を合成
        row_major float4x4 bones = 0; // 行列を0埋め
        [unroll]
        for (int i = 0; i < 4; i++)
        {
            bones += Bones[vs_in.SkinIndex[i]] * vs_in.SkinWeight[i];
        }
        // 座標と法線に適用
        vs_in.Pos = mul(vs_in.Pos, bones);
    }
    
    ShadowPSIn shadow_ps_in;
    
    // キャラクターの座標変換 : ローカル座標系 -> ワールド座標系へ変換
    shadow_ps_in.Pos = mul(vs_in.Pos, World);
    // カメラの逆向きに変換 : ワールド座標系 -> ビュー射影座標系へ変換
    shadow_ps_in.Pos = mul(shadow_ps_in.Pos, DirectionalLightVP);
    // 射影行列を変換されないように保存
    shadow_ps_in.WorldPos = shadow_ps_in.Pos;
    
    // UV座標
    //shadow_ps_in.UV = vs_in.UV * UVTiling + UVOffset;
    shadow_ps_in.UV = vs_in.UV;

    // 頂点色
    shadow_ps_in.Color = vs_in.Color;

    // 出力
    return shadow_ps_in;
}
