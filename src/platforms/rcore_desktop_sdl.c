/**********************************************************************************************
*
*   rcore_desktop_sdl - Functions to manage window, graphics device and inputs
*
*   PLATFORM: DESKTOP: SDL
*       - Windows (Win32, Win64)
*       - Linux (X11/Wayland desktop mode)
*       - Others (not tested)
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
*       - SDL 2 (main library): Windowing and inputs management
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

#include "SDL.h"            // SDL base library (window/rendered, input, timming... functionality)
#include "SDL_opengl.h"     // SDL OpenGL functionality (if required, instead of internal renderer)

#ifndef RL_NS_BEGIN
#define RL_NS_BEGIN
#define RL_NS_END
#endif

#define RL_NS_BEGIN

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    SDL_Window *window;
    SDL_GLContext glContext;

    SDL_Joystick *gamepad;
    SDL_Cursor *cursor;
    bool cursorRelative;
} PlatformData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
extern rlCoreData rlCORE;                   // Global rlCORE state context

static PlatformData platform = { 0 };   // Platform specific data

//----------------------------------------------------------------------------------
// Local Variables Definition
//----------------------------------------------------------------------------------
#define SCANCODE_MAPPED_NUM 232
static const rlKeyboardKey ScancodeToKey[SCANCODE_MAPPED_NUM] = {
    RL_KEY_NULL,           // SDL_SCANCODE_UNKNOWN
    0,
    0,
    0,
    RL_KEY_A,              // SDL_SCANCODE_A
    RL_KEY_B,              // SDL_SCANCODE_B
    RL_KEY_C,              // SDL_SCANCODE_C
    RL_KEY_D,              // SDL_SCANCODE_D
    RL_KEY_E,              // SDL_SCANCODE_E
    RL_KEY_F,              // SDL_SCANCODE_F
    RL_KEY_G,              // SDL_SCANCODE_G
    RL_KEY_H,              // SDL_SCANCODE_H
    RL_KEY_I,              // SDL_SCANCODE_I
    RL_KEY_J,              // SDL_SCANCODE_J
    RL_KEY_K,              // SDL_SCANCODE_K
    RL_KEY_L,              // SDL_SCANCODE_L
    RL_KEY_M,              // SDL_SCANCODE_M
    RL_KEY_N,              // SDL_SCANCODE_N
    RL_KEY_O,              // SDL_SCANCODE_O
    RL_KEY_P,              // SDL_SCANCODE_P
    RL_KEY_Q,              // SDL_SCANCODE_Q
    RL_KEY_R,              // SDL_SCANCODE_R
    RL_KEY_S,              // SDL_SCANCODE_S
    RL_KEY_T,              // SDL_SCANCODE_T
    RL_KEY_U,              // SDL_SCANCODE_U
    RL_KEY_V,              // SDL_SCANCODE_V
    RL_KEY_W,              // SDL_SCANCODE_W
    RL_KEY_X,              // SDL_SCANCODE_X
    RL_KEY_Y,              // SDL_SCANCODE_Y
    RL_KEY_Z,              // SDL_SCANCODE_Z
    RL_KEY_ONE,            // SDL_SCANCODE_1
    RL_KEY_TWO,            // SDL_SCANCODE_2
    RL_KEY_THREE,          // SDL_SCANCODE_3
    RL_KEY_FOUR,           // SDL_SCANCODE_4
    RL_KEY_FIVE,           // SDL_SCANCODE_5
    RL_KEY_SIX,            // SDL_SCANCODE_6
    RL_KEY_SEVEN,          // SDL_SCANCODE_7
    RL_KEY_EIGHT,          // SDL_SCANCODE_8
    RL_KEY_NINE,           // SDL_SCANCODE_9
    RL_KEY_ZERO,           // SDL_SCANCODE_0
    RL_KEY_ENTER,          // SDL_SCANCODE_RETURN
    RL_KEY_ESCAPE,         // SDL_SCANCODE_ESCAPE
    RL_KEY_BACKSPACE,      // SDL_SCANCODE_BACKSPACE
    RL_KEY_TAB,            // SDL_SCANCODE_TAB
    RL_KEY_SPACE,          // SDL_SCANCODE_SPACE
    RL_KEY_MINUS,          // SDL_SCANCODE_MINUS
    RL_KEY_EQUAL,          // SDL_SCANCODE_EQUALS
    RL_KEY_LEFT_BRACKET,   // SDL_SCANCODE_LEFTBRACKET
    RL_KEY_RIGHT_BRACKET,  // SDL_SCANCODE_RIGHTBRACKET
    RL_KEY_BACKSLASH,      // SDL_SCANCODE_BACKSLASH
    0,                  // SDL_SCANCODE_NONUSHASH
    RL_KEY_SEMICOLON,      // SDL_SCANCODE_SEMICOLON
    RL_KEY_APOSTROPHE,     // SDL_SCANCODE_APOSTROPHE
    RL_KEY_GRAVE,          // SDL_SCANCODE_GRAVE
    RL_KEY_COMMA,          // SDL_SCANCODE_COMMA
    RL_KEY_PERIOD,         // SDL_SCANCODE_PERIOD
    RL_KEY_SLASH,          // SDL_SCANCODE_SLASH
    RL_KEY_CAPS_LOCK,      // SDL_SCANCODE_CAPSLOCK
    RL_KEY_F1,             // SDL_SCANCODE_F1
    RL_KEY_F2,             // SDL_SCANCODE_F2
    RL_KEY_F3,             // SDL_SCANCODE_F3
    RL_KEY_F4,             // SDL_SCANCODE_F4
    RL_KEY_F5,             // SDL_SCANCODE_F5
    RL_KEY_F6,             // SDL_SCANCODE_F6
    RL_KEY_F7,             // SDL_SCANCODE_F7
    RL_KEY_F8,             // SDL_SCANCODE_F8
    RL_KEY_F9,             // SDL_SCANCODE_F9
    RL_KEY_F10,            // SDL_SCANCODE_F10
    RL_KEY_F11,            // SDL_SCANCODE_F11
    RL_KEY_F12,            // SDL_SCANCODE_F12
    RL_KEY_PRINT_SCREEN,   // SDL_SCANCODE_PRINTSCREEN
    RL_KEY_SCROLL_LOCK,    // SDL_SCANCODE_SCROLLLOCK
    RL_KEY_PAUSE,          // SDL_SCANCODE_PAUSE
    RL_KEY_INSERT,         // SDL_SCANCODE_INSERT
    RL_KEY_HOME,           // SDL_SCANCODE_HOME
    RL_KEY_PAGE_UP,        // SDL_SCANCODE_PAGEUP
    RL_KEY_DELETE,         // SDL_SCANCODE_DELETE
    RL_KEY_END,            // SDL_SCANCODE_END
    RL_KEY_PAGE_DOWN,      // SDL_SCANCODE_PAGEDOWN
    RL_KEY_RIGHT,          // SDL_SCANCODE_RIGHT
    RL_KEY_LEFT,           // SDL_SCANCODE_LEFT
    RL_KEY_DOWN,           // SDL_SCANCODE_DOWN
    RL_KEY_UP,             // SDL_SCANCODE_UP
    RL_KEY_NUM_LOCK,       // SDL_SCANCODE_NUMLOCKCLEAR
    RL_KEY_KP_DIVIDE,      // SDL_SCANCODE_KP_DIVIDE
    RL_KEY_KP_MULTIPLY,    // SDL_SCANCODE_KP_MULTIPLY
    RL_KEY_KP_SUBTRACT,    // SDL_SCANCODE_KP_MINUS
    RL_KEY_KP_ADD,         // SDL_SCANCODE_KP_PLUS
    RL_KEY_KP_ENTER,       // SDL_SCANCODE_KP_ENTER
    RL_KEY_KP_1,           // SDL_SCANCODE_KP_1
    RL_KEY_KP_2,           // SDL_SCANCODE_KP_2
    RL_KEY_KP_3,           // SDL_SCANCODE_KP_3
    RL_KEY_KP_4,           // SDL_SCANCODE_KP_4
    RL_KEY_KP_5,           // SDL_SCANCODE_KP_5
    RL_KEY_KP_6,           // SDL_SCANCODE_KP_6
    RL_KEY_KP_7,           // SDL_SCANCODE_KP_7
    RL_KEY_KP_8,           // SDL_SCANCODE_KP_8
    RL_KEY_KP_9,           // SDL_SCANCODE_KP_9
    RL_KEY_KP_0,           // SDL_SCANCODE_KP_0
    RL_KEY_KP_DECIMAL,     // SDL_SCANCODE_KP_PERIOD
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0,
    RL_KEY_LEFT_CONTROL,   //SDL_SCANCODE_LCTRL
    RL_KEY_LEFT_SHIFT,     //SDL_SCANCODE_LSHIFT
    RL_KEY_LEFT_ALT,       //SDL_SCANCODE_LALT
    RL_KEY_LEFT_SUPER,     //SDL_SCANCODE_LGUI
    RL_KEY_RIGHT_CONTROL,  //SDL_SCANCODE_RCTRL
    RL_KEY_RIGHT_SHIFT,    //SDL_SCANCODE_RSHIFT
    RL_KEY_RIGHT_ALT,      //SDL_SCANCODE_RALT
    RL_KEY_RIGHT_SUPER     //SDL_SCANCODE_RGUI
};

static const int CursorsLUT[] = {
    SDL_SYSTEM_CURSOR_ARROW,       // 0  RL_MOUSE_CURSOR_DEFAULT
    SDL_SYSTEM_CURSOR_ARROW,       // 1  RL_MOUSE_CURSOR_ARROW
    SDL_SYSTEM_CURSOR_IBEAM,       // 2  RL_MOUSE_CURSOR_IBEAM
    SDL_SYSTEM_CURSOR_CROSSHAIR,   // 3  RL_MOUSE_CURSOR_CROSSHAIR
    SDL_SYSTEM_CURSOR_HAND,        // 4  RL_MOUSE_CURSOR_POINTING_HAND
    SDL_SYSTEM_CURSOR_SIZEWE,      // 5  RL_MOUSE_CURSOR_RESIZE_EW
    SDL_SYSTEM_CURSOR_SIZENS,      // 6  RL_MOUSE_CURSOR_RESIZE_NS
    SDL_SYSTEM_CURSOR_SIZENWSE,    // 7  RL_MOUSE_CURSOR_RESIZE_NWSE
    SDL_SYSTEM_CURSOR_SIZENESW,    // 8  RL_MOUSE_CURSOR_RESIZE_NESW
    SDL_SYSTEM_CURSOR_SIZEALL,     // 9  RL_MOUSE_CURSOR_RESIZE_ALL
    SDL_SYSTEM_CURSOR_NO           // 10 RL_MOUSE_CURSOR_NOT_ALLOWED
    //SDL_SYSTEM_CURSOR_WAIT,      // No equivalent implemented on rlMouseCursor enum on raylib.h
    //SDL_SYSTEM_CURSOR_WAITARROW, // No equivalent implemented on rlMouseCursor enum on raylib.h
};

//----------------------------------------------------------------------------------
// Module Internal Functions Declaration
//----------------------------------------------------------------------------------
int InitPlatform(void);                                      // Initialize platform (graphics, inputs and more)
void ClosePlatform(void);                                    // Close platform

static rlKeyboardKey ConvertScancodeToKey(SDL_Scancode sdlScancode);  // Help convert SDL scancodes to raylib key

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
// NOTE: Functions declaration is provided by raylib.h

//----------------------------------------------------------------------------------
// Module Functions Definition: Window and Graphics Device
//----------------------------------------------------------------------------------

// Check if application should close
bool rlWindowShouldClose(void)
{
    if (rlCORE.Window.ready) return rlCORE.Window.shouldClose;
    else return true;
}

// Toggle fullscreen mode
void rlToggleFullscreen(void)
{
    const int monitor = SDL_GetWindowDisplayIndex(platform.window);
    const int monitorCount = SDL_GetNumVideoDisplays();
    if ((monitor >= 0) && (monitor < monitorCount))
    {
        if ((rlCORE.Window.flags & RL_FLAG_FULLSCREEN_MODE) > 0)
        {
            SDL_SetWindowFullscreen(platform.window, 0);
            rlCORE.Window.flags &= ~RL_FLAG_FULLSCREEN_MODE;
            rlCORE.Window.fullscreen = false;
        }
        else
        {
            SDL_SetWindowFullscreen(platform.window, SDL_WINDOW_FULLSCREEN);
            rlCORE.Window.flags |= RL_FLAG_FULLSCREEN_MODE;
            rlCORE.Window.fullscreen = true;
        }
    }
    else rlTRACELOG(RL_LOG_WARNING, "SDL: Failed to find selected monitor");
}

// Toggle borderless windowed mode
void rlToggleBorderlessWindowed(void)
{
    const int monitor = SDL_GetWindowDisplayIndex(platform.window);
    const int monitorCount = SDL_GetNumVideoDisplays();
    if ((monitor >= 0) && (monitor < monitorCount))
    {
        if ((rlCORE.Window.flags & RL_FLAG_BORDERLESS_WINDOWED_MODE) > 0)
        {
            SDL_SetWindowFullscreen(platform.window, 0);
            rlCORE.Window.flags &= ~RL_FLAG_BORDERLESS_WINDOWED_MODE;
        }
        else
        {
            SDL_SetWindowFullscreen(platform.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
            rlCORE.Window.flags |= RL_FLAG_BORDERLESS_WINDOWED_MODE;
        }
    }
    else rlTRACELOG(RL_LOG_WARNING, "SDL: Failed to find selected monitor");
}

// Set window state: maximized, if resizable
void rlMaximizeWindow(void)
{
    SDL_MaximizeWindow(platform.window);
    rlCORE.Window.flags |= RL_FLAG_WINDOW_MAXIMIZED;
}

// Set window state: minimized
void rlMinimizeWindow(void)
{
    SDL_MinimizeWindow(platform.window);
    rlCORE.Window.flags |= RL_FLAG_WINDOW_MINIMIZED;
}

// Set window state: not minimized/maximized
void rlRestoreWindow(void)
{
    SDL_ShowWindow(platform.window);
}

// Set window configuration state using flags
void rlSetWindowState(unsigned int flags)
{
    rlCORE.Window.flags |= flags;

    if (flags & RL_FLAG_VSYNC_HINT)
    {
        SDL_GL_SetSwapInterval(1);
    }
    if (flags & RL_FLAG_FULLSCREEN_MODE)
    {
        const int monitor = SDL_GetWindowDisplayIndex(platform.window);
        const int monitorCount = SDL_GetNumVideoDisplays();
        if ((monitor >= 0) && (monitor < monitorCount))
        {
            SDL_SetWindowFullscreen(platform.window, SDL_WINDOW_FULLSCREEN);
            rlCORE.Window.fullscreen = true;
        }
        else rlTRACELOG(RL_LOG_WARNING, "SDL: Failed to find selected monitor");
    }
    if (flags & RL_FLAG_WINDOW_RESIZABLE)
    {
        SDL_SetWindowResizable(platform.window, SDL_TRUE);
    }
    if (flags & RL_FLAG_WINDOW_UNDECORATED)
    {
        SDL_SetWindowBordered(platform.window, SDL_FALSE);
    }
    if (flags & RL_FLAG_WINDOW_HIDDEN)
    {
        SDL_HideWindow(platform.window);
    }
    if (flags & RL_FLAG_WINDOW_MINIMIZED)
    {
        SDL_MinimizeWindow(platform.window);
    }
    if (flags & RL_FLAG_WINDOW_MAXIMIZED)
    {
        SDL_MaximizeWindow(platform.window);
    }
    if (flags & RL_FLAG_WINDOW_UNFOCUSED)
    {
        // NOTE: To be able to implement this part it seems that we should
        // do it ourselves, via `Windows.h`, `X11/Xlib.h` or even `Cocoa.h`
        rlTRACELOG(RL_LOG_WARNING, "rlSetWindowState() - RL_FLAG_WINDOW_UNFOCUSED is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & RL_FLAG_WINDOW_TOPMOST)
    {
        SDL_SetWindowAlwaysOnTop(platform.window, SDL_FALSE);
    }
    if (flags & RL_FLAG_WINDOW_ALWAYS_RUN)
    {
        rlTRACELOG(RL_LOG_WARNING, "rlSetWindowState() - RL_FLAG_WINDOW_ALWAYS_RUN is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & RL_FLAG_WINDOW_TRANSPARENT)
    {
        rlTRACELOG(RL_LOG_WARNING, "rlSetWindowState() - RL_FLAG_WINDOW_TRANSPARENT is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & RL_FLAG_WINDOW_HIGHDPI)
    {
        // NOTE: Such a function does not seem to exist
        rlTRACELOG(RL_LOG_WARNING, "rlSetWindowState() - RL_FLAG_WINDOW_HIGHDPI is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & RL_FLAG_WINDOW_MOUSE_PASSTHROUGH)
    {
        //SDL_SetWindowGrab(platform.window, SDL_FALSE);
        rlTRACELOG(RL_LOG_WARNING, "rlSetWindowState() - RL_FLAG_WINDOW_MOUSE_PASSTHROUGH is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & RL_FLAG_BORDERLESS_WINDOWED_MODE)
    {
        const int monitor = SDL_GetWindowDisplayIndex(platform.window);
        const int monitorCount = SDL_GetNumVideoDisplays();
        if ((monitor >= 0) && (monitor < monitorCount))
        {
            SDL_SetWindowFullscreen(platform.window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
        else rlTRACELOG(RL_LOG_WARNING, "SDL: Failed to find selected monitor");
    }
    if (flags & RL_FLAG_MSAA_4X_HINT)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1); // Enable multisampling buffers
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // Enable multisampling
    }
    if (flags & RL_FLAG_INTERLACED_HINT)
    {
        rlTRACELOG(RL_LOG_WARNING, "rlSetWindowState() - RL_FLAG_INTERLACED_HINT is not supported on PLATFORM_DESKTOP_SDL");
    }
}

// Clear window configuration state flags
void rlClearWindowState(unsigned int flags)
{
    rlCORE.Window.flags &= ~flags;

    if (flags & RL_FLAG_VSYNC_HINT)
    {
        SDL_GL_SetSwapInterval(0);
    }
    if (flags & RL_FLAG_FULLSCREEN_MODE)
    {
        SDL_SetWindowFullscreen(platform.window, 0);
        rlCORE.Window.fullscreen = false;
    }
    if (flags & RL_FLAG_WINDOW_RESIZABLE)
    {
        SDL_SetWindowResizable(platform.window, SDL_FALSE);
    }
    if (flags & RL_FLAG_WINDOW_UNDECORATED)
    {
        SDL_SetWindowBordered(platform.window, SDL_TRUE);
    }
    if (flags & RL_FLAG_WINDOW_HIDDEN)
    {
        SDL_ShowWindow(platform.window);
    }
    if (flags & RL_FLAG_WINDOW_MINIMIZED)
    {
        SDL_RestoreWindow(platform.window);
    }
    if (flags & RL_FLAG_WINDOW_MAXIMIZED)
    {
        SDL_RestoreWindow(platform.window);
    }
    if (flags & RL_FLAG_WINDOW_UNFOCUSED)
    {
        //SDL_RaiseWindow(platform.window);
        rlTRACELOG(RL_LOG_WARNING, "rlClearWindowState() - RL_FLAG_WINDOW_UNFOCUSED is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & RL_FLAG_WINDOW_TOPMOST)
    {
        SDL_SetWindowAlwaysOnTop(platform.window, SDL_FALSE);
    }
    if (flags & RL_FLAG_WINDOW_ALWAYS_RUN)
    {
        rlTRACELOG(RL_LOG_WARNING, "rlClearWindowState() - RL_FLAG_WINDOW_ALWAYS_RUN is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & RL_FLAG_WINDOW_TRANSPARENT)
    {
        rlTRACELOG(RL_LOG_WARNING, "rlClearWindowState() - RL_FLAG_WINDOW_TRANSPARENT is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & RL_FLAG_WINDOW_HIGHDPI)
    {
        // NOTE: There also doesn't seem to be a feature to disable high DPI once enabled
        rlTRACELOG(RL_LOG_WARNING, "rlClearWindowState() - RL_FLAG_WINDOW_HIGHDPI is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & RL_FLAG_WINDOW_MOUSE_PASSTHROUGH)
    {
        //SDL_SetWindowGrab(platform.window, SDL_TRUE);
        rlTRACELOG(RL_LOG_WARNING, "rlClearWindowState() - RL_FLAG_WINDOW_MOUSE_PASSTHROUGH is not supported on PLATFORM_DESKTOP_SDL");
    }
    if (flags & RL_FLAG_BORDERLESS_WINDOWED_MODE)
    {
        SDL_SetWindowFullscreen(platform.window, 0);
    }
    if (flags & RL_FLAG_MSAA_4X_HINT)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0); // Disable multisampling buffers
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0); // Disable multisampling
    }
    if (flags & RL_FLAG_INTERLACED_HINT)
    {
        rlTRACELOG(RL_LOG_WARNING, "rlClearWindowState() - RL_FLAG_INTERLACED_HINT is not supported on PLATFORM_DESKTOP_SDL");
    }
}

// Set icon for window
void rlSetWindowIcon(rlImage image)
{
    SDL_Surface* iconSurface = NULL;

    Uint32 rmask, gmask, bmask, amask;
    int depth = 0;  // Depth in bits
    int pitch = 0;  // Pixel spacing (pitch) in bytes

    switch (image.format)
    {
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
            rmask = 0xFF, gmask = 0;
            bmask = 0, amask = 0;
            depth = 8, pitch = image.width;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
            rmask = 0xFF, gmask = 0xFF00;
            bmask = 0, amask = 0;
            depth = 16, pitch = image.width * 2;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5:
            rmask = 0xF800, gmask = 0x07E0;
            bmask = 0x001F, amask = 0;
            depth = 16, pitch = image.width * 2;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8:
            rmask = 0xFF0000, gmask = 0x00FF00;
            bmask = 0x0000FF, amask = 0;
            depth = 24, pitch = image.width * 3;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
            rmask = 0xF800, gmask = 0x07C0;
            bmask = 0x003E, amask = 0x0001;
            depth = 16, pitch = image.width * 2;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
            rmask = 0xF000, gmask = 0x0F00;
            bmask = 0x00F0, amask = 0x000F;
            depth = 16, pitch = image.width * 2;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
            rmask = 0xFF000000, gmask = 0x00FF0000;
            bmask = 0x0000FF00, amask = 0x000000FF;
            depth = 32, pitch = image.width * 4;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32:
            rmask = 0xFFFFFFFF, gmask = 0;
            bmask = 0, amask = 0;
            depth = 32, pitch = image.width * 4;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32:
            rmask = 0xFFFFFFFF, gmask = 0xFFFFFFFF;
            bmask = 0xFFFFFFFF, amask = 0;
            depth = 96, pitch = image.width * 12;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
            rmask = 0xFFFFFFFF, gmask = 0xFFFFFFFF;
            bmask = 0xFFFFFFFF, amask = 0xFFFFFFFF;
            depth = 128, pitch = image.width * 16;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16:
            rmask = 0xFFFF, gmask = 0;
            bmask = 0, amask = 0;
            depth = 16, pitch = image.width * 2;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16:
            rmask = 0xFFFF, gmask = 0xFFFF;
            bmask = 0xFFFF, amask = 0;
            depth = 48, pitch = image.width * 6;
            break;
        case RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
            rmask = 0xFFFF, gmask = 0xFFFF;
            bmask = 0xFFFF, amask = 0xFFFF;
            depth = 64, pitch = image.width * 8;
            break;
        default:
            // Compressed formats are not supported
            return;
    }

    iconSurface = SDL_CreateRGBSurfaceFrom(
        image.data, image.width, image.height, depth, pitch,
        rmask, gmask, bmask, amask
    );

    if (iconSurface)
    {
        SDL_SetWindowIcon(platform.window, iconSurface);
        SDL_FreeSurface(iconSurface);
    }
}

// Set icon for window
void rlSetWindowIcons(rlImage *images, int count)
{
    rlTRACELOG(RL_LOG_WARNING, "rlSetWindowIcons() not available on target platform");
}

// Set title for window
void rlSetWindowTitle(const char *title)
{
    SDL_SetWindowTitle(platform.window, title);

    rlCORE.Window.title = title;
}

// Set window position on screen (windowed mode)
void rlSetWindowPosition(int x, int y)
{
    SDL_SetWindowPosition(platform.window, x, y);

    rlCORE.Window.position.x = x;
    rlCORE.Window.position.y = y;
}

// Set monitor for the current window
void rlSetWindowMonitor(int monitor)
{
    const int monitorCount = SDL_GetNumVideoDisplays();
    if ((monitor >= 0) && (monitor < monitorCount))
    {
        // NOTE:
        // 1. SDL started supporting moving exclusive fullscreen windows between displays on SDL3,
        //    see commit https://github.com/libsdl-org/SDL/commit/3f5ef7dd422057edbcf3e736107e34be4b75d9ba
        // 2. A workround for SDL2 is leaving fullscreen, moving the window, then entering full screen again.
        const bool wasFullscreen = ((rlCORE.Window.flags & RL_FLAG_FULLSCREEN_MODE) > 0) ? true : false;

        const int screenWidth = rlCORE.Window.screen.width;
        const int screenHeight = rlCORE.Window.screen.height;
        SDL_Rect usableBounds;
        if (SDL_GetDisplayUsableBounds(monitor, &usableBounds) == 0)
        {
            if (wasFullscreen == 1) rlToggleFullscreen(); // Leave fullscreen.

            // If the screen size is larger than the monitor usable area, anchor it on the top left corner, otherwise, center it
            if ((screenWidth >= usableBounds.w) || (screenHeight >= usableBounds.h))
            {
                // NOTE:
                // 1. There's a known issue where if the window larger than the target display bounds,
                //    when moving the windows to that display, the window could be clipped back
                //    ending up positioned partly outside the target display.
                // 2. The workaround for that is, previously to moving the window,
                //    setting the window size to the target display size, so they match.
                // 3. It was't done here because we can't assume changing the window size automatically
                //    is acceptable behavior by the user.
                SDL_SetWindowPosition(platform.window, usableBounds.x, usableBounds.y);
                rlCORE.Window.position.x = usableBounds.x;
                rlCORE.Window.position.y = usableBounds.y;
            }
            else
            {
                const int x = usableBounds.x + (usableBounds.w/2) - (screenWidth/2);
                const int y = usableBounds.y + (usableBounds.h/2) - (screenHeight/2);
                SDL_SetWindowPosition(platform.window, x, y);
                rlCORE.Window.position.x = x;
                rlCORE.Window.position.y = y;
            }

            if (wasFullscreen == 1) rlToggleFullscreen(); // Re-enter fullscreen
        }
        else rlTRACELOG(RL_LOG_WARNING, "SDL: Failed to get selected display usable bounds");
    }
    else rlTRACELOG(RL_LOG_WARNING, "SDL: Failed to find selected monitor");
}

// Set window minimum dimensions (RL_FLAG_WINDOW_RESIZABLE)
void rlSetWindowMinSize(int width, int height)
{
    SDL_SetWindowMinimumSize(platform.window, width, height);

    rlCORE.Window.screenMin.width = width;
    rlCORE.Window.screenMin.height = height;
}

// Set window maximum dimensions (RL_FLAG_WINDOW_RESIZABLE)
void rlSetWindowMaxSize(int width, int height)
{
    SDL_SetWindowMaximumSize(platform.window, width, height);

    rlCORE.Window.screenMax.width = width;
    rlCORE.Window.screenMax.height = height;
}

// Set window dimensions
void rlSetWindowSize(int width, int height)
{
    SDL_SetWindowSize(platform.window, width, height);

    rlCORE.Window.screen.width = width;
    rlCORE.Window.screen.height = height;
}

// Set window opacity, value opacity is between 0.0 and 1.0
void rlSetWindowOpacity(float opacity)
{
    if (opacity >= 1.0f) opacity = 1.0f;
    else if (opacity <= 0.0f) opacity = 0.0f;

    SDL_SetWindowOpacity(platform.window, opacity);
}

// Set window focused
void rlSetWindowFocused(void)
{
    SDL_RaiseWindow(platform.window);
}

// Get native window handle
void *rlGetWindowHandle(void)
{
    return (void *)platform.window;
}

// Get number of monitors
int rlGetMonitorCount(void)
{
    int monitorCount = 0;

    monitorCount = SDL_GetNumVideoDisplays();

    return monitorCount;
}

// Get number of monitors
int rlGetCurrentMonitor(void)
{
    int currentMonitor = 0;

    currentMonitor = SDL_GetWindowDisplayIndex(platform.window);

    return currentMonitor;
}

// Get selected monitor position
rlVector2 rlGetMonitorPosition(int monitor)
{
    const int monitorCount = SDL_GetNumVideoDisplays();
    if ((monitor >= 0) && (monitor < monitorCount))
    {
        SDL_Rect displayBounds;
        if (SDL_GetDisplayUsableBounds(monitor, &displayBounds) == 0)
        {
            return (rlVector2){ (float)displayBounds.x, (float)displayBounds.y };
        }
        else rlTRACELOG(RL_LOG_WARNING, "SDL: Failed to get selected display usable bounds");
    }
    else rlTRACELOG(RL_LOG_WARNING, "SDL: Failed to find selected monitor");
    return (rlVector2){ 0.0f, 0.0f };
}

// Get selected monitor width (currently used by monitor)
int rlGetMonitorWidth(int monitor)
{
    int width = 0;

    const int monitorCount = SDL_GetNumVideoDisplays();
    if ((monitor >= 0) && (monitor < monitorCount))
    {
        SDL_DisplayMode mode;
        SDL_GetCurrentDisplayMode(monitor, &mode);
        width = mode.w;
    }
    else rlTRACELOG(RL_LOG_WARNING, "SDL: Failed to find selected monitor");

    return width;
}

// Get selected monitor height (currently used by monitor)
int rlGetMonitorHeight(int monitor)
{
    int height = 0;

    const int monitorCount = SDL_GetNumVideoDisplays();
    if ((monitor >= 0) && (monitor < monitorCount))
    {
        SDL_DisplayMode mode;
        SDL_GetCurrentDisplayMode(monitor, &mode);
        height = mode.h;
    }
    else rlTRACELOG(RL_LOG_WARNING, "SDL: Failed to find selected monitor");

    return height;
}

// Get selected monitor physical width in millimetres
int rlGetMonitorPhysicalWidth(int monitor)
{
    int width = 0;

    const int monitorCount = SDL_GetNumVideoDisplays();
    if ((monitor >= 0) && (monitor < monitorCount))
    {
        float ddpi = 0.0f;
        SDL_GetDisplayDPI(monitor, &ddpi, NULL, NULL);
        SDL_DisplayMode mode;
        SDL_GetCurrentDisplayMode(monitor, &mode);
        // Calculate size on inches, then convert to millimeter
        if (ddpi > 0.0f) width = (mode.w/ddpi)*25.4f;
    }
    else rlTRACELOG(RL_LOG_WARNING, "SDL: Failed to find selected monitor");

    return width;
}

// Get selected monitor physical height in millimetres
int rlGetMonitorPhysicalHeight(int monitor)
{
    int height = 0;

    const int monitorCount = SDL_GetNumVideoDisplays();
    if ((monitor >= 0) && (monitor < monitorCount))
    {
        float ddpi = 0.0f;
        SDL_GetDisplayDPI(monitor, &ddpi, NULL, NULL);
        SDL_DisplayMode mode;
        SDL_GetCurrentDisplayMode(monitor, &mode);
        // Calculate size on inches, then convert to millimeter
        if (ddpi > 0.0f) height = (mode.h/ddpi)*25.4f;
    }
    else rlTRACELOG(RL_LOG_WARNING, "SDL: Failed to find selected monitor");

    return height;
}

// Get selected monitor refresh rate
int rlGetMonitorRefreshRate(int monitor)
{
    int refresh = 0;

    const int monitorCount = SDL_GetNumVideoDisplays();
    if ((monitor >= 0) && (monitor < monitorCount))
    {
        SDL_DisplayMode mode;
        SDL_GetCurrentDisplayMode(monitor, &mode);
        refresh = mode.refresh_rate;
    }
    else rlTRACELOG(RL_LOG_WARNING, "SDL: Failed to find selected monitor");

    return refresh;
}

// Get the human-readable, UTF-8 encoded name of the selected monitor
const char *rlGetMonitorName(int monitor)
{
    const int monitorCount = SDL_GetNumVideoDisplays();

    if ((monitor >= 0) && (monitor < monitorCount)) return SDL_GetDisplayName(monitor);
    else rlTRACELOG(RL_LOG_WARNING, "SDL: Failed to find selected monitor");

    return "";
}

// Get window position XY on monitor
rlVector2 rlGetWindowPosition(void)
{
    int x = 0;
    int y = 0;

    SDL_GetWindowPosition(platform.window, &x, &y);

    return (rlVector2){ (float)x, (float)y };
}

// Get window scale DPI factor for current monitor
rlVector2 rlGetWindowScaleDPI(void)
{
    rlVector2 scale = { 1.0f, 1.0f };

    // NOTE: SDL_GetWindowDisplayScale was only added on SDL3
    //       see https://wiki.libsdl.org/SDL3/SDL_GetWindowDisplayScale
    // TODO: Implement the window scale factor calculation manually.
    rlTRACELOG(RL_LOG_WARNING, "rlGetWindowScaleDPI() not implemented on target platform");

    return scale;
}

// Set clipboard text content
void rlSetClipboardText(const char *text)
{
    SDL_SetClipboardText(text);
}

// Get clipboard text content
// NOTE: returned string must be freed with SDL_free()
const char *rlGetClipboardText(void)
{
    return SDL_GetClipboardText();
}

// Show mouse cursor
void rlShowCursor(void)
{
    SDL_ShowCursor(SDL_ENABLE);

    rlCORE.Input.Mouse.cursorHidden = false;
}

// Hides mouse cursor
void rlHideCursor(void)
{
    SDL_ShowCursor(SDL_DISABLE);

    rlCORE.Input.Mouse.cursorHidden = true;
}

// Enables cursor (unlock cursor)
void rlEnableCursor(void)
{
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_ShowCursor(SDL_ENABLE);

    platform.cursorRelative = false;
    rlCORE.Input.Mouse.cursorHidden = false;
}

// Disables cursor (lock cursor)
void rlDisableCursor(void)
{
    SDL_SetRelativeMouseMode(SDL_TRUE);

    platform.cursorRelative = true;
    rlCORE.Input.Mouse.cursorHidden = true;
}

// Swap back buffer with front buffer (screen drawing)
void rlSwapScreenBuffer(void)
{
    SDL_GL_SwapWindow(platform.window);
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

// Get elapsed time measure in seconds
double rlGetTime(void)
{
    unsigned int ms = SDL_GetTicks();    // Elapsed time in milliseconds since SDL_Init()
    double time = (double)ms/1000;
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
    else SDL_OpenURL(url);
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Inputs
//----------------------------------------------------------------------------------

// Set internal gamepad mappings
int rlSetGamepadMappings(const char *mappings)
{
    return SDL_GameControllerAddMapping(mappings);
}

// Set mouse position XY
void rlSetMousePosition(int x, int y)
{
    rlCORE.Input.Mouse.currentPosition = (rlVector2){ (float)x, (float)y };
    rlCORE.Input.Mouse.previousPosition = rlCORE.Input.Mouse.currentPosition;
}

// Set mouse cursor
void rlSetMouseCursor(int cursor)
{
    platform.cursor = SDL_CreateSystemCursor(CursorsLUT[cursor]);
    SDL_SetCursor(platform.cursor);

    rlCORE.Input.Mouse.cursor = cursor;
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

    // Reset key repeats
    for (int i = 0; i < RL_MAX_KEYBOARD_KEYS; i++) rlCORE.Input.Keyboard.keyRepeatInFrame[i] = 0;

    // Reset mouse wheel
    rlCORE.Input.Mouse.currentWheelMove.x = 0;
    rlCORE.Input.Mouse.currentWheelMove.y = 0;

    // Register previous mouse position
    if (platform.cursorRelative) rlCORE.Input.Mouse.currentPosition = (rlVector2){ 0.0f, 0.0f };
    else rlCORE.Input.Mouse.previousPosition = rlCORE.Input.Mouse.currentPosition;

    // Reset last gamepad button/axis registered state
    rlCORE.Input.Gamepad.lastButtonPressed = RL_GAMEPAD_BUTTON_UNKNOWN;
    for (int i = 0; i < RL_MAX_GAMEPADS; i++) rlCORE.Input.Gamepad.axisCount[i] = 0;

    // Register previous touch states
    for (int i = 0; i < RL_MAX_TOUCH_POINTS; i++) rlCORE.Input.Touch.previousTouchState[i] = rlCORE.Input.Touch.currentTouchState[i];

    // Reset touch positions
    // TODO: It resets on target platform the mouse position and not filled again until a move-event,
    // so, if mouse is not moved it returns a (0, 0) position... this behaviour should be reviewed!
    //for (int i = 0; i < RL_MAX_TOUCH_POINTS; i++) rlCORE.Input.Touch.position[i] = (rlVector2){ 0, 0 };

    // Map touch position to mouse position for convenience
    // WARNING: If the target desktop device supports touch screen, this behavious should be reviewed!
    // https://www.codeproject.com/Articles/668404/Programming-for-Multi-Touch
    // https://docs.microsoft.com/en-us/windows/win32/wintouch/getting-started-with-multi-touch-messages
    rlCORE.Input.Touch.position[0] = rlCORE.Input.Mouse.currentPosition;

    int touchAction = -1;       // 0-TOUCH_ACTION_UP, 1-TOUCH_ACTION_DOWN, 2-TOUCH_ACTION_MOVE
    bool gestureUpdate = false; // Flag to note gestures require to update

    // Register previous keys states
    // NOTE: Android supports up to 260 keys
    for (int i = 0; i < RL_MAX_KEYBOARD_KEYS; i++)
    {
        rlCORE.Input.Keyboard.previousKeyState[i] = rlCORE.Input.Keyboard.currentKeyState[i];
        rlCORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    // Register previous mouse states
    for (int i = 0; i < RL_MAX_MOUSE_BUTTONS; i++) rlCORE.Input.Mouse.previousButtonState[i] = rlCORE.Input.Mouse.currentButtonState[i];

    // Poll input events for current plaform
    //-----------------------------------------------------------------------------
    /*
    // WARNING: Indexes into this array are obtained by using SDL_Scancode values, not SDL_Keycode values
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    for (int i = 0; i < 256; ++i)
    {
        rlCORE.Input.Keyboard.currentKeyState[i] = keys[i];
        //if (keys[i]) rlTRACELOG(RL_LOG_WARNING, "Pressed key: %i", i);
    }
    */

    rlCORE.Window.resizedLastFrame = false;

    SDL_Event event = { 0 };
    while (SDL_PollEvent(&event) != 0)
    {
        // All input events can be processed after polling
        switch (event.type)
        {
            case SDL_QUIT: rlCORE.Window.shouldClose = true; break;

            case SDL_DROPFILE:      // Dropped file
            {
                if (rlCORE.Window.dropFileCount == 0)
                {
                    // When a new file is dropped, we reserve a fixed number of slots for all possible dropped files
                    // at the moment we limit the number of drops at once to 1024 files but this behaviour should probably be reviewed
                    // TODO: Pointers should probably be reallocated for any new file added...
                    rlCORE.Window.dropFilepaths = (char **)RL_CALLOC(1024, sizeof(char *));

                    rlCORE.Window.dropFilepaths[rlCORE.Window.dropFileCount] = (char *)RL_CALLOC(RL_MAX_FILEPATH_LENGTH, sizeof(char));
                    strcpy(rlCORE.Window.dropFilepaths[rlCORE.Window.dropFileCount], event.drop.file);
                    SDL_free(event.drop.file);

                    rlCORE.Window.dropFileCount++;
                }
                else if (rlCORE.Window.dropFileCount < 1024)
                {
                    rlCORE.Window.dropFilepaths[rlCORE.Window.dropFileCount] = (char *)RL_CALLOC(RL_MAX_FILEPATH_LENGTH, sizeof(char));
                    strcpy(rlCORE.Window.dropFilepaths[rlCORE.Window.dropFileCount], event.drop.file);
                    SDL_free(event.drop.file);

                    rlCORE.Window.dropFileCount++;
                }
                else rlTRACELOG(RL_LOG_WARNING, "FILE: Maximum drag and drop files at once is limited to 1024 files!");

            } break;

            // Window events are also polled (Minimized, maximized, close...)
            case SDL_WINDOWEVENT:
            {
                switch (event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        const int width = event.window.data1;
                        const int height = event.window.data2;
                        SetupViewport(width, height);
                        rlCORE.Window.screen.width = width;
                        rlCORE.Window.screen.height = height;
                        rlCORE.Window.currentFbo.width = width;
                        rlCORE.Window.currentFbo.height = height;
                        rlCORE.Window.resizedLastFrame = true;
                    } break;
                    case SDL_WINDOWEVENT_LEAVE:
                    case SDL_WINDOWEVENT_HIDDEN:
                    case SDL_WINDOWEVENT_MINIMIZED:
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                    case SDL_WINDOWEVENT_ENTER:
                    case SDL_WINDOWEVENT_SHOWN:
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                    case SDL_WINDOWEVENT_MAXIMIZED:
                    case SDL_WINDOWEVENT_RESTORED:
                    default: break;
                }
            } break;

            // Keyboard events
            case SDL_KEYDOWN:
            {
                rlKeyboardKey key = ConvertScancodeToKey(event.key.keysym.scancode);
                if (key != RL_KEY_NULL) rlCORE.Input.Keyboard.currentKeyState[key] = 1;

                // TODO: Put exitKey verification outside the switch?
                if (rlCORE.Input.Keyboard.currentKeyState[rlCORE.Input.Keyboard.exitKey])
                {
                    rlCORE.Window.shouldClose = true;
                }
            } break;

            case SDL_KEYUP:
            {
                rlKeyboardKey key = ConvertScancodeToKey(event.key.keysym.scancode);
                if (key != RL_KEY_NULL) rlCORE.Input.Keyboard.currentKeyState[key] = 0;
            } break;

            // Check mouse events
            case SDL_MOUSEBUTTONDOWN:
            {
                // NOTE: SDL2 mouse button order is LEFT, MIDDLE, RIGHT, but raylib uses LEFT, RIGHT, MIDDLE like GLFW
                //       The following conditions align SDL with raylib.h rlMouseButton enum order
                int btn = event.button.button - 1;
                if (btn == 2) btn = 1;
                else if (btn == 1) btn = 2;

                rlCORE.Input.Mouse.currentButtonState[btn] = 1;

                touchAction = 1;
                gestureUpdate = true;
            } break;
            case SDL_MOUSEBUTTONUP:
            {
                // NOTE: SDL2 mouse button order is LEFT, MIDDLE, RIGHT, but raylib uses LEFT, RIGHT, MIDDLE like GLFW
                //       The following conditions align SDL with raylib.h rlMouseButton enum order
                int btn = event.button.button - 1;
                if (btn == 2) btn = 1;
                else if (btn == 1) btn = 2;

                rlCORE.Input.Mouse.currentButtonState[btn] = 0;

                touchAction = 0;
                gestureUpdate = true;
            } break;
            case SDL_MOUSEWHEEL:
            {
                rlCORE.Input.Mouse.currentWheelMove.x = (float)event.wheel.x;
                rlCORE.Input.Mouse.currentWheelMove.y = (float)event.wheel.y;
            } break;
            case SDL_MOUSEMOTION:
            {
                if (platform.cursorRelative)
                {
                    rlCORE.Input.Mouse.currentPosition.x = (float)event.motion.xrel;
                    rlCORE.Input.Mouse.currentPosition.y = (float)event.motion.yrel;
                    rlCORE.Input.Mouse.previousPosition = (rlVector2){ 0.0f, 0.0f };
                }
                else
                {
                    rlCORE.Input.Mouse.currentPosition.x = (float)event.motion.x;
                    rlCORE.Input.Mouse.currentPosition.y = (float)event.motion.y;
                }

                rlCORE.Input.Touch.position[0] = rlCORE.Input.Mouse.currentPosition;
                touchAction = 2;
                gestureUpdate = true;
            } break;

            // Check gamepad events
            case SDL_JOYAXISMOTION:
            {
                // Motion on gamepad 0
                if (event.jaxis.which == 0)
                {
                    // X axis motion
                    if (event.jaxis.axis == 0)
                    {
                        //...
                    }
                    // Y axis motion
                    else if (event.jaxis.axis == 1)
                    {
                        //...
                    }
                }
            } break;
            default: break;
        }

