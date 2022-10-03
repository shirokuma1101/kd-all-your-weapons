#pragma once

class Window
{
public:

    ~Window() {
        Release();
    }

    HWND GetWindowHandle() const {
        return m_hWnd;
    }
    HINSTANCE GetInstanceHandle() const {
        return GetModuleHandle(0);
    }

    bool Create(std::string_view title, std::pair<int32_t, int32_t> position, std::pair<int32_t, int32_t> size);

    bool ProcessMessage();

private:

    static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void Release();

    HWND m_hWnd = 0;

};
