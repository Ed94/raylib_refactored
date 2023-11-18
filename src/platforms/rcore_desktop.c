/**********************************************************************************************
*
*   rcore_desktop - Functions to manage window, graphics device and inputs
*
*   PLATFORM: DESKTOP: GLFW
*       - Windows (Win32, Win64)
*       - Linux (X11/Wayland desktop mode)
*       - FreeBSD, OpenBSD, NetBSD, DragonFly (X11 desktop)
*       - OSX/macOS (x64, arm64)
*
*   LIMITATIONS:
*       - Limitation 01
*       - Limitation 02
*
*   POSSIBLE IMPROVEMENTS:
*       - Improvement 01
*       - Improvement 02
*
*   ADDITIONAL NOTES:
*       - rlTRACELOG() function is located in raylib [utils] module
*
*   CONFIGURATION:
*       #define RCORE_PLATFORM_CUSTOM_FLAG
*           Custom flag for rcore on target platform -not used-
*
*   DEPENDENCIES:
*       - rglfw: Manage graphic device, OpenGL context and inputs (Windows, Linux, OSX, FreeBSD...)
*       - gestures: Gestures system for touch-ready devices (or simulated from mouse inputs)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2013-2023 Ramon Santamaria (@raysan5) and contributors
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#define GLFW_INCLUDE_NONE       // Disable the standard OpenGL header inclusion on GLFW3
                                // NOTE: Already provided by rlgl implementation (on glad.h)
#include "GLFW/glfw3.h"         // GLFW3 library: Windows, OpenGL context and Input management
                                // NOTE: GLFW3 already includes gl.h (OpenGL) headers

// Support retrieving native window handlers
#if defined(_WIN32)
    typedef void *PVOID;
    typedef PVOID HANDLE;
    typedef HANDLE HWND;
    #define GLFW_EXPOSE_NATIVE_WIN32
    #define GLFW_NATIVE_INCLUDE_NONE // To avoid some symbols re-definition in windows.h
    #include "GLFW/glfw3native.h"

    #if defined(RL_SUPPORT_WINMM_HIGHRES_TIMER) && !defined(RL_SUPPORT_BUSY_WAIT_LOOP)
        // NOTE: Those functions require linking with winmm library
    #if __cplusplus
        extern "C" {
    #endif
        unsigned int __stdcall timeBeginPeriod(unsigned int uPeriod);
        unsigned int __stdcall timeEndPeriod(unsigned int uPeriod);
    #if __cplusplus
        }
    #endif
    #endif
#endif
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
    #include <sys/time.h>               // Required for: timespec, nanosleep(), select() - POSIX

    //#define GLFW_EXPOSE_NATIVE_X11      // WARNING: Exposing Xlib.h > X.h results in dup symbols for rlFont type
    //#define GLFW_EXPOSE_NATIVE_WAYLAND
    //#define GLFW_EXPOSE_NATIVE_MIR
    #include "GLFW/glfw3native.h"       // Required for: glfwGetX11Window()
#endif
#if defined(__APPLE__)
    #include <unistd.h>                 // Required for: usleep()

    //#define GLFW_EXPOSE_NATIVE_COCOA    // WARNING: Fails due to type redefinition
    void *glfwGetCocoaWindow(GLFWwindow* handle);
    #include "GLFW/glfw3native.h"       // Required for: glfwGetCocoaWindow()
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// TODO: HACK: Added flag if not provided by GLFW when using external library
// Latest GLFW release (GLFW 3.3.8) does not implement this flag, it was added for 3.4.0-dev
#if !defined(GLFW_MOUSE_PASSTHROUGH)
    #define GLFW_MOUSE_PASSTHROUGH      0x0002000D
#endif

#ifndef RL_NS_BEGIN
#define RL_NS_BEGIN
#define RL_NS_END
#endif

RL_NS_BEGIN

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    GLFWwindow *handle;                 // GLFW window handle (graphic device)
} PlatformData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern rlCoreData rlCORE;                   // Global rlCORE state context

static PlatformData platform = { 0 };   // Platform specific data

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
int InitPlatform(void);          // Initialize platform (graphics, inputs and more)
void ClosePlatform(void);        // Close platform

// Error callback event
static void ErrorCallback(int error, const char *description);                             // GLFW3 Error Callback, runs on GLFW3 error

// Window callbacks events
static void WindowSizeCallback(GLFWwindow *window, int width, int height);                 // GLFW3 WindowSize Callback, runs when window is resized
static void WindowIconifyCallback(GLFWwindow *window, int iconified);                      // GLFW3 WindowIconify Callback, runs when window is minimized/restored
static void WindowMaximizeCallback(GLFWwindow* window, int maximized);                     // GLFW3 Window Maximize Callback, runs when window is maximized
static void WindowFocusCallback(GLFWwindow *window, int focused);                          // GLFW3 WindowFocus Callback, runs when window get/lose focus
static void WindowDropCallback(GLFWwindow *window, int count, const char **paths);         // GLFW3 Window Drop Callback, runs when drop files into window

// Input callbacks events
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);  // GLFW3 Keyboard Callback, runs on key pressed
static void CharCallback(GLFWwindow *window, unsigned int key);                            // GLFW3 Char Key Callback, runs on key pressed (get char value)
static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);     // GLFW3 Mouse Button Callback, runs on mouse button pressed
static void MouseCursorPosCallback(GLFWwindow *window, double x, double y);                // GLFW3 Cursor Position Callback, runs on mouse move
static void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset);       // GLFW3 Srolling Callback, runs on mouse wheel
static void CursorEnterCallback(GLFWwindow *window, int enter);                            // GLFW3 Cursor Enter Callback, cursor enters client area
static void JoystickCallback(int jid, int event);                                           // GLFW3 Joystick Connected/Disconnected Callback

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
// NOTE: Functions declaration is provided by raylib.h

//----------------------------------------------------------------------------------
// Module Functions Definition: Window and Graphics Device
//----------------------------------------------------------------------------------

// Check if application should close
// NOTE: By default, if RL_KEY_ESCAPE pressed or window close icon clicked
bool rlWindowShouldClose(void)
{
    if (rlCORE.Window.ready) return rlCORE.Window.shouldClose;
    else return true;
}

// Toggle fullscreen mode
void rlToggleFullscreen(void)
{
    if (!rlCORE.Window.fullscreen)
    {
        // Store previous window position (in case we exit fullscreen)
        glfwGetWindowPos(platform.handle, &rlCORE.Window.position.x, &rlCORE.Window.position.y);

        int monitorCount = 0;
        int monitorIndex = rlGetCurrentMonitor();
        GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

        // Use current monitor, so we correctly get the display the window is on
        GLFWmonitor *monitor = (monitorIndex < monitorCount)? monitors[monitorIndex] : NULL;

        if (monitor == NULL)
        {
            rlTRACELOG(RL_LOG_WARNING, "GLFW: Failed to get monitor");

            rlCORE.Window.fullscreen = false;
            rlCORE.Window.flags &= ~RL_FLAG_FULLSCREEN_MODE;

            glfwSetWindowMonitor(platform.handle, NULL, 0, 0, rlCORE.Window.screen.width, rlCORE.Window.screen.height, GLFW_DONT_CARE);
        }
        else
        {
            rlCORE.Window.fullscreen = true;
            rlCORE.Window.flags |= RL_FLAG_FULLSCREEN_MODE;

            glfwSetWindowMonitor(platform.handle, monitor, 0, 0, rlCORE.Window.screen.width, rlCORE.Window.screen.height, GLFW_DONT_CARE);
        }

    }
    else
    {
        rlCORE.Window.fullscreen = false;
        rlCORE.Window.flags &= ~RL_FLAG_FULLSCREEN_MODE;

        glfwSetWindowMonitor(platform.handle, NULL, rlCORE.Window.position.x, rlCORE.Window.position.y, rlCORE.Window.screen.width, rlCORE.Window.screen.height, GLFW_DONT_CARE);
    }

    // Try to enable GPU V-Sync, so frames are limited to screen refresh rate (60Hz -> 60 FPS)
    // NOTE: V-Sync can be enabled by graphic driver configuration
    if (rlCORE.Window.flags & RL_FLAG_VSYNC_HINT) glfwSwapInterval(1);
}

// Toggle borderless windowed mode
void rlToggleBorderlessWindowed(void)
{
    // Leave fullscreen before attempting to set borderless windowed mode and get screen position from it
    bool wasOnFullscreen = false;
    if (rlCORE.Window.fullscreen)
    {
        rlCORE.Window.previousPosition = rlCORE.Window.position;
        rlToggleFullscreen();
        wasOnFullscreen = true;
    }

    const int monitor = rlGetCurrentMonitor();
    int monitorCount;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);

        if (mode)
        {
            if (!rlIsWindowState(RL_FLAG_BORDERLESS_WINDOWED_MODE))
            {
                // Store screen position and size
                // NOTE: If it was on fullscreen, screen position was already stored, so skip setting it here
                if (!wasOnFullscreen) glfwGetWindowPos(platform.handle, &rlCORE.Window.previousPosition.x, &rlCORE.Window.previousPosition.y);
                rlCORE.Window.previousScreen = rlCORE.Window.screen;

                // Set undecorated and topmost modes and flags
                glfwSetWindowAttrib(platform.handle, GLFW_DECORATED, GLFW_FALSE);
                rlCORE.Window.flags |= RL_FLAG_WINDOW_UNDECORATED;
                glfwSetWindowAttrib(platform.handle, GLFW_FLOATING, GLFW_TRUE);
                rlCORE.Window.flags |= RL_FLAG_WINDOW_TOPMOST;

                // Get monitor position and size
                int monitorPosX = 0;
                int monitorPosY = 0;
                glfwGetMonitorPos(monitors[monitor], &monitorPosX, &monitorPosY);
                const int monitorWidth = mode->width;
                const int monitorHeight = mode->height;

                // Set screen position and size
                glfwSetWindowPos(platform.handle, monitorPosX, monitorPosY);
                glfwSetWindowSize(platform.handle, monitorWidth, monitorHeight);

                // Refocus window
                glfwFocusWindow(platform.handle);

                rlCORE.Window.flags |= RL_FLAG_BORDERLESS_WINDOWED_MODE;
            }
            else
            {
                // Remove topmost and undecorated modes and flags
                glfwSetWindowAttrib(platform.handle, GLFW_FLOATING, GLFW_FALSE);
                rlCORE.Window.flags &= ~RL_FLAG_WINDOW_TOPMOST;
                glfwSetWindowAttrib(platform.handle, GLFW_DECORATED, GLFW_TRUE);
                rlCORE.Window.flags &= ~RL_FLAG_WINDOW_UNDECORATED;

                // Return previous screen size and position
                // NOTE: The order matters here, it must set size first, then set position, otherwise the screen will be positioned incorrectly
                glfwSetWindowSize(platform.handle,  rlCORE.Window.previousScreen.width, rlCORE.Window.previousScreen.height);
                glfwSetWindowPos(platform.handle, rlCORE.Window.previousPosition.x, rlCORE.Window.previousPosition.y);

                // Refocus window
                glfwFocusWindow(platform.handle);

                rlCORE.Window.flags &= ~RL_FLAG_BORDERLESS_WINDOWED_MODE;
            }
        }
        else rlTRACELOG(RL_LOG_WARNING, "GLFW: Failed to find video mode for selected monitor");
    }
    else rlTRACELOG(RL_LOG_WARNING, "GLFW: Failed to find selected monitor");
}

// Set window state: maximized, if resizable
void rlMaximizeWindow(void)
{
    if (glfwGetWindowAttrib(platform.handle, GLFW_RESIZABLE) == GLFW_TRUE)
    {
        glfwMaximizeWindow(platform.handle);
        rlCORE.Window.flags |= RL_FLAG_WINDOW_MAXIMIZED;
    }
}

// Set window state: minimized
void rlMinimizeWindow(void)
{
    // NOTE: Following function launches callback that sets appropriate flag!
    glfwIconifyWindow(platform.handle);
}

// Set window state: not minimized/maximized
void rlRestoreWindow(void)
{
    if (glfwGetWindowAttrib(platform.handle, GLFW_RESIZABLE) == GLFW_TRUE)
    {
        // Restores the specified window if it was previously iconified (minimized) or maximized
        glfwRestoreWindow(platform.handle);
        rlCORE.Window.flags &= ~RL_FLAG_WINDOW_MINIMIZED;
        rlCORE.Window.flags &= ~RL_FLAG_WINDOW_MAXIMIZED;
    }
}

// Set window configuration state using flags
void rlSetWindowState(unsigned int flags)
{
    // Check previous state and requested state to apply required changes
    // NOTE: In most cases the functions already change the flags internally

    // State change: RL_FLAG_VSYNC_HINT
    if (((rlCORE.Window.flags & RL_FLAG_VSYNC_HINT) != (flags & RL_FLAG_VSYNC_HINT)) && ((flags & RL_FLAG_VSYNC_HINT) > 0))
    {
        glfwSwapInterval(1);
        rlCORE.Window.flags |= RL_FLAG_VSYNC_HINT;
    }

    // State change: RL_FLAG_BORDERLESS_WINDOWED_MODE
    // NOTE: This must be handled before RL_FLAG_FULLSCREEN_MODE because rlToggleBorderlessWindowed() needs to get some fullscreen values if fullscreen is running
    if (((rlCORE.Window.flags & RL_FLAG_BORDERLESS_WINDOWED_MODE) != (flags & RL_FLAG_BORDERLESS_WINDOWED_MODE)) && ((flags & RL_FLAG_BORDERLESS_WINDOWED_MODE) > 0))
    {
        rlToggleBorderlessWindowed();     // NOTE: Window state flag updated inside function
    }

    // State change: RL_FLAG_FULLSCREEN_MODE
    if ((rlCORE.Window.flags & RL_FLAG_FULLSCREEN_MODE) != (flags & RL_FLAG_FULLSCREEN_MODE))
    {
        rlToggleFullscreen();     // NOTE: Window state flag updated inside function
    }

    // State change: RL_FLAG_WINDOW_RESIZABLE
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_RESIZABLE) != (flags & RL_FLAG_WINDOW_RESIZABLE)) && ((flags & RL_FLAG_WINDOW_RESIZABLE) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_RESIZABLE, GLFW_TRUE);
        rlCORE.Window.flags |= RL_FLAG_WINDOW_RESIZABLE;
    }

    // State change: RL_FLAG_WINDOW_UNDECORATED
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_UNDECORATED) != (flags & RL_FLAG_WINDOW_UNDECORATED)) && (flags & RL_FLAG_WINDOW_UNDECORATED))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_DECORATED, GLFW_FALSE);
        rlCORE.Window.flags |= RL_FLAG_WINDOW_UNDECORATED;
    }

    // State change: RL_FLAG_WINDOW_HIDDEN
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_HIDDEN) != (flags & RL_FLAG_WINDOW_HIDDEN)) && ((flags & RL_FLAG_WINDOW_HIDDEN) > 0))
    {
        glfwHideWindow(platform.handle);
        rlCORE.Window.flags |= RL_FLAG_WINDOW_HIDDEN;
    }

    // State change: RL_FLAG_WINDOW_MINIMIZED
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_MINIMIZED) != (flags & RL_FLAG_WINDOW_MINIMIZED)) && ((flags & RL_FLAG_WINDOW_MINIMIZED) > 0))
    {
        //GLFW_ICONIFIED
        rlMinimizeWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: RL_FLAG_WINDOW_MAXIMIZED
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_MAXIMIZED) != (flags & RL_FLAG_WINDOW_MAXIMIZED)) && ((flags & RL_FLAG_WINDOW_MAXIMIZED) > 0))
    {
        //GLFW_MAXIMIZED
        rlMaximizeWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: RL_FLAG_WINDOW_UNFOCUSED
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_UNFOCUSED) != (flags & RL_FLAG_WINDOW_UNFOCUSED)) && ((flags & RL_FLAG_WINDOW_UNFOCUSED) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
        rlCORE.Window.flags |= RL_FLAG_WINDOW_UNFOCUSED;
    }

    // State change: RL_FLAG_WINDOW_TOPMOST
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_TOPMOST) != (flags & RL_FLAG_WINDOW_TOPMOST)) && ((flags & RL_FLAG_WINDOW_TOPMOST) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_FLOATING, GLFW_TRUE);
        rlCORE.Window.flags |= RL_FLAG_WINDOW_TOPMOST;
    }

    // State change: RL_FLAG_WINDOW_ALWAYS_RUN
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_ALWAYS_RUN) != (flags & RL_FLAG_WINDOW_ALWAYS_RUN)) && ((flags & RL_FLAG_WINDOW_ALWAYS_RUN) > 0))
    {
        rlCORE.Window.flags |= RL_FLAG_WINDOW_ALWAYS_RUN;
    }

    // The following states can not be changed after window creation

    // State change: RL_FLAG_WINDOW_TRANSPARENT
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_TRANSPARENT) != (flags & RL_FLAG_WINDOW_TRANSPARENT)) && ((flags & RL_FLAG_WINDOW_TRANSPARENT) > 0))
    {
        rlTRACELOG(RL_LOG_WARNING, "WINDOW: Framebuffer transparency can only be configured before window initialization");
    }

    // State change: RL_FLAG_WINDOW_HIGHDPI
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_HIGHDPI) != (flags & RL_FLAG_WINDOW_HIGHDPI)) && ((flags & RL_FLAG_WINDOW_HIGHDPI) > 0))
    {
        rlTRACELOG(RL_LOG_WARNING, "WINDOW: High DPI can only be configured before window initialization");
    }

    // State change: RL_FLAG_WINDOW_MOUSE_PASSTHROUGH
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_MOUSE_PASSTHROUGH) != (flags & RL_FLAG_WINDOW_MOUSE_PASSTHROUGH)) && ((flags & RL_FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
        rlCORE.Window.flags |= RL_FLAG_WINDOW_MOUSE_PASSTHROUGH;
    }

    // State change: RL_FLAG_MSAA_4X_HINT
    if (((rlCORE.Window.flags & RL_FLAG_MSAA_4X_HINT) != (flags & RL_FLAG_MSAA_4X_HINT)) && ((flags & RL_FLAG_MSAA_4X_HINT) > 0))
    {
        rlTRACELOG(RL_LOG_WARNING, "WINDOW: MSAA can only be configured before window initialization");
    }

    // State change: RL_FLAG_INTERLACED_HINT
    if (((rlCORE.Window.flags & RL_FLAG_INTERLACED_HINT) != (flags & RL_FLAG_INTERLACED_HINT)) && ((flags & RL_FLAG_INTERLACED_HINT) > 0))
    {
        rlTRACELOG(RL_LOG_WARNING, "RPI: Interlaced mode can only be configured before window initialization");
    }
}

// Clear window configuration state flags
void rlClearWindowState(unsigned int flags)
{
    // Check previous state and requested state to apply required changes
    // NOTE: In most cases the functions already change the flags internally

    // State change: RL_FLAG_VSYNC_HINT
    if (((rlCORE.Window.flags & RL_FLAG_VSYNC_HINT) > 0) && ((flags & RL_FLAG_VSYNC_HINT) > 0))
    {
        glfwSwapInterval(0);
        rlCORE.Window.flags &= ~RL_FLAG_VSYNC_HINT;
    }

    // State change: RL_FLAG_BORDERLESS_WINDOWED_MODE
    // NOTE: This must be handled before RL_FLAG_FULLSCREEN_MODE because rlToggleBorderlessWindowed() needs to get some fullscreen values if fullscreen is running
    if (((rlCORE.Window.flags & RL_FLAG_BORDERLESS_WINDOWED_MODE) > 0) && ((flags & RL_FLAG_BORDERLESS_WINDOWED_MODE) > 0))
    {
        rlToggleBorderlessWindowed();     // NOTE: Window state flag updated inside function
    }

    // State change: RL_FLAG_FULLSCREEN_MODE
    if (((rlCORE.Window.flags & RL_FLAG_FULLSCREEN_MODE) > 0) && ((flags & RL_FLAG_FULLSCREEN_MODE) > 0))
    {
        rlToggleFullscreen();     // NOTE: Window state flag updated inside function
    }

    // State change: RL_FLAG_WINDOW_RESIZABLE
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_RESIZABLE) > 0) && ((flags & RL_FLAG_WINDOW_RESIZABLE) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_RESIZABLE, GLFW_FALSE);
        rlCORE.Window.flags &= ~RL_FLAG_WINDOW_RESIZABLE;
    }

    // State change: RL_FLAG_WINDOW_HIDDEN
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_HIDDEN) > 0) && ((flags & RL_FLAG_WINDOW_HIDDEN) > 0))
    {
        glfwShowWindow(platform.handle);
        rlCORE.Window.flags &= ~RL_FLAG_WINDOW_HIDDEN;
    }

    // State change: RL_FLAG_WINDOW_MINIMIZED
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_MINIMIZED) > 0) && ((flags & RL_FLAG_WINDOW_MINIMIZED) > 0))
    {
        rlRestoreWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: RL_FLAG_WINDOW_MAXIMIZED
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_MAXIMIZED) > 0) && ((flags & RL_FLAG_WINDOW_MAXIMIZED) > 0))
    {
        rlRestoreWindow();       // NOTE: Window state flag updated inside function
    }

    // State change: RL_FLAG_WINDOW_UNDECORATED
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_UNDECORATED) > 0) && ((flags & RL_FLAG_WINDOW_UNDECORATED) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_DECORATED, GLFW_TRUE);
        rlCORE.Window.flags &= ~RL_FLAG_WINDOW_UNDECORATED;
    }

    // State change: RL_FLAG_WINDOW_UNFOCUSED
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_UNFOCUSED) > 0) && ((flags & RL_FLAG_WINDOW_UNFOCUSED) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
        rlCORE.Window.flags &= ~RL_FLAG_WINDOW_UNFOCUSED;
    }

    // State change: RL_FLAG_WINDOW_TOPMOST
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_TOPMOST) > 0) && ((flags & RL_FLAG_WINDOW_TOPMOST) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_FLOATING, GLFW_FALSE);
        rlCORE.Window.flags &= ~RL_FLAG_WINDOW_TOPMOST;
    }

    // State change: RL_FLAG_WINDOW_ALWAYS_RUN
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_ALWAYS_RUN) > 0) && ((flags & RL_FLAG_WINDOW_ALWAYS_RUN) > 0))
    {
        rlCORE.Window.flags &= ~RL_FLAG_WINDOW_ALWAYS_RUN;
    }

    // The following states can not be changed after window creation

    // State change: RL_FLAG_WINDOW_TRANSPARENT
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_TRANSPARENT) > 0) && ((flags & RL_FLAG_WINDOW_TRANSPARENT) > 0))
    {
        rlTRACELOG(RL_LOG_WARNING, "WINDOW: Framebuffer transparency can only be configured before window initialization");
    }

    // State change: RL_FLAG_WINDOW_HIGHDPI
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_HIGHDPI) > 0) && ((flags & RL_FLAG_WINDOW_HIGHDPI) > 0))
    {
        rlTRACELOG(RL_LOG_WARNING, "WINDOW: High DPI can only be configured before window initialization");
    }

    // State change: RL_FLAG_WINDOW_MOUSE_PASSTHROUGH
    if (((rlCORE.Window.flags & RL_FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0) && ((flags & RL_FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0))
    {
        glfwSetWindowAttrib(platform.handle, GLFW_MOUSE_PASSTHROUGH, GLFW_FALSE);
        rlCORE.Window.flags &= ~RL_FLAG_WINDOW_MOUSE_PASSTHROUGH;
    }

    // State change: RL_FLAG_MSAA_4X_HINT
    if (((rlCORE.Window.flags & RL_FLAG_MSAA_4X_HINT) > 0) && ((flags & RL_FLAG_MSAA_4X_HINT) > 0))
    {
        rlTRACELOG(RL_LOG_WARNING, "WINDOW: MSAA can only be configured before window initialization");
    }

    // State change: RL_FLAG_INTERLACED_HINT
    if (((rlCORE.Window.flags & RL_FLAG_INTERLACED_HINT) > 0) && ((flags & RL_FLAG_INTERLACED_HINT) > 0))
    {
        rlTRACELOG(RL_LOG_WARNING, "RPI: Interlaced mode can only be configured before window initialization");
    }
}

// Set icon for window
// NOTE 1: rlImage must be in RGBA format, 8bit per channel
// NOTE 2: rlImage is scaled by the OS for all required sizes
void rlSetWindowIcon(rlImage image)
{
    if (image.data == NULL)
    {
        // Revert to the default window icon, pass in an empty image array
        glfwSetWindowIcon(platform.handle, 0, NULL);
    }
    else
    {
        if (image.format == RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
        {
            GLFWimage icon[1] = { 0 };

            icon[0].width = image.width;
            icon[0].height = image.height;
            icon[0].pixels = (unsigned char *)image.data;

            // NOTE 1: We only support one image icon
            // NOTE 2: The specified image data is copied before this function returns
            glfwSetWindowIcon(platform.handle, 1, icon);
        }
        else rlTRACELOG(RL_LOG_WARNING, "GLFW: Window icon image must be in R8G8B8A8 pixel format");
    }
}

// Set icon for window, multiple images
// NOTE 1: Images must be in RGBA format, 8bit per channel
// NOTE 2: The multiple images are used depending on provided sizes
// Standard Windows icon sizes: 256, 128, 96, 64, 48, 32, 24, 16
void rlSetWindowIcons(rlImage *images, int count)
{
    if ((images == NULL) || (count <= 0))
    {
        // Revert to the default window icon, pass in an empty image array
        glfwSetWindowIcon(platform.handle, 0, NULL);
    }
    else
    {
        int valid = 0;
        GLFWimage *icons = (GLFWimage*)RL_CALLOC(count, sizeof(GLFWimage));

        for (int i = 0; i < count; i++)
        {
            if (images[i].format == RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
            {
                icons[valid].width = images[i].width;
                icons[valid].height = images[i].height;
                icons[valid].pixels = (unsigned char *)images[i].data;

                valid++;
            }
            else rlTRACELOG(RL_LOG_WARNING, "GLFW: Window icon image must be in R8G8B8A8 pixel format");
        }
        // NOTE: Images data is copied internally before this function returns
        glfwSetWindowIcon(platform.handle, valid, icons);

        RL_FREE(icons);
    }
}

// Set title for window
void rlSetWindowTitle(const char *title)
{
    rlCORE.Window.title = title;
    glfwSetWindowTitle(platform.handle, title);
}

// Set window position on screen (windowed mode)
void rlSetWindowPosition(int x, int y)
{
    glfwSetWindowPos(platform.handle, x, y);
}

// Set monitor for the current window
void rlSetWindowMonitor(int monitor)
{
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        if (rlCORE.Window.fullscreen)
        {
            rlTRACELOG(RL_LOG_INFO, "GLFW: Selected fullscreen monitor: [%i] %s", monitor, glfwGetMonitorName(monitors[monitor]));

            const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);
            glfwSetWindowMonitor(platform.handle, monitors[monitor], 0, 0, mode->width, mode->height, mode->refreshRate);
        }
        else
        {
            rlTRACELOG(RL_LOG_INFO, "GLFW: Selected monitor: [%i] %s", monitor, glfwGetMonitorName(monitors[monitor]));

            const int screenWidth = rlCORE.Window.screen.width;
            const int screenHeight = rlCORE.Window.screen.height;
            int monitorWorkareaX = 0;
            int monitorWorkareaY = 0;
            int monitorWorkareaWidth = 0;
            int monitorWorkareaHeight = 0;
            glfwGetMonitorWorkarea(monitors[monitor], &monitorWorkareaX, &monitorWorkareaY, &monitorWorkareaWidth, &monitorWorkareaHeight);

            // If the screen size is larger than the monitor workarea, anchor it on the top left corner, otherwise, center it
            if ((screenWidth >= monitorWorkareaWidth) || (screenHeight >= monitorWorkareaHeight)) glfwSetWindowPos(platform.handle, monitorWorkareaX, monitorWorkareaY);
            else
            {
                const int x = monitorWorkareaX + (monitorWorkareaWidth/2) - (screenWidth/2);
                const int y = monitorWorkareaY + (monitorWorkareaHeight/2) - (screenHeight/2);
                glfwSetWindowPos(platform.handle, x, y);
            }
        }
    }
    else rlTRACELOG(RL_LOG_WARNING, "GLFW: Failed to find selected monitor");
}

// Set window minimum dimensions (RL_FLAG_WINDOW_RESIZABLE)
void rlSetWindowMinSize(int width, int height)
{
    rlCORE.Window.screenMin.width = width;
    rlCORE.Window.screenMin.height = height;

    int minWidth  = (rlCORE.Window.screenMin.width  == 0)? GLFW_DONT_CARE : (int)rlCORE.Window.screenMin.width;
    int minHeight = (rlCORE.Window.screenMin.height == 0)? GLFW_DONT_CARE : (int)rlCORE.Window.screenMin.height;
    int maxWidth  = (rlCORE.Window.screenMax.width  == 0)? GLFW_DONT_CARE : (int)rlCORE.Window.screenMax.width;
    int maxHeight = (rlCORE.Window.screenMax.height == 0)? GLFW_DONT_CARE : (int)rlCORE.Window.screenMax.height;

    glfwSetWindowSizeLimits(platform.handle, minWidth, minHeight, maxWidth, maxHeight);
}

// Set window maximum dimensions (RL_FLAG_WINDOW_RESIZABLE)
void rlSetWindowMaxSize(int width, int height)
{
    rlCORE.Window.screenMax.width = width;
    rlCORE.Window.screenMax.height = height;

    int minWidth  = (rlCORE.Window.screenMin.width  == 0)? GLFW_DONT_CARE : (int)rlCORE.Window.screenMin.width;
    int minHeight = (rlCORE.Window.screenMin.height == 0)? GLFW_DONT_CARE : (int)rlCORE.Window.screenMin.height;
    int maxWidth  = (rlCORE.Window.screenMax.width  == 0)? GLFW_DONT_CARE : (int)rlCORE.Window.screenMax.width;
    int maxHeight = (rlCORE.Window.screenMax.height == 0)? GLFW_DONT_CARE : (int)rlCORE.Window.screenMax.height;

    glfwSetWindowSizeLimits(platform.handle, minWidth, minHeight, maxWidth, maxHeight);
}

// Set window dimensions
void rlSetWindowSize(int width, int height)
{
    glfwSetWindowSize(platform.handle, width, height);
}

// Set window opacity, value opacity is between 0.0 and 1.0
void rlSetWindowOpacity(float opacity)
{
    if (opacity >= 1.0f) opacity = 1.0f;
    else if (opacity <= 0.0f) opacity = 0.0f;
    glfwSetWindowOpacity(platform.handle, opacity);
}

// Set window focused
void rlSetWindowFocused(void)
{
    glfwFocusWindow(platform.handle);
}

// Get native window handle
void *rlGetWindowHandle(void)
{
#if defined(_WIN32)
    // NOTE: Returned handle is: void *HWND (windows.h)
    return glfwGetWin32Window(platform.handle);
#endif
#if defined(__linux__)
    // NOTE: Returned handle is: unsigned long Window (X.h)
    // typedef unsigned long XID;
    // typedef XID Window;
    //unsigned long id = (unsigned long)glfwGetX11Window(platform.handle);
    //return NULL;    // TODO: Find a way to return value... cast to void *?
    return (void *)platform.handle;
#endif
#if defined(__APPLE__)
    // NOTE: Returned handle is: (objc_object *)
    return (void *)glfwGetCocoaWindow(platform.handle);
#endif

    return NULL;
}

// Get number of monitors
int rlGetMonitorCount(void)
{
    int monitorCount = 0;

    glfwGetMonitors(&monitorCount);

    return monitorCount;
}

// Get number of monitors
int rlGetCurrentMonitor(void)
{
    int index = 0;
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);
    GLFWmonitor *monitor = NULL;

    if (monitorCount >= 1)
    {
        if (rlIsWindowFullscreen())
        {
            // Get the handle of the monitor that the specified window is in full screen on
            monitor = glfwGetWindowMonitor(platform.handle);

            for (int i = 0; i < monitorCount; i++)
            {
                if (monitors[i] == monitor)
                {
                    index = i;
                    break;
                }
            }
        }
        else
        {
            // In case the window is between two monitors, we use below logic
            // to try to detect the "current monitor" for that window, note that
            // this is probably an overengineered solution for a very side case
            // trying to match SDL behaviour

            int closestDist = 0x7FFFFFFF;

            // Window center position
            int wcx = 0;
            int wcy = 0;

            glfwGetWindowPos(platform.handle, &wcx, &wcy);
            wcx += (int)rlCORE.Window.screen.width/2;
            wcy += (int)rlCORE.Window.screen.height/2;

            for (int i = 0; i < monitorCount; i++)
            {
                // Monitor top-left position
                int mx = 0;
                int my = 0;

                monitor = monitors[i];
                glfwGetMonitorPos(monitor, &mx, &my);
                const GLFWvidmode *mode = glfwGetVideoMode(monitor);

                if (mode)
                {
                    const int right = mx + mode->width - 1;
                    const int bottom = my + mode->height - 1;

                    if ((wcx >= mx) &&
                        (wcx <= right) &&
                        (wcy >= my) &&
                        (wcy <= bottom))
                    {
                        index = i;
                        break;
                    }

                    int xclosest = wcx;
                    if (wcx < mx) xclosest = mx;
                    else if (wcx > right) xclosest = right;

                    int yclosest = wcy;
                    if (wcy < my) yclosest = my;
                    else if (wcy > bottom) yclosest = bottom;

                    int dx = wcx - xclosest;
                    int dy = wcy - yclosest;
                    int dist = (dx*dx) + (dy*dy);
                    if (dist < closestDist)
                    {
                        index = i;
                        closestDist = dist;
                    }
                }
                else rlTRACELOG(RL_LOG_WARNING, "GLFW: Failed to find video mode for selected monitor");
            }
        }
    }

    return index;
}

// Get selected monitor position
rlVector2 rlGetMonitorPosition(int monitor)
{
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        int x, y;
        glfwGetMonitorPos(monitors[monitor], &x, &y);

    #ifdef __cplusplus
        return rlVector2{ (float)x, (float)y };
    #else
        return (rlVector2){ (float)x, (float)y };
    #endif
    }
    else rlTRACELOG(RL_LOG_WARNING, "GLFW: Failed to find selected monitor");

    #ifdef __cplusplus
        return rlVector2{ 0, 0 };
    #else
        return (rlVector2){ 0, 0 };
    #endif
}

// Get selected monitor width (currently used by monitor)
int rlGetMonitorWidth(int monitor)
{
    int width = 0;
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);

        if (mode) width = mode->width;
        else rlTRACELOG(RL_LOG_WARNING, "GLFW: Failed to find video mode for selected monitor");
    }
    else rlTRACELOG(RL_LOG_WARNING, "GLFW: Failed to find selected monitor");

    return width;
}

// Get selected monitor height (currently used by monitor)
int rlGetMonitorHeight(int monitor)
{
    int height = 0;
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        const GLFWvidmode *mode = glfwGetVideoMode(monitors[monitor]);

        if (mode) height = mode->height;
        else rlTRACELOG(RL_LOG_WARNING, "GLFW: Failed to find video mode for selected monitor");
    }
    else rlTRACELOG(RL_LOG_WARNING, "GLFW: Failed to find selected monitor");

    return height;
}

// Get selected monitor physical width in millimetres
int rlGetMonitorPhysicalWidth(int monitor)
{
    int width = 0;
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount)) glfwGetMonitorPhysicalSize(monitors[monitor], &width, NULL);
    else rlTRACELOG(RL_LOG_WARNING, "GLFW: Failed to find selected monitor");

    return width;
}

// Get selected monitor physical height in millimetres
int rlGetMonitorPhysicalHeight(int monitor)
{
    int height = 0;
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount)) glfwGetMonitorPhysicalSize(monitors[monitor], NULL, &height);
    else rlTRACELOG(RL_LOG_WARNING, "GLFW: Failed to find selected monitor");

    return height;
}

// Get selected monitor refresh rate
int rlGetMonitorRefreshRate(int monitor)
{
    int refresh = 0;
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        const GLFWvidmode *vidmode = glfwGetVideoMode(monitors[monitor]);
        refresh = vidmode->refreshRate;
    }
    else rlTRACELOG(RL_LOG_WARNING, "GLFW: Failed to find selected monitor");

    return refresh;
}

// Get the human-readable, UTF-8 encoded name of the selected monitor
const char *rlGetMonitorName(int monitor)
{
    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    if ((monitor >= 0) && (monitor < monitorCount))
    {
        return glfwGetMonitorName(monitors[monitor]);
    }
    else rlTRACELOG(RL_LOG_WARNING, "GLFW: Failed to find selected monitor");
    return "";
}

// Get window position XY on monitor
rlVector2 rlGetWindowPosition(void)
{
    int x = 0;
    int y = 0;

    glfwGetWindowPos(platform.handle, &x, &y);



#if __cplusplus
    return rlVector2{ (float)x, (float)y };
#else
    return (rlVector2){ (float)x, (float)y };
#endif
}

// Get window scale DPI factor for current monitor
rlVector2 rlGetWindowScaleDPI(void)
{
    float xdpi = 1.0;
    float ydpi = 1.0;
    rlVector2 scale = { 1.0f, 1.0f };
    rlVector2 windowPos = rlGetWindowPosition();

    int monitorCount = 0;
    GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);

    // Check window monitor
    for (int i = 0; i < monitorCount; i++)
    {
        glfwGetMonitorContentScale(monitors[i], &xdpi, &ydpi);

        int xpos, ypos, width, height;
        glfwGetMonitorWorkarea(monitors[i], &xpos, &ypos, &width, &height);

        if ((windowPos.x >= xpos) && (windowPos.x < xpos + width) &&
            (windowPos.y >= ypos) && (windowPos.y < ypos + height))
        {
            scale.x = xdpi;
            scale.y = ydpi;
            break;
        }
    }

    return scale;
}

// Set clipboard text content
void rlSetClipboardText(const char *text)
{
    glfwSetClipboardString(platform.handle, text);
}

// Get clipboard text content
// NOTE: returned string is allocated and freed by GLFW
const char *rlGetClipboardText(void)
{
    return glfwGetClipboardString(platform.handle);
}

// Show mouse cursor
void rlShowCursor(void)
{
    glfwSetInputMode(platform.handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    rlCORE.Input.Mouse.cursorHidden = false;
}

// Hides mouse cursor
void rlHideCursor(void)
{
    glfwSetInputMode(platform.handle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    rlCORE.Input.Mouse.cursorHidden = true;
}

// Enables cursor (unlock cursor)
void rlEnableCursor(void)
{
    glfwSetInputMode(platform.handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // Set cursor position in the middle
    rlSetMousePosition(rlCORE.Window.screen.width/2, rlCORE.Window.screen.height/2);

    rlCORE.Input.Mouse.cursorHidden = false;
}

// Disables cursor (lock cursor)
void rlDisableCursor(void)
{
    glfwSetInputMode(platform.handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set cursor position in the middle
    rlSetMousePosition(rlCORE.Window.screen.width/2, rlCORE.Window.screen.height/2);

    rlCORE.Input.Mouse.cursorHidden = true;
}

// Swap back buffer with front buffer (screen drawing)
void rlSwapScreenBuffer(void)
{
    glfwSwapBuffers(platform.handle);
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

// Get elapsed time measure in seconds since InitTimer()
double rlGetTime(void)
{
    double time = glfwGetTime();   // Elapsed time since glfwInit()
    return time;
}

// Open URL with default system browser (if available)
// NOTE: This function is only safe to use if you control the URL given.
// A user could craft a malicious string performing another action.
// Only call this function yourself not with user input or make sure to check the string yourself.
// Ref: https://github.com/raysan5/raylib/issues/686
void rlOpenURL(const char *url)
{
    // Security check to (partially) avoid malicious code
    if (strchr(url, '\'') != NULL) rlTRACELOG(RL_LOG_WARNING, "SYSTEM: Provided URL could be potentially malicious, avoid [\'] character");
    else
    {
        char *cmd = (char *)RL_CALLOC(strlen(url) + 32, sizeof(char));
#if defined(_WIN32)
        sprintf(cmd, "explorer \"%s\"", url);
#endif
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)
        sprintf(cmd, "xdg-open '%s'", url); // Alternatives: firefox, x-www-browser
#endif
#if defined(__APPLE__)
        sprintf(cmd, "open '%s'", url);
#endif
        int result = system(cmd);
        if (result == -1) rlTRACELOG(RL_LOG_WARNING, "rlOpenURL() child process could not be created");
        RL_FREE(cmd);
    }
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Inputs
//----------------------------------------------------------------------------------

// Set internal gamepad mappings
int rlSetGamepadMappings(const char *mappings)
{
    return glfwUpdateGamepadMappings(mappings);
}

// Set mouse position XY
void rlSetMousePosition(int x, int y)
{
#if __cplusplus
    rlCORE.Input.Mouse.currentPosition = rlVector2{ (float)x, (float)y };
#else
    rlCORE.Input.Mouse.currentPosition = (rlVector2){ (float)x, (float)y };
#endif
    rlCORE.Input.Mouse.previousPosition = rlCORE.Input.Mouse.currentPosition;

    // NOTE: emscripten not implemented
    glfwSetCursorPos(platform.handle, rlCORE.Input.Mouse.currentPosition.x, rlCORE.Input.Mouse.currentPosition.y);
}

// Set mouse cursor
void rlSetMouseCursor(int cursor)
{
    rlCORE.Input.Mouse.cursor = cursor;
    if (cursor == RL_MOUSE_CURSOR_DEFAULT) glfwSetCursor(platform.handle, NULL);
    else
    {
        // NOTE: We are relating internal GLFW enum values to our rlMouseCursor enum values
        glfwSetCursor(platform.handle, glfwCreateStandardCursor(0x00036000 + cursor));
    }
}

// Register all input events
void rlPollInputEvents(void)
{
#if defined(RL_SUPPORT_GESTURES_SYSTEM)
    // NOTE: Gestures update must be called every frame to reset gestures correctly
    // because rlProcessGestureEvent() is just called on an event, not every frame
    rlUpdateGestures();
#endif

    // Reset keys/chars pressed registered
    rlCORE.Input.Keyboard.keyPressedQueueCount = 0;
    rlCORE.Input.Keyboard.charPressedQueueCount = 0;

    // Reset last gamepad button/axis registered state
    rlCORE.Input.Gamepad.lastButtonPressed = 0;       // RL_GAMEPAD_BUTTON_UNKNOWN
    //rlCORE.Input.Gamepad.axisCount = 0;

    // Keyboard/Mouse input polling (automatically managed by GLFW3 through callback)

    // Register previous keys states
    for (int i = 0; i < RL_MAX_KEYBOARD_KEYS; i++)
    {
        rlCORE.Input.Keyboard.previousKeyState[i] = rlCORE.Input.Keyboard.currentKeyState[i];
        rlCORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    // Register previous mouse states
    for (int i = 0; i < RL_MAX_MOUSE_BUTTONS; i++) rlCORE.Input.Mouse.previousButtonState[i] = rlCORE.Input.Mouse.currentButtonState[i];

    // Register previous mouse wheel state
    rlCORE.Input.Mouse.previousWheelMove = rlCORE.Input.Mouse.currentWheelMove;

#if __cplusplus
    rlCORE.Input.Mouse.currentWheelMove = rlVector2{ 0.0f, 0.0f };
#else
    rlCORE.Input.Mouse.currentWheelMove = (rlVector2){ 0.0f, 0.0f };
#endif

    // Register previous mouse position
    rlCORE.Input.Mouse.previousPosition = rlCORE.Input.Mouse.currentPosition;

    // Register previous touch states
    for (int i = 0; i < RL_MAX_TOUCH_POINTS; i++) rlCORE.Input.Touch.previousTouchState[i] = rlCORE.Input.Touch.currentTouchState[i];

    // Reset touch positions
    //for (int i = 0; i < RL_MAX_TOUCH_POINTS; i++) rlCORE.Input.Touch.position[i] = (rlVector2){ 0, 0 };

    // Map touch position to mouse position for convenience
    // WARNING: If the target desktop device supports touch screen, this behavious should be reviewed!
    // TODO: GLFW does not support multi-touch input just yet
    // https://www.codeproject.com/Articles/668404/Programming-for-Multi-Touch
    // https://docs.microsoft.com/en-us/windows/win32/wintouch/getting-started-with-multi-touch-messages
    rlCORE.Input.Touch.position[0] = rlCORE.Input.Mouse.currentPosition;

    // Check if gamepads are ready
    // NOTE: We do it here in case of disconnection
    for (int i = 0; i < RL_MAX_GAMEPADS; i++)
    {
        if (glfwJoystickPresent(i)) rlCORE.Input.Gamepad.ready[i] = true;
        else rlCORE.Input.Gamepad.ready[i] = false;
    }

    // Register gamepads buttons events
    for (int i = 0; i < RL_MAX_GAMEPADS; i++)
    {
        if (rlCORE.Input.Gamepad.ready[i])     // Check if gamepad is available
        {
            // Register previous gamepad states
            for (int k = 0; k < RL_MAX_GAMEPAD_BUTTONS; k++) rlCORE.Input.Gamepad.previousButtonState[i][k] = rlCORE.Input.Gamepad.currentButtonState[i][k];

            // Get current gamepad state
            // NOTE: There is no callback available, so we get it manually
            GLFWgamepadstate state = { 0 };
            glfwGetGamepadState(i, &state); // This remapps all gamepads so they have their buttons mapped like an xbox controller

            const unsigned char *buttons = state.buttons;

            for (int k = 0; (buttons != NULL) && (k < GLFW_GAMEPAD_BUTTON_DPAD_LEFT + 1) && (k < RL_MAX_GAMEPAD_BUTTONS); k++)
            {
                int button = -1;        // rlGamepadButton enum values assigned

                switch (k)
                {
                    case GLFW_GAMEPAD_BUTTON_Y: button = RL_GAMEPAD_BUTTON_RIGHT_FACE_UP; break;
                    case GLFW_GAMEPAD_BUTTON_B: button = RL_GAMEPAD_BUTTON_RIGHT_FACE_RIGHT; break;
                    case GLFW_GAMEPAD_BUTTON_A: button = RL_GAMEPAD_BUTTON_RIGHT_FACE_DOWN; break;
                    case GLFW_GAMEPAD_BUTTON_X: button = RL_GAMEPAD_BUTTON_RIGHT_FACE_LEFT; break;

                    case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER: button = RL_GAMEPAD_BUTTON_LEFT_TRIGGER_1; break;
                    case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER: button = RL_GAMEPAD_BUTTON_RIGHT_TRIGGER_1; break;

                    case GLFW_GAMEPAD_BUTTON_BACK: button = RL_GAMEPAD_BUTTON_MIDDLE_LEFT; break;
                    case GLFW_GAMEPAD_BUTTON_GUIDE: button = RL_GAMEPAD_BUTTON_MIDDLE; break;
                    case GLFW_GAMEPAD_BUTTON_START: button = RL_GAMEPAD_BUTTON_MIDDLE_RIGHT; break;

                    case GLFW_GAMEPAD_BUTTON_DPAD_UP: button = RL_GAMEPAD_BUTTON_LEFT_FACE_UP; break;
                    case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT: button = RL_GAMEPAD_BUTTON_LEFT_FACE_RIGHT; break;
                    case GLFW_GAMEPAD_BUTTON_DPAD_DOWN: button = RL_GAMEPAD_BUTTON_LEFT_FACE_DOWN; break;
                    case GLFW_GAMEPAD_BUTTON_DPAD_LEFT: button = RL_GAMEPAD_BUTTON_LEFT_FACE_LEFT; break;

                    case GLFW_GAMEPAD_BUTTON_LEFT_THUMB: button = RL_GAMEPAD_BUTTON_LEFT_THUMB; break;
                    case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB: button = RL_GAMEPAD_BUTTON_RIGHT_THUMB; break;
                    default: break;
                }

                if (button != -1)   // Check for valid button
                {
                    if (buttons[k] == GLFW_PRESS)
                    {
                        rlCORE.Input.Gamepad.currentButtonState[i][button] = 1;
                        rlCORE.Input.Gamepad.lastButtonPressed = button;
                    }
                    else rlCORE.Input.Gamepad.currentButtonState[i][button] = 0;
                }
            }

            // Get current axis state
            const float *axes = state.axes;

            for (int k = 0; (axes != NULL) && (k < GLFW_GAMEPAD_AXIS_LAST + 1) && (k < RL_MAX_GAMEPAD_AXIS); k++)
            {
                rlCORE.Input.Gamepad.axisState[i][k] = axes[k];
            }

            // Register buttons for 2nd triggers (because GLFW doesn't count these as buttons but rather axis)
            rlCORE.Input.Gamepad.currentButtonState[i][RL_GAMEPAD_BUTTON_LEFT_TRIGGER_2] = (char)(rlCORE.Input.Gamepad.axisState[i][RL_GAMEPAD_AXIS_LEFT_TRIGGER] > 0.1f);
            rlCORE.Input.Gamepad.currentButtonState[i][RL_GAMEPAD_BUTTON_RIGHT_TRIGGER_2] = (char)(rlCORE.Input.Gamepad.axisState[i][RL_GAMEPAD_AXIS_RIGHT_TRIGGER] > 0.1f);

            rlCORE.Input.Gamepad.axisCount[i] = GLFW_GAMEPAD_AXIS_LAST + 1;
        }
    }

    rlCORE.Window.resizedLastFrame = false;

    if (rlCORE.Window.eventWaiting) glfwWaitEvents();     // Wait for in input events before continue (drawing is paused)
    else glfwPollEvents();      // Poll input events: keyboard/mouse/window events (callbacks) -> Update keys state

    // While window minimized, stop loop execution
    while (rlIsWindowState(RL_FLAG_WINDOW_MINIMIZED) && !rlIsWindowState(RL_FLAG_WINDOW_ALWAYS_RUN)) glfwWaitEvents();

    rlCORE.Window.shouldClose = glfwWindowShouldClose(platform.handle);

    // Reset close status for next frame
    glfwSetWindowShouldClose(platform.handle, GLFW_FALSE);
}


//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

// Initialize platform: graphics, inputs and more
int InitPlatform(void)
{
    glfwSetErrorCallback(ErrorCallback);
/*
    // TODO: Setup GLFW custom allocators to match raylib ones
    const GLFWallocator allocator = {
        .allocate = rlMemAlloc,
        .deallocate = rlMemFree,
        .reallocate = rlMemRealloc,
        .user = NULL
    };

    glfwInitAllocator(&allocator);
*/

