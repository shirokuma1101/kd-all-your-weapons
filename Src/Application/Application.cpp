#include "Application.h"

bool Application::Init()
{
    const Window::Size     window_size    = Window::HD;
    const Window::Position window_pos     = { 0, 0 };
    const std::string      window_title   = "Game";
    const bool             is_full_screen = false;

    /* ロケールを設定 */
    setlocale(LC_ALL, "japanese");

    /* C++のメモリリークを知らせる */
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    /* COM初期化 */
    if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {
        return false;
    }

    /* ウィンドウ作成 */
    if (!m_window.Create(window_title, window_pos, window_size)) {
        assert::RaiseAssert("ウィンドウ作成に失敗");
        return false;
    }

    /* Direct3D初期化 */
#ifdef ENABLE_DIRECTX11_DEBUG
    if (!DirectX11System::Instance().Init(m_window.GetWindowHandle(), window_size, true)) {
        assert::RaiseAssert("Direct3D初期化失敗");
        return false;
    }
#else
    if (!DirectX11System::Instance().Init(m_window.GetWindowHandle(), window_size, false)) {
        assert::RaiseAssert("Direct3D初期化失敗");
        return false;
    }
#endif
    if (is_full_screen) {
        DirectX11System::Instance().GetSwapChain()->SetFullscreenState(TRUE, 0);
    }

    m_spGameSystem = std::make_shared<GameSystem>();
    m_spGameSystem->Init();

    return true;
}

void Application::Run()
{
    while (true) {

        if (!m_window.ProcessMessage()) break;

        // ゲーム更新
        m_spGameSystem->Update();
        // ゲーム描画
        m_spGameSystem->Draw();
        // ImGui処理
        m_spGameSystem->ImGuiUpdate();

        // BackBuffer -> 画面表示
        DirectX11System::Instance().GetSwapChain()->Present(0, 0);

        // 経過時間を計算
        m_time.CalcDeltaTime();
    }

    Release();
}

// アプリケーション終了
void Application::Release()
{
    m_spGameSystem = nullptr;
    
    /* COM解放 */
    CoUninitialize();
}
