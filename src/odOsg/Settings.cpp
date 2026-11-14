/*
 * Settings.cpp
 */

#include <odOsg/Settings.h>

#include <fstream>

#include <odCore/ConfigFile.h>
#include <odCore/Logger.h>

namespace odOsg
{

    bool Settings::loadFromFile(const od::FilePath &path)
    {
        std::ifstream in(path.str());
        if(!in.is_open())
        {
            return false;
        }

        od::ConfigFile cfg;
        cfg.parse(in);

        windowWidth  = cfg.get<int>("video", "width", windowWidth);
        windowHeight = cfg.get<int>("video", "height", windowHeight);
        fullscreen   = cfg.get<int>("video", "fullscreen", fullscreen ? 1 : 0) != 0;
        screen       = cfg.get<int>("video", "screen", screen);
        horizontalFov = cfg.get<double>("video", "fov", horizontalFov);

        grabMouse    = cfg.get<int>("input", "grab_mouse", grabMouse ? 1 : 0) != 0;
        debugOverlay = cfg.get<int>("debug", "overlay", debugOverlay ? 1 : 0) != 0;

        masterVolume = cfg.get<float>("audio", "master_volume", masterVolume);

        sanitize();

        return true;
    }

    void Settings::sanitize()
    {
        Settings defaults;

        if(windowWidth < 1 || windowHeight < 1)
        {
            Logger::warn() << "Invalid window size " << windowWidth << "x" << windowHeight << " in config. Using default";
            windowWidth = defaults.windowWidth;
            windowHeight = defaults.windowHeight;
        }

        if(screen < 0)
        {
            Logger::warn() << "Invalid screen index " << screen << " in config. Using default";
            screen = defaults.screen;
        }

        if(horizontalFov <= 0 || horizontalFov >= 180)
        {
            Logger::warn() << "Invalid field of view " << horizontalFov << " in config. Using default";
            horizontalFov = defaults.horizontalFov;
        }

        if(masterVolume < 0.0f || masterVolume > 1.0f)
        {
            Logger::warn() << "Master volume " << masterVolume << " in config is outside of 0..1. Clamping";
            masterVolume = masterVolume < 0.0f ? 0.0f : 1.0f;
        }
    }

}
