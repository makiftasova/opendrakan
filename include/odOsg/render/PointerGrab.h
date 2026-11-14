/*
 * PointerGrab.h
 */

#ifndef INCLUDE_ODOSG_RENDER_POINTERGRAB_H_
#define INCLUDE_ODOSG_RENDER_POINTERGRAB_H_

namespace osgViewer
{
    class GraphicsWindow;
}

namespace odOsg
{

    /**
     * @brief Confines the mouse pointer to a window, so mouse movement can't leave it while the player is turning around.
     *
     * OpenSceneGraph has no support for this, so we use the native window system. This is only implemented for X11 (which
     * includes games running through XWayland). On other window systems, all functions do nothing and report failure.
     */
    class PointerGrab
    {
    public:

        PointerGrab() = delete;

        /**
         * @brief Returns true if grabbing the pointer is supported for the given window.
         */
        static bool isSupported(osgViewer::GraphicsWindow *window);

        /**
         * @brief Returns true if the given window currently has the keyboard focus.
         */
        static bool hasFocus(osgViewer::GraphicsWindow *window);

        /**
         * @brief Grabs the pointer and confines it to the window, or releases it again.
         * @return true if the pointer is now in the requested state.
         */
        static bool setGrab(osgViewer::GraphicsWindow *window, bool grab);

    };

}

#endif /* INCLUDE_ODOSG_RENDER_POINTERGRAB_H_ */
