#include "Window.h"

LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool Window::Create(std::string_view title, const Position& position, const Size& size)
{
    m_size                  = size;
    HINSTANCE hInstance     = GetInstanceHandle();
    std::wstring class_name = sjis_to_wide(title.data());
    std::wstring title_name = sjis_to_wide(title.data());

    /* ウィンドウを作るための設定 */
    WNDCLASSEX wc{};                                       // ウィンドウクラスの定義用
    wc.cbSize        = sizeof(WNDCLASSEX);                 // 構造体のサイズ
    wc.style         = 0;                                  // スタイル
    wc.lpfnWndProc   = Window::StaticWindowProc;           // ウィンドウプロシージャ名(アドレス)
    wc.cbClsExtra    = 0;                                  // ウィンドウクラスの保持するデータサイズ
    wc.cbWndExtra    = 0;                                  // ウィンドウの保持するデータサイズ
    wc.hInstance     = hInstance;                          // インスタンスハンドル
    wc.hIcon         = LoadIcon(nullptr, IDI_APPLICATION); // ラージアイコン
    wc.hIconSm       = LoadIcon(nullptr, IDI_APPLICATION); // スモールアイコン (win 4.0)
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);     // マウスカーソル
    wc.hbrBackground = NULL;                               // 背景色
    wc.lpszMenuName  = nullptr;                            // メインメニュー名
    wc.lpszClassName = class_name.c_str();                 // ウィンドウクラス名

    /* ウィンドウクラスの登録 */
    if (!RegisterClassEx(&wc)) {
        return false;
    }

    /* ウィンドウクラスに従いウィンドウを(メモリ上に)作る */
    m_hWnd = CreateWindow(
        class_name.c_str(),                  // ウィンドウクラス名
        title_name.c_str(),                  // ウィンドウのタイトル
        WS_OVERLAPPEDWINDOW - WS_THICKFRAME, // ウィンドウタイプを標準タイプに
        position.first,                      // ウィンドウの位置（X座標）
        position.second,                     // ウィンドウの位置（Y座標）
        m_size.first,                        // ウィンドウの幅
        m_size.second,                       // ウィンドウの高さ
        nullptr,                             // 親ウィンドウのハンドル
        nullptr,                             // メニューのハンドル
        hInstance,                           // インスタンスハンドル
        this                                 // 追加情報
    );

    /* クライアントサイズを設定 */
    Resize(size);

    /* ウィンドウの表示 */
    ShowWindow(m_hWnd, SW_SHOW);
    /* ウィンドウの更新 */
    UpdateWindow(m_hWnd);

    return true;
}

bool Window::ProcessMessage()
{
    /* メッセージを取得 */
    MSG msg{};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        /* メッセージが終了の場合 */
        if (msg.message == WM_QUIT) {
            return false;
        }
        /* メッセージ処理 */
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

void Window::Resize(const Size& size)
{
    m_size = size;
    WINDOWINFO wi{};
    SecureZeroMemory(&wi, sizeof(wi));
    GetWindowInfo(m_hWnd, &wi);
    LONG new_width  = (wi.rcWindow.right  - wi.rcWindow.left) - (wi.rcClient.right  - wi.rcClient.left) + m_size.first;
    LONG new_height = (wi.rcWindow.bottom - wi.rcWindow.top)  - (wi.rcClient.bottom - wi.rcClient.top)  + m_size.second;
    SetWindowPos(m_hWnd, NULL, 0, 0, new_width, new_height, SWP_NOMOVE | SWP_NOZORDER);
}

void Window::SetWindowStyle(LONG_PTR new_long)
{
    SetWindowLongPtr(m_hWnd, GWL_STYLE, new_long);
    ShowWindow(m_hWnd, SW_SHOW);
}

LRESULT CALLBACK Window::StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // ウィンドウプロパティから、GameWindowクラスのインスタンスを取得
    Window* this_window = (Window*)GetProp(hWnd, L"WindowInstance");

    // nullptrの場合は、デフォルト処理を実行
    if (!this_window) {
        switch (uMsg) {
        case WM_CREATE:
            // SetPropは1回しか行われないのでcase内での初期化を許容するために括弧を付ける
            {
                // CreateWindow()で渡したパラメータを取得
                CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
                Window* window = (Window*)cs->lpCreateParams;
                // ウィンドウプロパティにこのクラスのインスタンスアドレスを埋め込んでおく
                // 次回から、this_window->WindowProcの方へ処理が流れていく
                SetProp(hWnd, L"WindowInstance", window);
            }
            return 0;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }

    // インスタンス側のWindow関数を実行する
    return this_window->WindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK Window::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
        return true;
    }
    
    switch (uMsg) {
    case WM_DESTROY:
        RemoveProp(hWnd, L"WindowInstance");
        PostQuitMessage(0);
        break;
    case WM_CLOSE:
        Release();
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

void Window::Release()
{
    if (m_hWnd) {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
}