#if defined(__APPLE__)
    glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif
    // Initialize GLFW internal global state
    int result = glfwInit();
    if (result == GLFW_FALSE) { rlTRACELOG(RL_LOG_WARNING, "GLFW: Failed to initialize GLFW"); return -1; }

    // Initialize graphic device: display/window and graphic context
    //----------------------------------------------------------------------------
    glfwDefaultWindowHints();                       // Set default windows hints
    //glfwWindowHint(GLFW_RED_BITS, 8);             // Framebuffer red color component bits
    //glfwWindowHint(GLFW_GREEN_BITS, 8);           // Framebuffer green color component bits
    //glfwWindowHint(GLFW_BLUE_BITS, 8);            // Framebuffer blue color component bits
    //glfwWindowHint(GLFW_ALPHA_BITS, 8);           // Framebuffer alpha color component bits
    //glfwWindowHint(GLFW_DEPTH_BITS, 24);          // Depthbuffer bits
    //glfwWindowHint(GLFW_REFRESH_RATE, 0);         // Refresh rate for fullscreen window
    //glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API); // OpenGL API to use. Alternative: GLFW_OPENGL_ES_API
    //glfwWindowHint(GLFW_AUX_BUFFERS, 0);          // Number of auxiliar buffers

    // Check window creation flags
    if ((rlCORE.Window.flags & RL_FLAG_FULLSCREEN_MODE) > 0) rlCORE.Window.fullscreen = true;

    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_HIDDEN) > 0) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Visible window
    else glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);     // Window initially hidden

    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_UNDECORATED) > 0) glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // Border and buttons on Window
    else glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);   // Decorated window

    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_RESIZABLE) > 0) glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // Resizable window
    else glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);  // Avoid window being resizable

    // Disable RL_FLAG_WINDOW_MINIMIZED, not supported on initialization
    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_MINIMIZED) > 0) rlCORE.Window.flags &= ~RL_FLAG_WINDOW_MINIMIZED;

    // Disable RL_FLAG_WINDOW_MAXIMIZED, not supported on initialization
    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_MAXIMIZED) > 0) rlCORE.Window.flags &= ~RL_FLAG_WINDOW_MAXIMIZED;

    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_UNFOCUSED) > 0) glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
    else glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_TOPMOST) > 0) glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    else glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);

    // NOTE: Some GLFW flags are not supported on HTML5
    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_TRANSPARENT) > 0) glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);     // Transparent framebuffer
    else glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);  // Opaque framebuffer

    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_HIGHDPI) > 0)
    {
        // Resize window content area based on the monitor content scale.
        // NOTE: This hint only has an effect on platforms where screen coordinates and pixels always map 1:1 such as Windows and X11.
        // On platforms like macOS the resolution of the framebuffer is changed independently of the window size.
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);   // Scale content area based on the monitor content scale where window is placed on
#if defined(__APPLE__)
        glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
