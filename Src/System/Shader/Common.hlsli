/**************************************************
*
* Common
*
**************************************************/

/* Camera */
cbuffer CameraCB : register(b0)
{
    // カメラ情報
    row_major float4x4 View;       // ビュー行列
    row_major float4x4 Projection; // 射影行列
    float3             Position;   // カメラのワールド座標

    // フォグ
    int    DistanceFogEnable; // 距離フォグ有効
    float3 DistanceFogColor;  // 距離フォグ色
    float  DistanceFogStart;  // 距離フォグ開始距離
    float  DistanceFogEnd;    // 距離フォグ終了距離
};

/* Light */
cbuffer LightCB : register(b1)
{
    // 環境光
    float3 AmbientLight;

    // 平行光
    float3             DirectionalLightDirection; // 方向
    float3             DirectionalLightColor;     // 色
    row_major float4x4 DirectionalLightVP;        // ビュー行列 x 射影行列

    // 点光源
    int PointLightCount; // 点光源数
    struct PointLight
    {
        float3 Position;    // 位置
        float3 Color;       // 色
        float3 Attenuation; // 減衰率
    } PointLights[8];       // 点光源
};
