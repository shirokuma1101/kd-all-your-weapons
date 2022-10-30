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

    void SetWindowSettings(int resolution, float rendering_resolution_percentage = 100.f, bool is_full_screen = false) {
        Window::Size size;
        switch (resolution) {
        case 0:
            size = Window::HD;
            break;
        case 1:
            size = Window::FHD;
            break;
        case 2:
            size = Window::QHD;
            break;
        case 3:
            size = Window::UHD;
            break;
        }
        m_window.Resize(size);
        DirectX11System::WorkInstance().Resize({
            static_cast<Window::Size::first_type>(size.first * (rendering_resolution_percentage / 100.f)),
            static_cast<Window::Size::second_type>(size.second * (rendering_resolution_percentage / 100.f))
            });
        if (is_full_screen) {
            DirectX11System::Instance().GetSwapChain()->SetFullscreenState(TRUE, 0);
        }
        else {
            DirectX11System::Instance().GetSwapChain()->SetFullscreenState(FALSE, 0);
        }
    }

private:

    void Release();

    Window                      m_window; // ウィンドウ
    std::shared_ptr<GameSystem> m_spGameSystem;
    DeltaTime                   m_deltaTime;   // FPS制御

    MACRO_SINGLETON(Application, WorkInstance, Instance, Destruct);
    
};