#endif
    }
    else glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);

    // Mouse passthrough
    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0) glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
    else glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_FALSE);

    if (rlCORE.Window.flags & RL_FLAG_MSAA_4X_HINT)
    {
        // NOTE: MSAA is only enabled for main framebuffer, not user-created FBOs
        rlTRACELOG(RL_LOG_INFO, "DISPLAY: Trying to enable MSAA x4");
        glfwWindowHint(GLFW_SAMPLES, 4);   // Tries to enable multisampling x4 (MSAA), default is 0
    }

    // NOTE: When asking for an OpenGL context version, most drivers provide the highest supported version
    // with backward compatibility to older OpenGL versions.
    // For example, if using OpenGL 1.1, driver can provide a 4.3 backwards compatible context.

    // Check selection OpenGL version
    if (rlglGetVersion() == RL_OPENGL_21)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);          // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);          // Choose OpenGL minor version (just hint)
    }
    else if (rlglGetVersion() == RL_OPENGL_33)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);          // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);          // Choose OpenGL minor version (just hint)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Profiles Hint: Only 3.3 and above!
                                                                       // Values: GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_COMPAT_PROFILE
#if defined(__APPLE__)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);  // OSX Requires forward compatibility
#else
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE); // Forward Compatibility Hint: Only 3.3 and above!
#endif
        //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); // Request OpenGL DEBUG context
    }
    else if (rlglGetVersion() == RL_OPENGL_43)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);          // Choose OpenGL major version (just hint)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);          // Choose OpenGL minor version (just hint)
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_FALSE);
#if defined(RLGL_ENABLE_OPENGL_DEBUG_CONTEXT)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);   // Enable OpenGL Debug Context
#endif
    }
    else if (rlglGetVersion() == RL_OPENGL_ES_20)                 // Request OpenGL ES 2.0 context
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    }
    else if (rlglGetVersion() == RL_OPENGL_ES_30)                 // Request OpenGL ES 3.0 context
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    }

    // NOTE: GLFW 3.4+ defers initialization of the Joystick subsystem on the first call to any Joystick related functions.
    // Forcing this initialization here avoids doing it on rlPollInputEvents() called by rlEndDrawing() after first frame has been just drawn.
    // The initialization will still happen and possible delays still occur, but before the window is shown, which is a nicer experience.
    // REF: https://github.com/raysan5/raylib/issues/1554
    glfwSetJoystickCallback(NULL);

    // Find monitor resolution
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    if (!monitor)
    {
        rlTRACELOG(RL_LOG_WARNING, "GLFW: Failed to get primary monitor");
        return -1;
    }

    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    rlCORE.Window.display.width = mode->width;
    rlCORE.Window.display.height = mode->height;

    // Set screen width/height to the display width/height if they are 0
    if (rlCORE.Window.screen.width == 0) rlCORE.Window.screen.width = rlCORE.Window.display.width;
    if (rlCORE.Window.screen.height == 0) rlCORE.Window.screen.height = rlCORE.Window.display.height;

    if (rlCORE.Window.fullscreen)
    {
        // remember center for switchinging from fullscreen to window
        if ((rlCORE.Window.screen.height == rlCORE.Window.display.height) && (rlCORE.Window.screen.width == rlCORE.Window.display.width))
        {
            // If screen width/height equal to the display, we can't calculate the window pos for toggling full-screened/windowed.
            // Toggling full-screened/windowed with pos(0, 0) can cause problems in some platforms, such as X11.
            rlCORE.Window.position.x = rlCORE.Window.display.width/4;
            rlCORE.Window.position.y = rlCORE.Window.display.height/4;
        }
        else
        {
            rlCORE.Window.position.x = rlCORE.Window.display.width/2 - rlCORE.Window.screen.width/2;
            rlCORE.Window.position.y = rlCORE.Window.display.height/2 - rlCORE.Window.screen.height/2;
        }

        if (rlCORE.Window.position.x < 0) rlCORE.Window.position.x = 0;
        if (rlCORE.Window.position.y < 0) rlCORE.Window.position.y = 0;

        // Obtain recommended rlCORE.Window.display.width/rlCORE.Window.display.height from a valid videomode for the monitor
        int count = 0;
        const GLFWvidmode *modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);

        // Get closest video mode to desired rlCORE.Window.screen.width/rlCORE.Window.screen.height
        for (int i = 0; i < count; i++)
        {
            if ((unsigned int)modes[i].width >= rlCORE.Window.screen.width)
            {
                if ((unsigned int)modes[i].height >= rlCORE.Window.screen.height)
                {
                    rlCORE.Window.display.width = modes[i].width;
                    rlCORE.Window.display.height = modes[i].height;
                    break;
                }
            }
        }

        rlTRACELOG(RL_LOG_WARNING, "SYSTEM: Closest fullscreen videomode: %i x %i", rlCORE.Window.display.width, rlCORE.Window.display.height);

        // NOTE: ISSUE: Closest videomode could not match monitor aspect-ratio, for example,
        // for a desired screen size of 800x450 (16:9), closest supported videomode is 800x600 (4:3),
        // framebuffer is rendered correctly but once displayed on a 16:9 monitor, it gets stretched
        // by the sides to fit all monitor space...

        // Try to setup the most appropriate fullscreen framebuffer for the requested screenWidth/screenHeight
        // It considers device display resolution mode and setups a framebuffer with black bars if required (render size/offset)
        // Modified global variables: rlCORE.Window.screen.width/rlCORE.Window.screen.height - rlCORE.Window.render.width/rlCORE.Window.render.height - rlCORE.Window.renderOffset.x/rlCORE.Window.renderOffset.y - rlCORE.Window.screenScale
        // TODO: It is a quite cumbersome solution to display size vs requested size, it should be reviewed or removed...
        // HighDPI monitors are properly considered in a following similar function: SetupViewport()
        SetupFramebuffer(rlCORE.Window.display.width, rlCORE.Window.display.height);

        platform.handle = glfwCreateWindow(rlCORE.Window.display.width, rlCORE.Window.display.height, (rlCORE.Window.title != 0)? rlCORE.Window.title : " ", glfwGetPrimaryMonitor(), NULL);

        // NOTE: Full-screen change, not working properly...
        //glfwSetWindowMonitor(platform.handle, glfwGetPrimaryMonitor(), 0, 0, rlCORE.Window.screen.width, rlCORE.Window.screen.height, GLFW_DONT_CARE);
    }
    else
    {
        // If we are windowed fullscreen, ensures that window does not minimize when focus is lost
        if ((rlCORE.Window.screen.height == rlCORE.Window.display.height) && (rlCORE.Window.screen.width == rlCORE.Window.display.width))
        {
            glfwWindowHint(GLFW_AUTO_ICONIFY, 0);
        }

        // No-fullscreen window creation
        platform.handle = glfwCreateWindow(rlCORE.Window.screen.width, rlCORE.Window.screen.height, (rlCORE.Window.title != 0)? rlCORE.Window.title : " ", NULL, NULL);

        if (platform.handle)
        {
            rlCORE.Window.render.width = rlCORE.Window.screen.width;
            rlCORE.Window.render.height = rlCORE.Window.screen.height;
        }
    }

    if (!platform.handle)
    {
        glfwTerminate();
        rlTRACELOG(RL_LOG_WARNING, "GLFW: Failed to initialize Window");
        return -1;
    }

    glfwMakeContextCurrent(platform.handle);
    result = glfwGetError(NULL);

    // Check context activation
    if ((result != GLFW_NO_WINDOW_CONTEXT) && (result != GLFW_PLATFORM_ERROR))
    {
        rlCORE.Window.ready = true;

        glfwSwapInterval(0);        // No V-Sync by default

        // Try to enable GPU V-Sync, so frames are limited to screen refresh rate (60Hz -> 60 FPS)
        // NOTE: V-Sync can be enabled by graphic driver configuration, it doesn't need
        // to be activated on web platforms since VSync is enforced there.
        if (rlCORE.Window.flags & RL_FLAG_VSYNC_HINT)
        {
            // WARNING: It seems to hit a critical render path in Intel HD Graphics
            glfwSwapInterval(1);
            rlTRACELOG(RL_LOG_INFO, "DISPLAY: Trying to enable VSYNC");
        }

        int fbWidth = rlCORE.Window.screen.width;
        int fbHeight = rlCORE.Window.screen.height;

        if ((rlCORE.Window.flags & RL_FLAG_WINDOW_HIGHDPI) > 0)
        {
            // NOTE: On APPLE platforms system should manage window/input scaling and also framebuffer scaling.
            // Framebuffer scaling should be activated with: glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
    #if !defined(__APPLE__)
            glfwGetFramebufferSize(platform.handle, &fbWidth, &fbHeight);

            // Screen scaling matrix is required in case desired screen area is different from display area
            rlCORE.Window.screenScale = rlMatrixScale((float)fbWidth/rlCORE.Window.screen.width, (float)fbHeight/rlCORE.Window.screen.height, 1.0f);

            // Mouse input scaling for the new screen size
            rlSetMouseScale((float)rlCORE.Window.screen.width/fbWidth, (float)rlCORE.Window.screen.height/fbHeight);
    #endif
        }

        rlCORE.Window.render.width = fbWidth;
        rlCORE.Window.render.height = fbHeight;
        rlCORE.Window.currentFbo.width = fbWidth;
        rlCORE.Window.currentFbo.height = fbHeight;

        rlTRACELOG(RL_LOG_INFO, "DISPLAY: Device initialized successfully");
        rlTRACELOG(RL_LOG_INFO, "    > Display size: %i x %i", rlCORE.Window.display.width, rlCORE.Window.display.height);
        rlTRACELOG(RL_LOG_INFO, "    > Screen size:  %i x %i", rlCORE.Window.screen.width, rlCORE.Window.screen.height);
        rlTRACELOG(RL_LOG_INFO, "    > Render size:  %i x %i", rlCORE.Window.render.width, rlCORE.Window.render.height);
        rlTRACELOG(RL_LOG_INFO, "    > Viewport offsets: %i, %i", rlCORE.Window.renderOffset.x, rlCORE.Window.renderOffset.y);
    }
    else
    {
        rlTRACELOG(RL_LOG_FATAL, "PLATFORM: Failed to initialize graphics device");
        return -1;
    }

    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_MINIMIZED) > 0) rlMinimizeWindow();

    // If graphic device is no properly initialized, we end program
    if (!rlCORE.Window.ready) { rlTRACELOG(RL_LOG_FATAL, "PLATFORM: Failed to initialize graphic device"); return -1; }
    else rlSetWindowPosition(rlGetMonitorWidth(rlGetCurrentMonitor())/2 - rlCORE.Window.screen.width/2, rlGetMonitorHeight(rlGetCurrentMonitor())/2 - rlCORE.Window.screen.height/2);

    // Load OpenGL extensions
    // NOTE: GL procedures address loader is required to load extensions
    rlglLoadExtensions(glfwGetProcAddress);
    //----------------------------------------------------------------------------

    // Initialize input events callbacks
    //----------------------------------------------------------------------------
    // Set window callback events
    glfwSetWindowSizeCallback(platform.handle, WindowSizeCallback);      // NOTE: Resizing not allowed by default!
    glfwSetWindowMaximizeCallback(platform.handle, WindowMaximizeCallback);
    glfwSetWindowIconifyCallback(platform.handle, WindowIconifyCallback);
    glfwSetWindowFocusCallback(platform.handle, WindowFocusCallback);
    glfwSetDropCallback(platform.handle, WindowDropCallback);

    // Set input callback events
    glfwSetKeyCallback(platform.handle, KeyCallback);
    glfwSetCharCallback(platform.handle, CharCallback);
    glfwSetMouseButtonCallback(platform.handle, MouseButtonCallback);
    glfwSetCursorPosCallback(platform.handle, MouseCursorPosCallback);   // Track mouse position changes
    glfwSetScrollCallback(platform.handle, MouseScrollCallback);
    glfwSetCursorEnterCallback(platform.handle, CursorEnterCallback);
    glfwSetJoystickCallback(JoystickCallback);

    glfwSetInputMode(platform.handle, GLFW_LOCK_KEY_MODS, GLFW_TRUE);    // Enable lock keys modifiers (CAPS, NUM)

    // Retrieve gamepad names
    for (int i = 0; i < RL_MAX_GAMEPADS; i++)
    {
        if (glfwJoystickPresent(i)) strcpy(rlCORE.Input.Gamepad.name[i], glfwGetJoystickName(i));
    }
    //----------------------------------------------------------------------------

    // Initialize timming system
    //----------------------------------------------------------------------------
    InitTimer();
    //----------------------------------------------------------------------------

    // Initialize storage system
    //----------------------------------------------------------------------------
    rlCORE.Storage.basePath = rlGetWorkingDirectory();
    //----------------------------------------------------------------------------

    rlTRACELOG(RL_LOG_INFO, "PLATFORM: DESKTOP (GLFW): Initialized successfully");

    return 0;
}

