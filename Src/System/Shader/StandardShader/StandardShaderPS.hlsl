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
/*
float BlinnPhong(float3 lightDir, float3 vCam, float3 normal, float specPower)
{
    float3 H = normalize(-lightDir + vCam);
    // カメラの角度差(0～1)
    float NdotH = saturate(dot(normal, H));
    float spec = pow(NdotH, specPower);

    // 正規化Blinn-Phong
    return spec * ((specPower + 2) / (2 * 3.1415926535));
}
*/
// Lambert拡散反射光を計算
float3 CalcLambertDiffuse(float3 light_dir, float3 light_col, float3 normal)
{
    // ピクセルの法線とライトの方向の内積を計算する
    float t = dot(normal, light_dir) * -1.0f;
    // 内積の値を0以上の値にする
    t = max(0.0f, t);
    // 拡散反射光を計算する
    return light_col * t;
}

// Phong鏡面反射光を計算
float3 CalcPhongSpecular(float3 light_dir, float3 light_col, float3 cam_dir, float3 normal)
{
    // 反射ベクトルを求める
    float3 ref_vec = reflect(light_dir, normal);
    // 鏡面反射の強さを求める
    float t = dot(ref_vec, cam_dir);
    // 鏡面反射の強さを0以上の数値にする
    t = max(0.0f, t);
    // 鏡面反射の強さを絞る
    t = pow(t, 10.0f);
    // 鏡面反射光を求める
    return light_col * t;
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
    float cam_dist = length(cam_dir);           // カメラ - ピクセル距離
    cam_dir = normalize(cam_dir);
    

    /* 材質色 */
    // ベースカラー
    float4 base_color = BaseTex.Sample(Sampler, ps_in.UV) * BaseColor * ps_in.Color;
    // 金属性
    float4 mr = MRTex.Sample(Sampler, ps_in.UV);
    float metallic = mr.b * Metallic;
    // 粗さ
    float roughness = mr.g * Roughness;
    // 法線
    float3 normal = normalize(NormalTex.Sample(Sampler, ps_in.UV).rgb * 2 - 1);
    normal = ps_in.WorldTangent * normal.x + ps_in.WorldBinormal * normal.y + ps_in.WorldNormal * normal.z;

    
    /* ライティング */

    // 最終的な色
    float3 color = 0;
    
    /* 平行光 */
    {
        /*
        // 材質の拡散色　非金属ほど材質の色になり、金属ほど拡散色は無くなる
        const float3 base_diffuse = lerp(base_color.rgb, float3(0, 0, 0), metallic);
        // 材質の反射色　非金属ほど光の色をそのまま反射し、金属ほど材質の色が乗る
        const float3 base_specular = lerp(0.04, base_color.rgb, metallic);
        
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
        */
    }
    {
        // ディレクションライトによるLambert拡散反射光を計算する
        float3 diff_direction = CalcLambertDiffuse(DirectionalLightDirection, DirectionalLightColor, normal);
        // ディレクションライトによるPhong鏡面反射光を計算する
        float3 spec_direction = CalcPhongSpecular(DirectionalLightDirection, DirectionalLightColor, cam_dir, normal);
        /* TODO: PointLight
        // ポイントライトによるLambert拡散反射光とPhong鏡面反射光を計算する
        // サーフェイスに入射するポイントライトの光の向きを計算する
        float3 ligDir = psIn.worldPos - ptPosition;
        // 正規化して大きさ1のベクトルにする
        ligDir = normalize(ligDir);
        // 減衰なしのLambert拡散反射光を計算する
        float3 diffPoint = CalcLambertDiffuse(
            ligDir,     // ライトの方向
            ptColor,    // ライトのカラー
            psIn.normal // サーフェイスの法線
        );
        // 減衰なしのPhong鏡面反射光を計算する
        float3 specPoint = CalcPhongSpecular(
            ligDir,        // ライトの方向
            ptColor,       // ライトのカラー
            psIn.worldPos, // サーフェイスのワールド座標
            psIn.normal    // サーフェイスの法線
        );
        // 距離による影響率を計算する
        float3 distance = length(psIn.worldPos - ptPosition);
        // 影響率は距離に比例して小さくなっていく
        float affect = 1.0f - 1.0f / ptRange * distance;
        // 影響力がマイナスにならないように補正をかける
        if (affect < 0.0f)
        {
            affect = 0.0f;
        }
        // 影響を指数関数的にする。今回のサンプルでは3乗している
        affect = pow(affect, 3.0f);
        // 拡散反射光と鏡面反射光に影響率を乗算して影響を弱める
        diffPoint *= affect;
        specPoint *= affect;
        // 2つの反射光を合算して最終的な反射光を求める
        float3 diffuseLig = diffPoint + diff_direction;
        float3 specularLig = specPoint + spec_direction;
        */
        float3 diffuse_lig = diff_direction;
        float3 specular_lig = spec_direction;

        // 拡散反射光と鏡面反射光、環境光を足し算して、最終的な光を求める
        float3 lig = diffuse_lig + specular_lig + AmbientLight;
        color += BaseTex.Sample(Sampler, ps_in.UV).xyz;

        // テクスチャカラーに求めた光を乗算して最終出力カラーを求める
        color.xyz *= lig;
    }
    
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
