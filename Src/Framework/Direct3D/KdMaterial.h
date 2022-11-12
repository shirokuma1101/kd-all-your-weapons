#pragma once

//==========================================================
// マテリアル
//==========================================================
struct KdMaterial
{
    KdMaterial()
        : BaseColorTex(DirectX11System::WorkInstance().GetWhiteTexture())
        , MetallicRoughnessTex(DirectX11System::WorkInstance().GetWhiteTexture())
        , EmissiveTex(DirectX11System::WorkInstance().GetWhiteTexture())
        , NormalTex(DirectX11System::WorkInstance().GetNormalTexture())
    {}
    
    void Load(std::string_view filePath);

    void SetTextures(const std::string& fileDir, std::string_view baseTex, std::string_view mrTex, std::string_view emiTex, std::string_view nmlTex);

    //---------------------------------------
    // 材質データ
    //---------------------------------------

    // 名前
    std::string Name;

    // 基本色
    std::shared_ptr<DirectX11Texture> BaseColorTex; // 基本色テクスチャ
    Math::Vector4 BaseColor = { 1,1,1,1 }; // 基本色のスケーリング係数(RGBA)

    // 金属性、粗さ
    std::shared_ptr<DirectX11Texture>	MetallicRoughnessTex; // B:金属製 G:粗さ
    float Metallic = 0.0f; // 金属性のスケーリング係数
    float Roughness = 1.0f; // 粗さのスケーリング係数

    // 自己発光
    std::shared_ptr<DirectX11Texture> EmissiveTex; // 自己発光テクスチャ
    Math::Vector3 Emissive = { 0,0,0 }; // 自己発光のスケーリング係数(RGB)

    // 法線マップ
    std::shared_ptr<DirectX11Texture> NormalTex;

};
