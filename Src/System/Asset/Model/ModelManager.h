#pragma once

#include <ExternalDependencies/Asset/IAsset/IAssetManager.h>
#include "ModelData.h"

class ModelManager : public IAssetManager<ModelData>
{
public:

	ModelManager() {}
	~ModelManager() {
        Release();
    }

	std::shared_ptr<KdModelWork> CopyData(std::string_view name) {
		auto data = std::make_shared<KdModelWork>();
		data->SetModel(std::make_shared<KdModelData>(*GetData(name).get()));
		return data;
	}

	const std::shared_ptr<KdModelWork> GetOriginalData(std::string_view name) const {
		return GetAsset(name)->GetOriginalData();
	}

private:

};
