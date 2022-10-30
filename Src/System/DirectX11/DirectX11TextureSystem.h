#pragma once

#include "ExternalDependencies/DirectX11/DirectX11Texture.h"

class DirectX11TextureSystem : public DirectX11Texture
{
public:

    DirectX11TextureSystem()
        : DirectX11Texture(DirectX11System::Instance().GetDev().Get(), DirectX11System::Instance().GetCtx().Get())
    {}

};

class DirectX11RenderTargetSystem : public DirectX11RenderTarget
{
public:

    DirectX11RenderTargetSystem()
        : DirectX11RenderTarget(DirectX11System::Instance().GetDev().Get(), DirectX11System::Instance().GetCtx().Get())
    {}

};

class DirectX11RenderTargetChangerSystem : public DirectX11RenderTargetChanger
{
public:

    DirectX11RenderTargetChangerSystem()
        : DirectX11RenderTargetChanger(DirectX11System::Instance().GetDev().Get(), DirectX11System::Instance().GetCtx().Get())
    {}

};
