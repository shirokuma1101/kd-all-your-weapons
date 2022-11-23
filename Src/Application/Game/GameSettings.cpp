#include "GameSettings.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/optional.hpp>

void GameSettings::Load()
{
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(m_settingsPath, pt);

    displayMode    = static_cast<DisplayMode>(pt.get<UtDisplayMode>("SETTINGS.DisplayMode"));
    resolutionType = static_cast<ResolutionType>(pt.get<UtResolutionType>("SETTINGS.ResolutionType"));
    frameRateLimit = static_cast<FrameRateLimit>(pt.get<UtFrameRateLimit>("SETTINGS.FrameRateLimit"));
    renderScaling  = pt.get<float>("SETTINGS.RenderScaling");
    anisotropicFiltering = static_cast<AnisotropicFiltering>(pt.get<UtAnisotropicFiltering>("SETTINGS.AnisotropicFiltering"));
    shadowMaps     = static_cast<ShadowMaps>(pt.get<UtShadowMaps>("SETTINGS.ShadowMaps"));
    masterVolume   = pt.get<float>("SETTINGS.MasterVolume");
}

void GameSettings::Save()
{
    boost::property_tree::ptree pt;
    
    pt.put<UtDisplayMode>("SETTINGS.DisplayMode", static_cast<UtDisplayMode>(displayMode));
    pt.put<UtResolutionType>("SETTINGS.ResolutionType", static_cast<UtResolutionType>(resolutionType));
    pt.put<UtFrameRateLimit>("SETTINGS.FrameRateLimit", static_cast<UtFrameRateLimit>(frameRateLimit));
    pt.put<float>("SETTINGS.RenderScaling", renderScaling);
    pt.put<UtAnisotropicFiltering>("SETTINGS.AnisotropicFiltering", static_cast<UtAnisotropicFiltering>(anisotropicFiltering));
    pt.put<UtShadowMaps>("SETTINGS.ShadowMaps", static_cast<UtShadowMaps>(shadowMaps));
    pt.put<float>("SETTINGS.MasterVolume", masterVolume);

    boost::property_tree::ini_parser::write_ini(m_settingsPath, pt);
}
