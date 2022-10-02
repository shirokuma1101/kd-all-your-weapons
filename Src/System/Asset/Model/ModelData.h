#pragma once

#include <ExternalDependencies/Asset/IAsset/IAssetData.h>

class ModelData : public IAssetData<KdModelData>
{
public:

	ModelData(std::string_view path)
		: IAssetData(path)
	{}
	~ModelData() override {
		Release();
	}

	bool Load() const override {
		const bool successed = LoadProcess([&] {
			return m_upAssetData->Load(m_filePath);
		});
		if (successed) {
			m_spModel = std::make_shared<KdModelWork>();
			m_spModel->SetModel(std::make_shared<KdModelData>(*m_upAssetData));
		}
		return successed;
	}

	const std::shared_ptr<KdModelWork> GetOriginalData() const {
		return m_spModel;
	}

private:
	
	void Release() {}

	mutable std::shared_ptr<KdModelWork> m_spModel = nullptr;

};
