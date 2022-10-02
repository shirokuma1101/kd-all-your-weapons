#pragma once

#include "ExternalDependencies/DirectX11/DirectX11Buffer.h"
#include "System/DirectX11/DirectX11System.h"

class DirectX11BufferSystem : public DirectX11Buffer
{
public:

    DirectX11BufferSystem()
        : DirectX11Buffer(DirectX11System::Instance().GetDev().Get(), DirectX11System::Instance().GetCtx().Get())
    {}
    
};

template<class T>
class DirectX11ConstantBufferSystem : public DirectX11ConstantBuffer<T>
{
public:

    DirectX11ConstantBufferSystem()
        : DirectX11ConstantBuffer<T>(DirectX11System::Instance().GetDev().Get(), DirectX11System::Instance().GetCtx().Get())
    {}
    
};