// Close platform
void ClosePlatform(void)
{
    glfwDestroyWindow(platform.handle);
    glfwTerminate();

#if defined(_WIN32) && defined(RL_SUPPORT_WINMM_HIGHRES_TIMER) && !defined(RL_SUPPORT_BUSY_WAIT_LOOP)
    timeEndPeriod(1);           // Restore time period
#endif
}

// GLFW3 Error Callback, runs on GLFW3 error
static void ErrorCallback(int error, const char *description)
{
    rlTRACELOG(RL_LOG_WARNING, "GLFW: Error: %i Description: %s", error, description);
}

// GLFW3 WindowSize Callback, runs when window is resizedLastFrame
// NOTE: Window resizing not allowed by default
static void WindowSizeCallback(GLFWwindow *window, int width, int height)
{
    // Reset viewport and projection matrix for new size
    SetupViewport(width, height);

    rlCORE.Window.currentFbo.width = width;
    rlCORE.Window.currentFbo.height = height;
    rlCORE.Window.resizedLastFrame = true;

    if (rlIsWindowFullscreen()) return;

    // Set current screen size
#if defined(__APPLE__)
    rlCORE.Window.screen.width = width;
    rlCORE.Window.screen.height = height;
#else
    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_HIGHDPI) > 0)
    {
        rlVector2 windowScaleDPI = rlGetWindowScaleDPI();

        rlCORE.Window.screen.width = (unsigned int)(width/windowScaleDPI.x);
        rlCORE.Window.screen.height = (unsigned int)(height/windowScaleDPI.y);
    }
    else
    {
        rlCORE.Window.screen.width = width;
        rlCORE.Window.screen.height = height;
    }
