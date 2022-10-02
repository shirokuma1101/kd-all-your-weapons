#pragma once

#include "ExternalDependencies/DirectX11/DirectX11Texture.h"

class DirectX11TextureSystem : public DirectX11Texture
{
public:

    DirectX11TextureSystem()
        : DirectX11Texture(DirectX11System::Instance().GetDev().Get(), DirectX11System::Instance().GetCtx().Get())
    {}

};
