#pragma once

#include "ExternalDependencies/Asset/Json/JsonManager.h"

#include "System/Asset/Model/ModelManager.h"
#include "System/Asset/Texture/TextureManager.h"

class AssetManager
{
public:

	enum class AssetType {
		Json,
		Texture,
		Model,
	};

	AssetManager()
		: m_upJsonMgr(std::make_unique<JsonManager>())
		, m_upTextureMgr(std::make_unique<TextureManager>())
		, m_upModelMgr(std::make_unique<ModelManager>())
	{}

	void Register(AssetType type, std::string_view path) {
		switch (type) {
		case AssetType::Json:
			m_upJsonMgr->Register(path);
			break;
		case AssetType::Texture:
			m_upTextureMgr->Register(path);
			break;
		case AssetType::Model:
			m_upModelMgr->Register(path);
			break;
		}
	}

	void Register(AssetType type, const std::unordered_map<std::string, std::unique_ptr<JsonData>>& jsons, std::initializer_list<std::string_view> keys = {}) {
		switch (type) {
		case AssetType::Json:
			m_upJsonMgr->Register(jsons, keys);
			break;
		case AssetType::Texture:
			m_upTextureMgr->Register(jsons, keys);
			break;
		case AssetType::Model:
			m_upModelMgr->Register(jsons, keys);
			break;
		}
	}

	void Load(AssetType type) {
		switch (type) {
		case AssetType::Json:
			m_upJsonMgr->Load();
			break;
		case AssetType::Texture:
			m_upTextureMgr->Load();
			break;
		case AssetType::Model:
			m_upModelMgr->Load();
			break;
		}
	}
	
	void Load(AssetType type, std::string_view name) {
		switch (type) {
		case AssetType::Json:
			m_upJsonMgr->Load(name);
			break;
		case AssetType::Texture:
			m_upTextureMgr->Load(name);
			break;
		case AssetType::Model:
			m_upModelMgr->Load(name);
			break;
		}
	}

	void AsyncLoad(AssetType type, std::string_view name, bool force = false) {
		switch (type) {
		case AssetType::Json:
			m_upJsonMgr->AsyncLoad(name, force);
			break;
		case AssetType::Texture:
			m_upTextureMgr->AsyncLoad(name, force);
			break;
		case AssetType::Model:
			m_upModelMgr->AsyncLoad(name, force);
			break;
		}
	}

	std::string GetFilePath(AssetType type, std::string_view name) {
		switch (type) {
		case AssetType::Json:
			return m_upJsonMgr->GetFilePath(name);
			break;
		case AssetType::Texture:
			return m_upTextureMgr->GetFilePath(name);
			break;
		case AssetType::Model:
			return m_upModelMgr->GetFilePath(name);
			break;
		}
	}

	const std::unique_ptr<JsonManager>& GetJsonMgr() const {
		return m_upJsonMgr;
	}

	const std::unique_ptr<TextureManager>& GetTextureMgr() const {
		return m_upTextureMgr;;
	}

	const std::unique_ptr<ModelManager>& GetModelMgr() const {
		return m_upModelMgr;
	}

private:

	std::unique_ptr<JsonManager>    m_upJsonMgr;
	std::unique_ptr<TextureManager> m_upTextureMgr;
	std::unique_ptr<ModelManager>   m_upModelMgr;

};
