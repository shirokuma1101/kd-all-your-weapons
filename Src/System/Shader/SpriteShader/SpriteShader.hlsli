/**************************************************
*
* SpriteShader
*
**************************************************/

struct VSIn
{
    float4 Pos : POSITION;
    float2 UV  : TEXCOORD0;
};

struct PSIn
{
    float4 Pos : SV_POSITON;
    float2 UV  : TEXCOORD0;
};

// 定数バッファ
cbuffer SpriteCB : register(b2)
{
    row_major float4x4 Transform; // 変換行列
    float4             Color;
};

cbuffer ProjectionCB : register(b3)
{
    row_major float4x4 Projection; // 射影変換行列
};

