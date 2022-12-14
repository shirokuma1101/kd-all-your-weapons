#include "Application.h"

#ifdef _DEBUG
//#define ENABLE_BREAKPOINT_ALLOCATE_MEMORY 1;
#define ENABLE_DIRECTX11_DEBUG
//#define ENABLE_DIRECTX11_DETAILED_MEMORY_INFOMATION
#define ENABLE_IMGUI
#endif

bool Application::Init()
{
    const Window::Size     window_size  = Window::HD;
    const Window::Position window_pos   = { 0, 0 };
    const std::string      window_title = "AllYourWeapons";

    /* ロケールを設定 */
    setlocale(LC_ALL, "japanese");

    /* C++のメモリリークを知らせる */
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#ifdef ENABLE_BREAKPOINT_ALLOCATE_MEMORY
    constexpr int break_alloc = ENABLE_BREAKPOINT_ALLOCATE_MEMORY;
    _CrtSetBreakAlloc(break_alloc);
#endif
    
    /* COM初期化 */
    if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {
        return false;
    }

    /* ウィンドウ作成 */
    if (!m_window.Create(window_title, window_pos, window_size)) {
        assert::ShowError(ASSERT_FILE_LINE, "ウィンドウ作成に失敗");
        return false;
    }

    /* Direct3D初期化 */
#ifdef ENABLE_DIRECTX11_DEBUG
    constexpr bool enable_debug = true;
#else
    constexpr bool enable_debug = false;
#endif
#ifdef ENABLE_DIRECTX11_DETAILED_MEMORY_INFOMATION
    constexpr bool enable_detailed_memory_infomation = true;
#else
    constexpr bool enable_detailed_memory_infomation = false;
#endif
    if (!DirectX11System::WorkInstance().Init(m_window.GetWindowHandle(), window_size, enable_debug, enable_detailed_memory_infomation)) {
        assert::ShowError(ASSERT_FILE_LINE, "Direct3D初期化失敗");
        return false;
    }
    
    m_spGameSystem = std::make_shared<GameSystem>();
    m_spGameSystem->Init();

    return true;
}

void Application::Run()
{
    while (true) {
        // ウィンドウメッセージ処理
        if (!m_window.ProcessMessage()) break;

        // ゲーム更新
        m_spGameSystem->Update();
        // ゲーム描画
        m_spGameSystem->Draw();
            
#ifdef ENABLE_IMGUI
        // ImGui処理
        m_spGameSystem->ImGuiUpdate();
#endif

        // BackBuffer -> 画面表示
        DirectX11System::Instance().GetSwapChain()->Present(0, 0);

        // 経過時間を計算
        m_deltaTime.Calc();
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
