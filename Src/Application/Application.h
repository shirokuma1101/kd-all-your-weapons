#pragma once

#include "Math/DeltaTime.h"
#include "System/Window/Window.h"
#include "Application/Game/GameSystem.h"

class Application
{
public:

    ~Application() {
        Release();
    }

    bool Init();

    void Run();

    Window& GetWindow() noexcept {
        return m_window;
    }
    const Window& GetWindow() const noexcept {
        return m_window;
    }
    std::shared_ptr<GameSystem> GetGameSystem() noexcept {
        return m_spGameSystem;
    }
    std::shared_ptr<const GameSystem> GetGameSystem() const noexcept {
        return m_spGameSystem;
    }
    double GetDeltaTime() const noexcept {
        return m_deltaTime.time;
    }

private:

    void Release();

    Window                      m_window;    // ウィンドウ
    std::shared_ptr<GameSystem> m_spGameSystem;
    DeltaTime                   m_deltaTime; // FPS制御

    MACRO_SINGLETON(Application, WorkInstance, Instance, Destruct);
    
};