#if defined(RL_SUPPORT_GESTURES_SYSTEM)
        if (gestureUpdate)
        {
            // Process mouse events as touches to be able to use mouse-gestures
            rlGestureEvent gestureEvent = { 0 };

            // Register touch actions
            gestureEvent.touchAction = touchAction;

            // Assign a pointer ID
            gestureEvent.pointId[0] = 0;

            // Register touch points count
            gestureEvent.pointCount = 1;

            // Register touch points position, only one point registered
            if (touchAction == 2) gestureEvent.position[0] = rlCORE.Input.Touch.position[0];
            else gestureEvent.position[0] = rlGetMousePosition();

            // rlNormalize gestureEvent.position[0] for rlCORE.Window.screen.width and rlCORE.Window.screen.height
            gestureEvent.position[0].x /= (float)rlGetScreenWidth();
            gestureEvent.position[0].y /= (float)rlGetScreenHeight();

            // rlGesture data is sent to gestures-system for processing
            rlProcessGestureEvent(gestureEvent);
        }
#endif
    }
    //-----------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

// Initialize platform: graphics, inputs and more
int InitPlatform(void)
{
    // Initialize SDL internal global state
    int result = SDL_Init(SDL_INIT_EVERYTHING);
    if (result < 0) { rlTRACELOG(RL_LOG_WARNING, "SDL: Failed to initialize SDL"); return -1; }

    // Initialize graphic device: display/window and graphic context
    //----------------------------------------------------------------------------
    unsigned int flags = 0;
    flags |= SDL_WINDOW_SHOWN;
    flags |= SDL_WINDOW_OPENGL;
    flags |= SDL_WINDOW_INPUT_FOCUS;
    flags |= SDL_WINDOW_MOUSE_FOCUS;
    flags |= SDL_WINDOW_MOUSE_CAPTURE;  // Window has mouse captured

    // Check window creation flags
    if ((rlCORE.Window.flags & RL_FLAG_FULLSCREEN_MODE) > 0)
    {
        rlCORE.Window.fullscreen = true;
        flags |= SDL_WINDOW_FULLSCREEN;
    }

    //if ((rlCORE.Window.flags & RL_FLAG_WINDOW_HIDDEN) == 0) flags |= SDL_WINDOW_HIDDEN;
    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_UNDECORATED) > 0) flags |= SDL_WINDOW_BORDERLESS;
    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_RESIZABLE) > 0) flags |= SDL_WINDOW_RESIZABLE;
    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_MINIMIZED) > 0) flags |= SDL_WINDOW_MINIMIZED;
    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_MAXIMIZED) > 0) flags |= SDL_WINDOW_MAXIMIZED;

    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_UNFOCUSED) > 0)
    {
        flags &= ~SDL_WINDOW_INPUT_FOCUS;
        flags &= ~SDL_WINDOW_MOUSE_FOCUS;
    }

    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_TOPMOST) > 0) flags |= SDL_WINDOW_ALWAYS_ON_TOP;
    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_MOUSE_PASSTHROUGH) > 0) flags &= ~SDL_WINDOW_MOUSE_CAPTURE;

    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_HIGHDPI) > 0) flags |= SDL_WINDOW_ALLOW_HIGHDPI;

    //if ((rlCORE.Window.flags & RL_FLAG_WINDOW_TRANSPARENT) > 0) flags |= SDL_WINDOW_TRANSPARENT;     // Alternative: SDL_GL_ALPHA_SIZE = 8

    //if ((rlCORE.Window.flags & RL_FLAG_FULLSCREEN_DESKTOP) > 0) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

    // NOTE: Some OpenGL context attributes must be set before window creation

    // Check selection OpenGL version
    if (rlglGetVersion() == RL_OPENGL_21)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    }
    else if (rlglGetVersion() == RL_OPENGL_33)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