#endif

    // NOTE: Postprocessing texture is not scaled to new size
}

// GLFW3 WindowIconify Callback, runs when window is minimized/restored
static void WindowIconifyCallback(GLFWwindow *window, int iconified)
{
    if (iconified) rlCORE.Window.flags |= RL_FLAG_WINDOW_MINIMIZED;  // The window was iconified
    else rlCORE.Window.flags &= ~RL_FLAG_WINDOW_MINIMIZED;           // The window was restored
}

// GLFW3 WindowMaximize Callback, runs when window is maximized/restored
static void WindowMaximizeCallback(GLFWwindow *window, int maximized)
{
    if (maximized) rlCORE.Window.flags |= RL_FLAG_WINDOW_MAXIMIZED;  // The window was maximized
    else rlCORE.Window.flags &= ~RL_FLAG_WINDOW_MAXIMIZED;           // The window was restored
}

// GLFW3 WindowFocus Callback, runs when window get/lose focus
static void WindowFocusCallback(GLFWwindow *window, int focused)
{
    if (focused) rlCORE.Window.flags &= ~RL_FLAG_WINDOW_UNFOCUSED;   // The window was focused
    else rlCORE.Window.flags |= RL_FLAG_WINDOW_UNFOCUSED;            // The window lost focus
}

