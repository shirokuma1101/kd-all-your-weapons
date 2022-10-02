#pragma once

#include <ExternalDependencies/Asset/IAsset/IAssetData.h>

class TextureData : public IAssetData<DirectX11TextureSystem>
{
public:

    TextureData(std::string_view path)
        : IAssetData(path)
    {}
	~TextureData() override {
		Release();
	}

	bool Load() const override {
		return LoadProcess([&] {
			return m_upAssetData->Load(m_filePath);
		});
	}

private:

	void Release() {}

};
