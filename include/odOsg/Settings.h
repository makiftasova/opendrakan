/*
 * Settings.h
 */

#ifndef INCLUDE_ODOSG_SETTINGS_H_
#define INCLUDE_ODOSG_SETTINGS_H_

#include <string>

#include <odCore/FilePath.h>

namespace odOsg
{

    /**
     * @brief User-configurable settings of the odOsg executable. Loaded from an INI-style config file (odOsg.cfg).
     *
     * Every value has a sane default, so the config file and every key in it are optional.
     */
    struct Settings
    {
        static constexpr const char *DEFAULT_FILE_NAME = "odOsg.cfg";

        // [video]
        int windowWidth = 1280;
        int windowHeight = 720;
        bool fullscreen = false;
        int screen = 0;             ///< Index of the screen to open the window on
        double horizontalFov = 90.0; ///< Horizontal field of view of the 3D camera in degrees (the vertical one follows from the window's aspect ratio)

        // [input]
        bool grabMouse = true;      ///< Keep the mouse pointer inside the window while it has the focus

        // [debug]
        bool debugOverlay = true;   ///< Show the debug overlay (frame rate, player position, terrain under the player)

        // [audio]
        float masterVolume = 1.0f;  ///< 0.0 (silent) to 1.0 (full volume)

        /**
         * @brief Loads values from the given file, leaving defaults in place for everything not mentioned in it.
         *
         * @return true if the file was found and parsed. If it does not exist, nothing is changed.
         */
        bool loadFromFile(const od::FilePath &path);

        /**
         * @brief Checks the loaded values and replaces invalid ones with defaults (logging a warning for each).
         */
        void sanitize();
    };

}

#endif /* INCLUDE_ODOSG_SETTINGS_H_ */
