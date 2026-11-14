/*
 * PointerGrab.cpp
 */

#include <odOsg/render/PointerGrab.h>

#include <osgViewer/GraphicsWindow>

#ifdef OD_HAVE_X11
    // note: Xlib defines a lot of macros (None, Bool, ...), so this has to stay in this file
    #include <osgViewer/api/X11/GraphicsWindowX11>
    #include <X11/Xlib.h>
#endif

namespace odOsg
{

#ifdef OD_HAVE_X11

    static osgViewer::GraphicsWindowX11 *_asX11Window(osgViewer::GraphicsWindow *window)
    {
        return dynamic_cast<osgViewer::GraphicsWindowX11*>(window);
    }

    bool PointerGrab::isSupported(osgViewer::GraphicsWindow *window)
    {
        return _asX11Window(window) != nullptr;
    }

    bool PointerGrab::hasFocus(osgViewer::GraphicsWindow *window)
    {
        auto x11Window = _asX11Window(window);
        if(x11Window == nullptr)
        {
            return false;
        }

        Display *display = x11Window->getDisplayToUse();
        if(display == nullptr)
        {
            return false;
        }

        ::Window focused = 0;
        int revertTo = 0;
        XGetInputFocus(display, &focused, &revertTo);

        return focused == x11Window->getWindow();
    }

    bool PointerGrab::setGrab(osgViewer::GraphicsWindow *window, bool grab)
    {
        auto x11Window = _asX11Window(window);
        if(x11Window == nullptr)
        {
            return false;
        }

        Display *display = x11Window->getDisplayToUse();
        if(display == nullptr)
        {
            return false;
        }

        ::Window nativeWindow = x11Window->getWindow();

        if(grab)
        {
            // confine the pointer to our window. we keep whatever cursor the window has (the game hides it).
            int result = XGrabPointer(display, nativeWindow, True,
                                      ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                                      GrabModeAsync, GrabModeAsync,
                                      nativeWindow, None, CurrentTime);
            XFlush(display);

            return result == GrabSuccess;

        }else
        {
            XUngrabPointer(display, CurrentTime);
            XFlush(display);

            return true;
        }
    }

#else

    bool PointerGrab::isSupported(osgViewer::GraphicsWindow *window)
    {
        return false;
    }

    bool PointerGrab::hasFocus(osgViewer::GraphicsWindow *window)
    {
        return false;
    }

    bool PointerGrab::setGrab(osgViewer::GraphicsWindow *window, bool grab)
    {
        return false;
    }

#endif

}
