#include "KdMaterial.h"

void KdMaterial::Load(std::string_view filePath)
{
    auto fs = std::filesystem::path(filePath);
	std::string fileDir = fs.parent_path().string();
	std::string fileName = fs.filename().string();

    SetTextures(fileDir, fileName + ".png", fileName + "_rm.png", fileName + "_ems.png", fileName + "_nml.png");
}

void KdMaterial::SetTextures(const std::string& fileDir, std::string_view baseTexName, std::string_view mrTexName, std::string_view emiTexName, std::string_view nmlTexName)
{
	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 基本色
	if (!baseTexName.empty() && std::filesystem::exists(fileDir + "/" + baseTexName.data())) {
		BaseColorTex = std::make_shared<DirectX11TextureSystem>();
		BaseColorTex->Load(fileDir + "/" + baseTexName.data());
	}

	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	// 金属性・粗さ
	if (!mrTexName.empty() && std::filesystem::exists(fileDir + "/" + mrTexName.data())) {
		MetallicRoughnessTex = std::make_shared<DirectX11TextureSystem>();
		MetallicRoughnessTex->Load(fileDir + "/" + mrTexName.data());

		Metallic = 1.0f;
	}

	// 自己発光・エミッシブ
	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	if (!emiTexName.empty() && std::filesystem::exists(fileDir + "/" + emiTexName.data())) {
		EmissiveTex = std::make_shared<DirectX11TextureSystem>();
		EmissiveTex->Load(fileDir + "/" + emiTexName.data());

		Emissive = { 1.0f, 1.0f, 1.0f };
	}

	// 法線
	// ===== ===== ===== ===== ===== ===== ===== ===== ===== ===== =====
	if (!nmlTexName.empty() && std::filesystem::exists(fileDir + "/" + nmlTexName.data())) {
		NormalTex = std::make_shared<DirectX11TextureSystem>();
		NormalTex->Load(fileDir + "/" + nmlTexName.data());
	}
}