// GLFW3 Window Drop Callback, runs when drop files into window
static void WindowDropCallback(GLFWwindow *window, int count, const char **paths)
{
    if (count > 0)
    {
        // In case previous dropped filepaths have not been freed, we free them
        if (rlCORE.Window.dropFileCount > 0)
        {
            for (unsigned int i = 0; i < rlCORE.Window.dropFileCount; i++) RL_FREE(rlCORE.Window.dropFilepaths[i]);

            RL_FREE(rlCORE.Window.dropFilepaths);

            rlCORE.Window.dropFileCount = 0;
            rlCORE.Window.dropFilepaths = NULL;
        }

        // WARNING: Paths are freed by GLFW when the callback returns, we must keep an internal copy
        rlCORE.Window.dropFileCount = count;
        rlCORE.Window.dropFilepaths = (char **)RL_CALLOC(rlCORE.Window.dropFileCount, sizeof(char *));

        for (unsigned int i = 0; i < rlCORE.Window.dropFileCount; i++)
        {
            rlCORE.Window.dropFilepaths[i] = (char *)RL_CALLOC(RL_MAX_FILEPATH_LENGTH, sizeof(char));
            strcpy(rlCORE.Window.dropFilepaths[i], paths[i]);
        }
    }
}

// GLFW3 Keyboard Callback, runs on key pressed
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key < 0) return;    // Security check, macOS fn key generates -1

    // WARNING: GLFW could return GLFW_REPEAT, we need to consider it as 1
    // to work properly with our implementation (rlIsKeyDown/rlIsKeyUp checks)
    if (action == GLFW_RELEASE) rlCORE.Input.Keyboard.currentKeyState[key] = 0;
    else if(action == GLFW_PRESS) rlCORE.Input.Keyboard.currentKeyState[key] = 1;
    else if(action == GLFW_REPEAT) rlCORE.Input.Keyboard.keyRepeatInFrame[key] = 1;

    // WARNING: Check if CAPS/NUM key modifiers are enabled and force down state for those keys
    if (((key == RL_KEY_CAPS_LOCK) && ((mods & GLFW_MOD_CAPS_LOCK) > 0)) ||
        ((key == RL_KEY_NUM_LOCK) && ((mods & GLFW_MOD_NUM_LOCK) > 0))) rlCORE.Input.Keyboard.currentKeyState[key] = 1;

    // Check if there is space available in the key queue
    if ((rlCORE.Input.Keyboard.keyPressedQueueCount < RL_MAX_KEY_PRESSED_QUEUE) && (action == GLFW_PRESS))
    {
        // Add character to the queue
        rlCORE.Input.Keyboard.keyPressedQueue[rlCORE.Input.Keyboard.keyPressedQueueCount] = key;
        rlCORE.Input.Keyboard.keyPressedQueueCount++;
    }

    // Check the exit key to set close window
    if ((key == rlCORE.Input.Keyboard.exitKey) && (action == GLFW_PRESS)) glfwSetWindowShouldClose(platform.handle, GLFW_TRUE);
}

