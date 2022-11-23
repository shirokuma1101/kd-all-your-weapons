#pragma once

/**************************************************
* 
* Game settings
* ref:"https://www.flightsimulator.blog/2022/10/21/best-graphics-settings-guide/"
* 
**************************************************/
struct GameSettings {
    /* Graphics */
    enum class DisplayMode {
        WINDOWED,
        BORDERLESS,
        FULLSCREEN,
    };
    enum class ResolutionType {
        HD,    // { 1280,  720 }
        FHD,   // { 1920, 1080 }
        QHD,   // { 2560, 1440 }
        UHD,   // { 3840, 2160 }
        UWQHD, // { 3440, 1440 }
        UWQHDP,// { 3840, 1600 }
    };
    enum class FrameRateLimit {
        NO,
        _30 = 30,
        _60 = 60,
        _90 = 90,
        _120 = 120,
        _144 = 144,
        _165 = 165,
        _180 = 180,
        _240 = 240,
        _360 = 360,
    };
    enum class AnisotropicFiltering {
        ANISOTROPICx1,
        ANISOTROPICx2,
        ANISOTROPICx4,
        ANISOTROPICx8,
        ANISOTROPICx16,
    };
    enum class ShadowMaps {
        NO,
        x512,
        x1024,
        x2048,
        x4096,
        x8192,
    };
    using UtDisplayMode    = std::underlying_type_t<DisplayMode>;
    using UtResolutionType = std::underlying_type_t<ResolutionType>;
    using UtFrameRateLimit = std::underlying_type_t<FrameRateLimit>;
    using UtAnisotropicFiltering = std::underlying_type_t<AnisotropicFiltering>;
    using UtShadowMaps     = std::underlying_type_t<ShadowMaps>;

    GameSettings(std::string_view file_path)
        : m_settingsPath(file_path)
    {
        std::error_code ec;
        if (!std::filesystem::exists(m_settingsPath, ec)) {
            Save();
        }
    }

    void Load();
    void Save();

    /* Convert */
    
    static Window::Size ToResolution(ResolutionType type) noexcept {
        switch (type) {
        case ResolutionType::HD:
            return { 1280,  720 };
        case ResolutionType::FHD:
            return { 1920, 1080 };
        case ResolutionType::QHD:
            return { 2560, 1440 };
        case ResolutionType::UHD:
            return { 3840, 2160 };
        case ResolutionType::UWQHD:
            return { 3440, 1440 };
        case ResolutionType::UWQHDP:
            return { 3840, 1600 };
        default:
            return { 0, 0 };
        }
    }
    static std::pair<int32_t, int32_t> ToShadowMaps(ShadowMaps type) noexcept {
        switch (type) {
        case ShadowMaps::x512:
            return {  512,  512 };
        case ShadowMaps::x1024:
            return { 1024, 1024 };
        case ShadowMaps::x2048:
            return { 2048, 2048 };
        case ShadowMaps::x4096:
            return { 4096, 4096 };
        case ShadowMaps::x8192:
            return { 8192, 8192 };
        default:
            return { 0, 0 };
        }
    }

    /* FilePath */
    const std::string    m_settingsPath;

    /* Graphics */
    DisplayMode          displayMode    = DisplayMode::WINDOWED;
    ResolutionType       resolutionType = ResolutionType::HD;
    FrameRateLimit       frameRateLimit = FrameRateLimit::_60;
    float                renderScaling  = 100.f;
    AnisotropicFiltering anisotropicFiltering = AnisotropicFiltering::ANISOTROPICx1;
    ShadowMaps           shadowMaps     = ShadowMaps::x512;
    /* Sound */
    float                masterVolume   = 0.f;

};
