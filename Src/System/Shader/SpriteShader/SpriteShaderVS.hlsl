#include "SpriteShader.hlsli"

PSIn main(VSIn vs_in)
{
    PSIn ps_in;

    // 頂点座標を射影変換
    ps_in.Pos = mul(vs_in.Pos, Transform);
    ps_in.Pos = mul(ps_in.Pos, Projection);

    // UV座標
    ps_in.UV = ps_in.UV;

    return ps_in;
}
