#include "../Common.hlsli"
#include "StandardShader.hlsli"

// テクスチャ
Texture2D BaseTex     : register(t0); // ベースカラーテクスチャ
Texture2D EmissiveTex : register(t1); // エミッシブテクスチャ
Texture2D MRTex       : register(t2); // メタリック/ラフネステクスチャ
Texture2D NormalTex   : register(t3); // 法線マップテクスチャ

// 特殊処理用テクスチャ
Texture2D ShadowTex   : register(t10); // シャドウテクスチャ
Texture2D DissolveTex : register(t11); // ディゾルブテクスチャ

// サンプラーステート
SamplerState           Sampler     : register(s0);
SamplerComparisonState SamplerComp : register(s1);

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
    //アルファテスト
    if (base_color.a < 0.1f)
    {
        discard;
    }
    // 金属性
    float4 mr = MRTex.Sample(Sampler, ps_in.UV);
    float metallic = mr.b * Metallic;
    // 粗さ
    float roughness = mr.g * Roughness;
    // 法線
    float3 normal = normalize(NormalTex.Sample(Sampler, ps_in.UV).rgb * 2 - 1);
    normal = ps_in.WorldTangent * normal.x + ps_in.WorldBinormal * normal.y + ps_in.WorldNormal * normal.z;


    /* シャドウ */
    // 最終的な影
    float shadow = 1.0f;
    {
        // ピクセルの3D座標から、DepthMapFromLight空間へ変換
        float4 light_pos = mul(float4(ps_in.WorldPos, 1), DirectionalLightVP);
        light_pos.xyz /= light_pos.w;

        // 深度マップの範囲内？
        if (abs(light_pos.x) <= 1 && abs(light_pos.y) <= 1 && light_pos.z <= 1)
        {
            // 射影座標 -> UV座標へ変換
            float2 uv = light_pos.xy * float2(1, -1) * 0.5 + 0.5;
            // ライトカメラからの距離
            float z = light_pos.z - 0.002; // シャドウアクネ対策
        
            // 画像のサイズからテクセルサイズを求める
            float w, h;
            ShadowTex.GetDimensions(w, h);
            float tw = 1.0 / w;
            float th = 1.0 / h;
    
            // uvの周辺3x3も判定し、平均値を求める
            shadow = 0;
            for (int y = -1; y <= 1; y++)
            {
                for (int x = -1; x <= 1; x++)
                {
                    shadow += ShadowTex.SampleCmpLevelZero(SamplerComp, uv + float2(x * tw, y * th), z);
                }
            }
            shadow *= 0.11;
        }
    }

    /* ライティング */
    // 最終的な色
    float3 color = 0;
    {
        /* 平行光 */
        // ディレクションライトによるLambert拡散反射光を計算する
        float3 diff_direction = CalcLambertDiffuse(DirectionalLightDirection, DirectionalLightColor, normal);
        // ディレクションライトによるPhong鏡面反射光を計算する
        float3 spec_direction = CalcPhongSpecular(DirectionalLightDirection, DirectionalLightColor, cam_dir, normal);

        /* ポイントライト */
        float3 diffuse_light = 0;
        float3 specular_light = 0;
        for (int i = 0; i < PointLightCount; i++)
        {
            // サーフェイスに入射するポイントライトの光の向きを計算する
            float3 light_dir = ps_in.WorldPos - PointLights[i].Position;
            // ポイントライトからの距離
            float light_dist = length(light_dir);
            // ポイントライトへの方向を正規化
            light_dir = normalize(light_dir);
            // ポイントライトによる減衰なしのLambert拡散反射光を計算する
            float3 diff_point = CalcLambertDiffuse(light_dir, PointLights[i].Color, normal);
            // ポイントライトによる減衰なしのPhong鏡面反射光を計算する
            float3 spec_point = CalcPhongSpecular(light_dir, PointLights[i].Color, cam_dir, normal);
            // ポイントライトによる減衰を計算する
            float att = 1.0f / (PointLights[i].Attenuation.x + PointLights[i].Attenuation.y * light_dist + PointLights[i].Attenuation.z * light_dist * light_dist);
            // ポイントライトによる減衰を適用する
            diff_point *= att;
            spec_point *= att;
            // ポイントライトによるLambert拡散反射光を加算する
            diffuse_light += diff_point;
            // ポイントライトによるPhong鏡面反射光を加算する
            specular_light += spec_point;
        }
        // ディレクションライトを加算する
        diffuse_light += diff_direction * shadow;
        specular_light += spec_direction * shadow;
        // 拡散反射光と鏡面反射光、環境光を足し算して、最終的な光を求める
        float3 light = diffuse_light + specular_light + AmbientLight;
        color += BaseTex.Sample(Sampler, ps_in.UV).xyz;
        // テクスチャカラーに求めた光を乗算して最終出力カラーを求める
        color.xyz *= light;
    }

    /* エミッシブ */
    color += EmissiveTex.Sample(Sampler, ps_in.UV).rgb * Emissive;

    /* 距離フォグ */
    if (DistanceFogEnable && FogEnable)
    {
        // フォグ 1(近い)～0(遠い)
        float fog = saturate((DistanceFogEnd - cam_dist) / (DistanceFogEnd - DistanceFogStart));
        //float fog = exp(-0.06 * cam_dist);
        //float fog = exp(-mul(0.03 * cam_dist, 2));
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
