#pragma once

#include "Utility/Macro.h"

#include "ExternalDependencies/DirectX11/DirectX11.h"

class ShaderManager;
class DirectX11BufferSystem;

class DirectX11System : public DirectX11
{
public:

    virtual ~DirectX11System() override {
        Release();
    }

    bool Init(HWND hWnd, const std::pair<int32_t, int32_t>& size, bool is_debug, bool enable_msaa = false) override;

    auto GetShaderManager() {
        return m_spShaderManager;
    }
    auto GetShaderManager() const {
        return m_spShaderManager;
    }

    void DrawVertices(D3D_PRIMITIVE_TOPOLOGY topology, UINT vertex_size, UINT stride, const void* vertex_stream);
    
private:

    void Release();

    std::shared_ptr<ShaderManager>                         m_spShaderManager;

    std::array<std::shared_ptr<DirectX11BufferSystem>, 10> m_spTempFixedVertexBuffers;
    std::shared_ptr<DirectX11BufferSystem>                 m_spTempVertexBuffer;

    SIMPLE_SINGLETON(DirectX11System, Instance);

};