// GLFW3 Char Key Callback, runs on key down (gets equivalent unicode char value)
static void CharCallback(GLFWwindow *window, unsigned int key)
{
    //rlTRACELOG(RL_LOG_DEBUG, "Char Callback: KEY:%i(%c)", key, key);

    // NOTE: Registers any key down considering OS keyboard layout but
    // does not detect action events, those should be managed by user...
    // Ref: https://github.com/glfw/glfw/issues/668#issuecomment-166794907
    // Ref: https://www.glfw.org/docs/latest/input_guide.html#input_char

    // Check if there is space available in the queue
    if (rlCORE.Input.Keyboard.charPressedQueueCount < RL_MAX_CHAR_PRESSED_QUEUE)
    {
        // Add character to the queue
        rlCORE.Input.Keyboard.charPressedQueue[rlCORE.Input.Keyboard.charPressedQueueCount] = key;
        rlCORE.Input.Keyboard.charPressedQueueCount++;
    }
}

// GLFW3 Mouse Button Callback, runs on mouse button pressed
static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    // WARNING: GLFW could only return GLFW_PRESS (1) or GLFW_RELEASE (0) for now,
    // but future releases may add more actions (i.e. GLFW_REPEAT)
    rlCORE.Input.Mouse.currentButtonState[button] = action;

