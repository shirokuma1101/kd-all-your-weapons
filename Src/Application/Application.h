#pragma once

#include "Math/Time.h"
#include "System/Window/Window.h"
#include "Application/Game/GameSystem.h"

class Application
{
public:

    bool Init();

    void Run();
    
    const Window& GetWindow() const noexcept {
        return m_window;
    }
    double GetDeltaTime() const noexcept {
        return m_time.deltaTime;
    }
    std::shared_ptr<GameSystem> GetGameSystem() {
        return m_spGameSystem;
    }

private:

    Time                        m_time;   // FPS制御
    Window                      m_window; // ウィンドウ
    std::shared_ptr<GameSystem> m_spGameSystem;

    void Release();

    SIMPLE_SINGLETON(Application, Instance);
    
};
