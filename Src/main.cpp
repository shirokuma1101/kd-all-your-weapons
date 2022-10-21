#include "Application/Application.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    hInstance; hPrevInstance; lpCmdLine; nShowCmd; // C4100 'identifier' : unreferenced formal parameter
    
    // インスタンスを取得
    auto& app = Application::WorkInstance();

    // 初期化
    if (!app.Init()) {
        return -1;
    }

    // 実行
    app.Run();

    return 0;
}
