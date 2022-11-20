#pragma once

class PostProcessShader
{
public:

    PostProcessShader() {}
    ~PostProcessShader() {
        Release();
    }

    bool Init();

    void Release();

    void SetToDevice();

private:

    ID3D11PixelShader* m_pPS          = nullptr;
    ID3D11InputLayout* m_pInputLayout = nullptr;

    DirectX11RenderTargetSystem        m_lightBloomRT;

    DirectX11RenderTargetSystem        m_postEffectRT;
    DirectX11RenderTargetChangerSystem m_postEffectRtc;

};
