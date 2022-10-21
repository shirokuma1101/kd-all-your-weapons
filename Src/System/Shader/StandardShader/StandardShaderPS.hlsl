#include "../Common.hlsli"
#include "StandardShader.hlsli"

// テクスチャ
Texture2D BaseTex     : register(t0); // ベースカラーテクスチャ
Texture2D EmissiveTex : register(t1); // エミッシブテクスチャ
Texture2D MRTex       : register(t2); // メタリック/ラフネステクスチャ
Texture2D NormalTex   : register(t3); // 法線マップテクスチャ

// 特殊処理用テクスチャ
Texture2D DissolveTex : register(t11); // ディゾルブテクスチャ

// サンプラーステート
SamplerState Sampler : register(s0);

// BlinnPhong NDF
// ・lightDir    … ライトの方向
// ・vCam        … ピクセルからカメラへの方向
// ・normal      … 法線
// ・specPower   … 反射の鋭さ
float BlinnPhong(float3 lightDir, float3 vCam, float3 normal, float specPower)
{
    float3 H = normalize(-lightDir + vCam);
    // カメラの角度差(0～1)
    float NdotH = saturate(dot(normal, H));
    float spec = pow(NdotH, specPower);

    // 正規化Blinn-Phong
    return spec * ((specPower + 2) / (2 * 3.1415926535));
}

float4 main(PSIn ps_in) : SV_Target0
{
    // ディゾルブによる描画スキップ
    //float dissolve = DissolveTex.Sample(SS, In.UV).r;
    //if (dissolve <= DissolveThreshold)
    //{
    //    discard;
    //}
    
    
    /* カメラ */
    
    float3 cam_dir = Position - ps_in.WorldPos; // カメラへの方向
    float cam_dist = length(cam_dir);        // カメラ - ピクセル距離
    cam_dir = normalize(cam_dir);
    

    /* 材質色 */
    float4 mr = MRTex.Sample(Sampler, ps_in.UV);
    
    // ベースカラー
    float4 base_color = BaseTex.Sample(Sampler, ps_in.UV) * BaseColor * ps_in.Color;
    // 金属性
    float metallic = mr.b * Metallic;
    // 粗さ
    float roughness = mr.g * Roughness;
    // 法線
    float3 normal = normalize(NormalTex.Sample(Sampler, ps_in.UV).rgb * 2 - 1);
    normal = ps_in.WorldTangent * normal.x + ps_in.WorldBinormal * normal.y + ps_in.WorldNormal * normal.z;

    
    /* ライティング */
    
    // 最終的な色
    float3 color = 0;
    
    // 材質の拡散色　非金属ほど材質の色になり、金属ほど拡散色は無くなる
    const float3 base_diffuse = lerp(base_color.rgb, float3(0, 0, 0), metallic);
    // 材質の反射色　非金属ほど光の色をそのまま反射し、金属ほど材質の色が乗る
    const float3 base_specular = lerp(0.04, base_color.rgb, metallic);
    
    /* 平行光 */
    
    {
        // Diffuse(拡散光) 正規化Lambertを使用
        // 光の方向と法線の方向との角度さが光の強さになる
        float light_diffuse = dot(-DirectionalLightDirection, normal);
        light_diffuse = saturate(light_diffuse); // マイナス値は0にする　0(暗)～1(明)になる
        // 正規化Lambert
        light_diffuse /= 3.1415926535;
        // 光の色 * 材質の拡散色 * 透明率
        color += (DirectionalLightColor * light_diffuse) * base_diffuse * base_color.a;

        // Specular(反射色) 正規化Blinn-Phong NDFを使用
        // 反射した光の強さを求める
        // ラフネスから、Blinn-Phong用のSpecularPowerを求める
        float smoothness = 1.0 - roughness; // ラフネスを逆転させ「滑らか」さにする
        float specular_power = pow(2, 13 * smoothness); // 1～8192
        // Blinn-Phong NDF
        float spec = BlinnPhong(DirectionalLightDirection, cam_dir, normal, specular_power);
        // 光の色 * 反射光の強さ * 材質の反射色 * 正規化係数 * 透明率
        color += (DirectionalLightColor * spec) * base_specular * base_color.a;
    }
    
    /* 環境光 */
    color += AmbientLight * base_color.rgb * base_color.a;

    /* エミッシブ */
    color += EmissiveTex.Sample(Sampler, ps_in.UV).rgb * Emissive;

    /* 距離フォグ */
    if (DistanceFogEnable && FogEnable)
    {
        // フォグ 1(近い)～0(遠い)
        float fog = saturate(1.0 / exp(cam_dist * DistanceFogDensity));
        color.rgb = lerp(DistanceFogColor, color.rgb, fog);
    }
    
    /* リムライト */
    // 内積を用いて0.0～1.0を出している (worldnormalを使用)
    float rim = 1.0f - saturate(dot(cam_dir, ps_in.WorldNormal));
    // 最終的なリムの値を出す。
    float3 emission = RimColor.rgb * pow(rim, RimPower) * RimPower;
    color.rgb += emission;
    
    return float4(color, base_color.a);
}
