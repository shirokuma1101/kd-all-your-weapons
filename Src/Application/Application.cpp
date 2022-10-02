#include "Application.h"

bool Application::Init()
{
    const std::pair<int32_t, int32_t> window_size = { 1280, 720 };
    
    /* ロケールを設定 */
    setlocale(LC_ALL, "japanese");

    /* C++のメモリリークを知らせる */
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    /* COM初期化 */
    if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {
        return false;
    }

    /* ウィンドウ作成 */
    if (!m_window.Create("title!", { 0, 0 }, window_size)) {
        assert::RaiseAssert("ウィンドウ作成に失敗");
        return false;
    }

    // デバイスのデバッグモードを有効にする
    bool is_debug = false;
#ifdef _DEBUG
    is_debug = true;
#endif
    /* Direct3D初期化 */
    if (!DirectX11System::Instance().Init(m_window.GetWindowHandle(), window_size, is_debug)) {
        assert::RaiseAssert("Direct3D初期化失敗");
        return false;
    }
    
    bool is_full_screen = false;
    //if (MessageBoxA(m_window.GetWindowHandle(), "フルスクリーンにしますか？", "確認", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) {
    //	is_full_screen = true;
    //}
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