#if defined(RL_SUPPORT_GESTURES_SYSTEM) && defined(RL_SUPPORT_MOUSE_GESTURES)
    // Process mouse events as touches to be able to use mouse-gestures
    rlGestureEvent gestureEvent = { 0 };

    // Register touch actions
    if ((rlCORE.Input.Mouse.currentButtonState[button] == 1) && (rlCORE.Input.Mouse.previousButtonState[button] == 0)) gestureEvent.touchAction = TOUCH_ACTION_DOWN;
    else if ((rlCORE.Input.Mouse.currentButtonState[button] == 0) && (rlCORE.Input.Mouse.previousButtonState[button] == 1)) gestureEvent.touchAction = TOUCH_ACTION_UP;

    // NOTE: TOUCH_ACTION_MOVE event is registered in MouseCursorPosCallback()

    // Assign a pointer ID
    gestureEvent.pointId[0] = 0;

    // Register touch points count
    gestureEvent.pointCount = 1;

    // Register touch points position, only one point registered
    gestureEvent.position[0] = rlGetMousePosition();

    // rlNormalize gestureEvent.position[0] for rlCORE.Window.screen.width and rlCORE.Window.screen.height
    gestureEvent.position[0].x /= (float)rlGetScreenWidth();
    gestureEvent.position[0].y /= (float)rlGetScreenHeight();

    // rlGesture data is sent to gestures-system for processing
    rlProcessGestureEvent(gestureEvent);
#endif
}

// GLFW3 Cursor Position Callback, runs on mouse move
static void MouseCursorPosCallback(GLFWwindow *window, double x, double y)
{
    rlCORE.Input.Mouse.currentPosition.x = (float)x;
    rlCORE.Input.Mouse.currentPosition.y = (float)y;
    rlCORE.Input.Touch.position[0] = rlCORE.Input.Mouse.currentPosition;

#if defined(RL_SUPPORT_GESTURES_SYSTEM) && defined(RL_SUPPORT_MOUSE_GESTURES)
    // Process mouse events as touches to be able to use mouse-gestures
    rlGestureEvent gestureEvent = { 0 };

    gestureEvent.touchAction = TOUCH_ACTION_MOVE;

    // Assign a pointer ID
    gestureEvent.pointId[0] = 0;

    // Register touch points count
    gestureEvent.pointCount = 1;

    // Register touch points position, only one point registered
    gestureEvent.position[0] = rlCORE.Input.Touch.position[0];

    // rlNormalize gestureEvent.position[0] for rlCORE.Window.screen.width and rlCORE.Window.screen.height
    gestureEvent.position[0].x /= (float)rlGetScreenWidth();
    gestureEvent.position[0].y /= (float)rlGetScreenHeight();

    // rlGesture data is sent to gestures-system for processing
    rlProcessGestureEvent(gestureEvent);
#endif
}

// GLFW3 Scrolling Callback, runs on mouse wheel
static void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
#if __cplusplus
    rlCORE.Input.Mouse.currentWheelMove = rlVector2{ (float)xoffset, (float)yoffset };
#else
    rlCORE.Input.Mouse.currentWheelMove = (rlVector2){ (float)xoffset, (float)yoffset };
#endif
}

// GLFW3 CursorEnter Callback, when cursor enters the window
static void CursorEnterCallback(GLFWwindow *window, int enter)
{
    if (enter) rlCORE.Input.Mouse.cursorOnScreen = true;
    else rlCORE.Input.Mouse.cursorOnScreen = false;
}

// GLFW3 Joystick Connected/Disconnected Callback
static void JoystickCallback(int jid, int event)
{
    if (event == GLFW_CONNECTED)
    {
        strcpy(rlCORE.Input.Gamepad.name[jid], glfwGetJoystickName(jid));
    }
    else if (event == GLFW_DISCONNECTED)
    {
        memset(rlCORE.Input.Gamepad.name[jid], 0, 64);
    }
}

RL_NS_END

// EOF
