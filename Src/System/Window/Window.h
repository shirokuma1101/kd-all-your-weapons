#pragma once

class Window
{
public:

    using Position = std::pair<int32_t, int32_t>;
    using Size     = std::pair<int32_t, int32_t>;

    static constexpr Size HD     = { 1280, 720 };
    static constexpr Size FHD    = { 1920, 1080 };
    static constexpr Size QHD    = { 2560, 1440 };
    static constexpr Size UHD    = { 3840, 2160 };
    static constexpr Size UWQHD  = { 3440, 1440 };
    static constexpr Size UWQHDP = { 3840, 1600 };
    
    ~Window() {
        Release();
    }

    const Size& GetSize() const noexcept {
        return m_size;
    }
    HWND GetWindowHandle() const {
        return m_hWnd;
    }
    HINSTANCE GetInstanceHandle() const {
        return GetModuleHandle(0);
    }

    bool Create(std::string_view title, const Position& position, const Size& size);

    bool ProcessMessage();

    void Resize(const Size& size);

    static float ToAspectRatio(const Size& size) noexcept {
        return static_cast<float>(size.first) / static_cast<float>(size.second);
    }

private:

    static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void Release();

    Size m_size;
    HWND m_hWnd = 0;

};
