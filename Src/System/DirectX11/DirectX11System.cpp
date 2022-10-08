#include "DirectX11System.h"

#include "System/Shader/ShaderManager.h"

bool DirectX11System::Init(HWND hWnd, const std::pair<int32_t, int32_t>& size, bool is_debug, bool detailed_memory_infomation, bool enable_msaa)
{
    if (!DirectX11::Init(hWnd, size, is_debug, detailed_memory_infomation, enable_msaa)) {
        return false;
    }
    m_spShaderManager = std::make_shared<ShaderManager>();
    m_spShaderManager->Init();
    for (auto&& e : m_spTempFixedVertexBuffers) {
        e = std::make_shared<DirectX11BufferSystem>();
    }
    m_spTempVertexBuffer = std::make_shared<DirectX11BufferSystem>();
    return true;
}

void DirectX11System::DrawVertices(D3D_PRIMITIVE_TOPOLOGY topology, UINT vertex_size, UINT stride, const void* vertex_stream)
{
    // プリミティブトポロジーをセット
    m_cpCtx->IASetPrimitiveTopology(topology);

    // 全頂点の総バイトサイズ
    UINT total_size = vertex_size * stride;

    // 最適な固定長バッファを検索
    std::shared_ptr<DirectX11BufferSystem> buffer = nullptr;
    for (auto&& n : m_spTempFixedVertexBuffers) {
        if (total_size < n->GetSize()) {
            buffer = n;
            break;
        }
    }
    // 可変長のバッファを使用
    if (buffer == nullptr) {
        buffer = m_spTempVertexBuffer;
        // 頂点バッファのサイズが小さいときは、リサイズのため再作成する
        if (m_spTempVertexBuffer->GetSize() < total_size) {
            m_spTempVertexBuffer->Create(total_size, D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, nullptr);
        }
    }

    //============================================================
    //
    // 単純なDISCARDでの書き込み。
    //  DISCARDは、新たなバッファを内部で作成し、前回のバッファは使い終わると無効にするものです。
    //  つまり書き込む度に新規のバッファになる感じです。
    //  バッファのサイズが大きいと、その分新規のバッファを作成する時間がかかってしまいます。
    //  これを改善したい場合は、「DISCARDとNO_OVERWRITEの組み合わせ技」の方法で行うほうがよいです。
    //
    //============================================================

    // 全頂点をバッファに書き込み(DISCARD指定)
    buffer->Write(vertex_stream, total_size);

    // 頂点バッファーをデバイスへセット
    {
        UINT offset = 0;
        m_cpCtx->IASetVertexBuffers(0, 1, buffer->GetAddress(), &stride, &offset);
    }

    // 描画
    m_cpCtx->Draw(vertex_size, 0);
}

void DirectX11System::Release()
{
    for (auto&& e : m_spTempFixedVertexBuffers) {
        e.reset();
    }
    m_spTempVertexBuffer.reset();
    
    m_spShaderManager.reset();
}
