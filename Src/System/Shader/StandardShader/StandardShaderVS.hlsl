#include "../Common.hlsli"
#include "StandardShader.hlsli"

PSIn main(VSIn vs_in)
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
        vs_in.Normal = mul(vs_in.Normal, (float3x3) bones);
    }
    
    PSIn ps_in;
    
    // キャラクターの座標変換 : ローカル座標系 -> ワールド座標系へ変換
    ps_in.Pos = mul(vs_in.Pos, World);
    ps_in.WorldPos = ps_in.Pos.xyz;
    // カメラの逆向きに変換 : ワールド座標系 -> ビュー座標系へ変換
    ps_in.Pos = mul(ps_in.Pos, View);
    // 射影空間に変換 : ビュー座標系 -> 射影座標系へ変換
    ps_in.Pos = mul(ps_in.Pos, Projection);

    // UV座標
    ps_in.UV = vs_in.UV * UVTiling + UVOffset;

    // 頂点色
    ps_in.Color = vs_in.Color;

    // 法線
    ps_in.WorldNormal = normalize(mul(vs_in.Normal, (float3x3) World));
    // 接線
    ps_in.WorldTangent = normalize(mul(vs_in.Tangent, (float3x3) World));
    // 従接線
    float3 binormal = cross(vs_in.Normal, vs_in.Tangent);
    ps_in.WorldBinormal = normalize(mul(binormal, (float3x3) World));
    
    // 出力
    return ps_in;
}