#if defined(__APPLE__)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);  // OSX Requires forward compatibility
#else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    }
    else if (rlglGetVersion() == RL_OPENGL_43)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#if defined(RLGL_ENABLE_OPENGL_DEBUG_CONTEXT)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);   // Enable OpenGL Debug Context
#endif
    }
    else if (rlglGetVersion() == RL_OPENGL_ES_20)                 // Request OpenGL ES 2.0 context
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    }
    else if (rlglGetVersion() == RL_OPENGL_ES_30)                 // Request OpenGL ES 3.0 context
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    }

    if (rlCORE.Window.flags & RL_FLAG_VSYNC_HINT)
    {
        SDL_GL_SetSwapInterval(1);
    }

    if (rlCORE.Window.flags & RL_FLAG_MSAA_4X_HINT)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    }

    // Init window
    platform.window = SDL_CreateWindow(rlCORE.Window.title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, rlCORE.Window.screen.width, rlCORE.Window.screen.height, flags);

    // Init OpenGL context
    platform.glContext = SDL_GL_CreateContext(platform.window);

    // Check window and glContext have been initialized succesfully
    if ((platform.window != NULL) && (platform.glContext != NULL))
    {
        rlCORE.Window.ready = true;

        SDL_DisplayMode displayMode = { 0 };
        SDL_GetCurrentDisplayMode(rlGetCurrentMonitor(), &displayMode);

        rlCORE.Window.display.width = displayMode.w;
        rlCORE.Window.display.height = displayMode.h;

        rlCORE.Window.render.width = rlCORE.Window.screen.width;
        rlCORE.Window.render.height = rlCORE.Window.screen.height;
        rlCORE.Window.currentFbo.width = rlCORE.Window.render.width;
        rlCORE.Window.currentFbo.height = rlCORE.Window.render.height;

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

    // Load OpenGL extensions
    // NOTE: GL procedures address loader is required to load extensions
    rlglLoadExtensions(SDL_GL_GetProcAddress);
    //----------------------------------------------------------------------------

    // Initialize input events system
    //----------------------------------------------------------------------------
    if (SDL_NumJoysticks() >= 1)
    {
        platform.gamepad = SDL_JoystickOpen(0);
        //if (platform.gamepadgamepad == NULL) rlTRACELOG(RL_LOG_WARNING, "PLATFORM: Unable to open game controller [ERROR: %s]", SDL_GetError());
    }

    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    //----------------------------------------------------------------------------

    // Initialize timming system
    //----------------------------------------------------------------------------
    // NOTE: No need to call InitTimer(), let SDL manage it internally
    rlCORE.Time.previous = rlGetTime();     // Get time as double
    //----------------------------------------------------------------------------

    // Initialize storage system
    //----------------------------------------------------------------------------
    rlCORE.Storage.basePath = rlGetWorkingDirectory();  // Define base path for storage
    //----------------------------------------------------------------------------

    rlTRACELOG(RL_LOG_INFO, "PLATFORM: DESKTOP (SDL): Initialized successfully");

    return 0;
}

// Close platform
void ClosePlatform(void)
{
    SDL_FreeCursor(platform.cursor); // Free cursor
    SDL_GL_DeleteContext(platform.glContext); // Deinitialize OpenGL context
    SDL_DestroyWindow(platform.window);
    SDL_Quit(); // Deinitialize SDL internal global state
}

// Scancode to keycode mapping
static rlKeyboardKey ConvertScancodeToKey(SDL_Scancode sdlScancode)
{
    if (sdlScancode >= 0 && sdlScancode < SCANCODE_MAPPED_NUM)
    {
        return ScancodeToKey[sdlScancode];
    }
    return RL_KEY_NULL; // No equivalent key in Raylib
}

RL_NS_END

// EOF
