#pragma once

#include <ExternalDependencies/Asset/IAsset/IAssetManager.h>
#include "TextureData.h"

class TextureManager : public IAssetManager<TextureData>
{
public:

    TextureManager() {}
    ~TextureManager() {
        Release();
    }

private:

};
