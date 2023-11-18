/**********************************************************************************************
*
*   rcore - Window/display management, Graphic device/context management and input management
*
*   PLATFORMS SUPPORTED:
*       > PLATFORM_DESKTOP (GLFW backend):
*           - Windows (Win32, Win64)
*           - Linux (X11/Wayland desktop mode)
*           - macOS/OSX (x64, arm64)
*           - FreeBSD, OpenBSD, NetBSD, DragonFly (X11 desktop)
*       > PLATFORM_DESKTOP_SDL (SDL backend):
*           - Windows (Win32, Win64)
*           - Linux (X11/Wayland desktop mode)
*           - Others (not tested)
*       > PLATFORM_WEB:
*           - HTML5 (WebAssembly)
*       > PLATFORM_DRM:
*           - Raspberry Pi 0-5 (DRM/KMS)
*           - Linux DRM subsystem (KMS mode)
*       > PLATFORM_ANDROID:
*           - Android (ARM, ARM64)
*
*   CONFIGURATION:
*       #define RL_SUPPORT_DEFAULT_FONT (default)
*           Default font is loaded on window initialization to be available for the user to render simple text.
*           NOTE: If enabled, uses external module functions to load default raylib font (module: text)
*
*       #define RL_SUPPORT_CAMERA_SYSTEM
*           rlCamera module is included (rcamera.h) and multiple predefined cameras are available:
*               free, 1st/3rd person, orbital, custom
*
*       #define RL_SUPPORT_GESTURES_SYSTEM
*           Gestures module is included (rgestures.h) to support gestures detection: tap, hold, swipe, drag
*
*       #define RL_SUPPORT_MOUSE_GESTURES
*           Mouse gestures are directly mapped like touches and processed by gestures system.
*
*       #define RL_SUPPORT_BUSY_WAIT_LOOP
*           Use busy wait loop for timing sync, if not defined, a high-resolution timer is setup and used
*
*       #define RL_SUPPORT_PARTIALBUSY_WAIT_LOOP
*           Use a partial-busy wait loop, in this case frame sleeps for most of the time and runs a busy-wait-loop at the end
*
*       #define RL_SUPPORT_SCREEN_CAPTURE
*           Allow automatic screen capture of current screen pressing F12, defined in KeyCallback()
*
*       #define RL_SUPPORT_GIF_RECORDING
*           Allow automatic gif recording of current screen pressing CTRL+F12, defined in KeyCallback()
*
*       #define RL_SUPPORT_COMPRESSION_API
*           Support rlCompressData() and rlDecompressData() functions, those functions use zlib implementation
*           provided by stb_image and stb_image_write libraries, so, those libraries must be enabled on textures module
*           for linkage
*
*       #define RL_SUPPORT_AUTOMATION_EVENTS
*           Support automatic events recording and playing, useful for automated testing systems or AI based game playing
*
*   DEPENDENCIES:
*       raymath  - 3D math functionality (rlVector2, rlVector3, rlMatrix, rlQuaternion)
*       camera   - Multiple 3D camera modes (free, orbital, 1st person, 3rd person)
*       gestures - Gestures system for touch-ready devices (or simulated from mouse inputs)
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

#include "raylib.h"                 // Declares module functions

// Check if config flags have been externally provided on compilation line
#if !defined(EXTERNAL_CONFIG_FLAGS)
    #include "config.h"             // Defines module configuration flags
#endif

#include "utils.h"                  // Required for: rlTRACELOG() macros

#include <stdlib.h>                 // Required for: srand(), rand(), atexit()
#include <stdio.h>                  // Required for: sprintf() [Used in rlOpenURL()]
#include <string.h>                 // Required for: strrchr(), strcmp(), strlen(), memset()
#include <time.h>                   // Required for: time() [Used in InitTimer()]
#include <math.h>                   // Required for: tan() [Used in rlBeginMode3D()], atan2f() [Used in rlLoadVrStereoConfig()]

#define RLGL_IMPLEMENTATION
#include "rlgl.h"                   // OpenGL abstraction layer to OpenGL 1.1, 3.3+ or ES2

#define RAYMATH_IMPLEMENTATION
#include "raymath.h"                // rlVector2, rlVector3, rlQuaternion and rlMatrix functionality

#if defined(RL_SUPPORT_GESTURES_SYSTEM)
    #define RGESTURES_IMPLEMENTATION
    #include "rgestures.h"           // Gestures detection functionality
#endif

#if defined(RL_SUPPORT_CAMERA_SYSTEM)
    #define RCAMERA_IMPLEMENTATION
    #include "rcamera.h"             // rlCamera system functionality
#endif

#if defined(RL_SUPPORT_GIF_RECORDING)
    #define MSF_GIF_MALLOC(contextPointer, newSize) RL_MALLOC(newSize)
    #define MSF_GIF_REALLOC(contextPointer, oldMemory, oldSize, newSize) RL_REALLOC(oldMemory, newSize)
    #define MSF_GIF_FREE(contextPointer, oldMemory, oldSize) RL_FREE(oldMemory)

    #define MSF_GIF_IMPL
    #include "external/msf_gif.h"   // GIF recording functionality
#endif

#if defined(RL_SUPPORT_COMPRESSION_API)
    #define SINFL_IMPLEMENTATION
    #define SINFL_NO_SIMD
    #include "external/sinfl.h"     // Deflate (RFC 1951) decompressor

    #define SDEFL_IMPLEMENTATION
    #include "external/sdefl.h"     // Deflate (RFC 1951) compressor
#endif

#if defined(RL_SUPPORT_RPRAND_GENERATOR)
    #define RPRAND_IMPLEMENTATION
    #include "external/rprand.h"
#endif

#if defined(__linux__) && !defined(_GNU_SOURCE)
    #define _GNU_SOURCE
#endif

// Platform specific defines to handle rlGetApplicationDirectory()
#if defined(_WIN32)
    #ifndef RL_MAX_PATH
        #define RL_MAX_PATH 1025
    #endif
    #ifdef __cplusplus
    extern "C" {
    #endif
    __declspec(dllimport) unsigned long __stdcall GetModuleFileNameA(void *hModule, void *lpFilename, unsigned long nSize);
    __declspec(dllimport) unsigned long __stdcall GetModuleFileNameW(void *hModule, void *lpFilename, unsigned long nSize);
    __declspec(dllimport) int __stdcall WideCharToMultiByte(unsigned int cp, unsigned long flags, void *widestr, int cchwide, void *str, int cbmb, void *defchar, int *used_default);
    #ifdef __cplusplus
    }
    #endif
#elif defined(__linux__)
    #include <unistd.h>
#elif defined(__APPLE__)
    #include <sys/syslimits.h>
    #include <mach-o/dyld.h>
#endif // OSs

#define _CRT_INTERNAL_NONSTDC_NAMES  1
#include <sys/stat.h>               // Required for: stat(), S_ISREG [Used in rlGetFileModTime(), IsFilePath()]

#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
    #define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

#if defined(_WIN32) && (defined(_MSC_VER) || defined(__TINYC__))
    #define DIRENT_MALLOC RL_MALLOC
    #define DIRENT_FREE RL_FREE

    #include "external/dirent.h"    // Required for: DIR, opendir(), closedir() [Used in rlLoadDirectoryFiles()]
#else
    #include <dirent.h>             // Required for: DIR, opendir(), closedir() [Used in rlLoadDirectoryFiles()]
#endif

#if defined(_WIN32)
    #include <direct.h>             // Required for: _getch(), _chdir()
    #define GETCWD _getcwd          // NOTE: MSDN recommends not to use getcwd(), chdir()
    #define CHDIR _chdir
    #include <io.h>                 // Required for: _access() [Used in rlFileExists()]
#else
    #include <unistd.h>             // Required for: getch(), chdir() (POSIX), access()
    #define GETCWD getcwd
    #define CHDIR chdir
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#ifndef RL_MAX_FILEPATH_CAPACITY
    #define RL_MAX_FILEPATH_CAPACITY       8192        // Maximum capacity for filepath
#endif
#ifndef RL_MAX_FILEPATH_LENGTH
    #if defined(_WIN32)
        #define RL_MAX_FILEPATH_LENGTH      256        // On Win32, RL_MAX_PATH = 260 (limits.h) but Windows 10, Version 1607 enables long paths...
    #else
        #define RL_MAX_FILEPATH_LENGTH     4096        // On Linux, PATH_MAX = 4096 by default (limits.h)
    #endif
#endif

#ifndef RL_MAX_KEYBOARD_KEYS
    #define RL_MAX_KEYBOARD_KEYS            512        // Maximum number of keyboard keys supported
#endif
#ifndef RL_MAX_MOUSE_BUTTONS
    #define RL_MAX_MOUSE_BUTTONS              8        // Maximum number of mouse buttons supported
#endif
#ifndef RL_MAX_GAMEPADS
    #define RL_MAX_GAMEPADS                   4        // Maximum number of gamepads supported
#endif
#ifndef RL_MAX_GAMEPAD_AXIS
    #define RL_MAX_GAMEPAD_AXIS               8        // Maximum number of axis supported (per gamepad)
#endif
#ifndef RL_MAX_GAMEPAD_BUTTONS
    #define RL_MAX_GAMEPAD_BUTTONS           32        // Maximum number of buttons supported (per gamepad)
#endif
#ifndef RL_MAX_TOUCH_POINTS
    #define RL_MAX_TOUCH_POINTS               8        // Maximum number of touch points supported
#endif
#ifndef RL_MAX_KEY_PRESSED_QUEUE
    #define RL_MAX_KEY_PRESSED_QUEUE         16        // Maximum number of keys in the key input queue
#endif
#ifndef RL_MAX_CHAR_PRESSED_QUEUE
    #define RL_MAX_CHAR_PRESSED_QUEUE        16        // Maximum number of characters in the char input queue
#endif

#ifndef RL_MAX_DECOMPRESSION_SIZE
    #define RL_MAX_DECOMPRESSION_SIZE        64        // Maximum size allocated for decompression in MB
#endif

#ifndef RL_MAX_AUTOMATION_EVENTS
    #define RL_MAX_AUTOMATION_EVENTS      16384        // Maximum number of automation events to record
#endif

// Flags operation macros
#define RL_FLAG_SET(n, f) ((n) |= (f))
#define RL_FLAG_CLEAR(n, f) ((n) &= ~(f))
#define RL_FLAG_TOGGLE(n, f) ((n) ^= (f))
#define RL_FLAG_CHECK(n, f) ((n) & (f))

#if (defined(__linux__) || defined(PLATFORM_WEB)) && (_POSIX_C_SOURCE < 199309L)
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 199309L // Required for: CLOCK_MONOTONIC if compiled with c99 without gnu ext.
#endif

RL_NS_BEGIN

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct { int x; int y; } rlPoint;
typedef struct { unsigned int width; unsigned int height; } rlSize;

// Core global state context data
typedef struct rlCoreData {
    struct {
        const char *title;                  // Window text title const pointer
        unsigned int flags;                 // Configuration flags (bit based), keeps window state
        bool ready;                         // Check if window has been initialized successfully
        bool fullscreen;                    // Check if fullscreen mode is enabled
        bool shouldClose;                   // Check if window set for closing
        bool resizedLastFrame;              // Check if window has been resized last frame
        bool eventWaiting;                  // Wait for events before ending frame
        bool usingFbo;                      // Using FBO (rlRenderTexture) for rendering instead of default framebuffer

        rlPoint position;                     // Window position (required on fullscreen toggle)
        rlPoint previousPosition;             // Window previous position (required on borderless windowed toggle)
        rlSize display;                       // Display width and height (monitor, device-screen, LCD, ...)
        rlSize screen;                        // Screen width and height (used render area)
        rlSize previousScreen;                // Screen previous width and height (required on borderless windowed toggle)
        rlSize currentFbo;                    // Current render width and height (depends on active fbo)
        rlSize render;                        // Framebuffer width and height (render area, including black bars if required)
        rlPoint renderOffset;                 // Offset from render area (must be divided by 2)
        rlSize screenMin;                     // Screen minimum width and height (for resizable window)
        rlSize screenMax;                     // Screen maximum width and height (for resizable window)
        rlMatrix screenScale;                 // rlMatrix to scale screen (framebuffer rendering)

        char **dropFilepaths;               // Store dropped files paths pointers (provided by GLFW)
        unsigned int dropFileCount;         // Count dropped files strings

    } Window;
    struct {
        const char *basePath;               // Base path for data storage

    } Storage;
    struct {
        struct {
            int exitKey;                    // Default exit key
            char currentKeyState[RL_MAX_KEYBOARD_KEYS]; // Registers current frame key state
            char previousKeyState[RL_MAX_KEYBOARD_KEYS]; // Registers previous frame key state

            // NOTE: Since key press logic involves comparing prev vs cur key state, we need to handle key repeats specially
            char keyRepeatInFrame[RL_MAX_KEYBOARD_KEYS]; // Registers key repeats for current frame.

            int keyPressedQueue[RL_MAX_KEY_PRESSED_QUEUE]; // Input keys queue
            int keyPressedQueueCount;       // Input keys queue count

            int charPressedQueue[RL_MAX_CHAR_PRESSED_QUEUE]; // Input characters queue (unicode)
            int charPressedQueueCount;      // Input characters queue count

        } Keyboard;
        struct {
            rlVector2 offset;                 // Mouse offset
            rlVector2 scale;                  // Mouse scaling
            rlVector2 currentPosition;        // Mouse position on screen
            rlVector2 previousPosition;       // Previous mouse position

            int cursor;                     // Tracks current mouse cursor
            bool cursorHidden;              // Track if cursor is hidden
            bool cursorOnScreen;            // Tracks if cursor is inside client area

            char currentButtonState[RL_MAX_MOUSE_BUTTONS];     // Registers current mouse button state
            char previousButtonState[RL_MAX_MOUSE_BUTTONS];    // Registers previous mouse button state
            rlVector2 currentWheelMove;       // Registers current mouse wheel variation
            rlVector2 previousWheelMove;      // Registers previous mouse wheel variation

        } Mouse;
        struct {
            int pointCount;                             // Number of touch points active
            int pointId[RL_MAX_TOUCH_POINTS];              // rlPoint identifiers
            rlVector2 position[RL_MAX_TOUCH_POINTS];         // Touch position on screen
            char currentTouchState[RL_MAX_TOUCH_POINTS];   // Registers current touch state
            char previousTouchState[RL_MAX_TOUCH_POINTS];  // Registers previous touch state

        } Touch;
        struct {
            int lastButtonPressed;          // Register last gamepad button pressed
            int axisCount[RL_MAX_GAMEPADS];                  // Register number of available gamepad axis
            bool ready[RL_MAX_GAMEPADS];       // Flag to know if gamepad is ready
            char name[RL_MAX_GAMEPADS][64];    // Gamepad name holder
            char currentButtonState[RL_MAX_GAMEPADS][RL_MAX_GAMEPAD_BUTTONS];     // Current gamepad buttons state
            char previousButtonState[RL_MAX_GAMEPADS][RL_MAX_GAMEPAD_BUTTONS];    // Previous gamepad buttons state
            float axisState[RL_MAX_GAMEPADS][RL_MAX_GAMEPAD_AXIS];                // Gamepad axis state

        } Gamepad;
    } Input;
    struct {
        double current;                     // Current time measure
        double previous;                    // Previous time measure
        double update;                      // Time measure for frame update
        double draw;                        // Time measure for frame draw
        double frame;                       // Time measure for one frame
        double target;                      // Desired time for one frame, if 0 not applied
        unsigned long long int base;        // Base time measure for hi-res timer (PLATFORM_ANDROID, PLATFORM_DRM)
        unsigned int frameCounter;          // Frame counter

    } Time;
} rlCoreData;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
RLAPI const char *raylib_version = RAYLIB_VERSION;  // raylib version exported symbol, required for some bindings

rlCoreData rlCORE = { 0 };               // Global rlCORE state context

#if defined(RL_SUPPORT_SCREEN_CAPTURE)
static int screenshotCounter = 0;    // Screenshots counter
#endif

#if defined(RL_SUPPORT_GIF_RECORDING)
int gifFrameCounter = 0;             // GIF frames counter
bool gifRecording = false;           // GIF recording state
MsfGifState gifState = { 0 };        // MSGIF context state
#endif

#if defined(RL_SUPPORT_AUTOMATION_EVENTS)
// Automation events type
typedef enum AutomationEventType {
    EVENT_NONE = 0,
    // Input events
    INPUT_KEY_UP,                   // param[0]: key
    INPUT_KEY_DOWN,                 // param[0]: key
    INPUT_KEY_PRESSED,              // param[0]: key
    INPUT_KEY_RELEASED,             // param[0]: key
    INPUT_MOUSE_BUTTON_UP,          // param[0]: button
    INPUT_MOUSE_BUTTON_DOWN,        // param[0]: button
    INPUT_MOUSE_POSITION,           // param[0]: x, param[1]: y
    INPUT_MOUSE_WHEEL_MOTION,       // param[0]: x delta, param[1]: y delta
    INPUT_GAMEPAD_CONNECT,          // param[0]: gamepad
    INPUT_GAMEPAD_DISCONNECT,       // param[0]: gamepad
    INPUT_GAMEPAD_BUTTON_UP,        // param[0]: button
    INPUT_GAMEPAD_BUTTON_DOWN,      // param[0]: button
    INPUT_GAMEPAD_AXIS_MOTION,      // param[0]: axis, param[1]: delta
    INPUT_TOUCH_UP,                 // param[0]: id
    INPUT_TOUCH_DOWN,               // param[0]: id
    INPUT_TOUCH_POSITION,           // param[0]: x, param[1]: y
    INPUT_GESTURE,                  // param[0]: gesture
    // Window events
    WINDOW_CLOSE,                   // no params
    WINDOW_MAXIMIZE,                // no params
    WINDOW_MINIMIZE,                // no params
    WINDOW_RESIZE,                  // param[0]: width, param[1]: height
    // Custom events
    ACTION_TAKE_SCREENSHOT,         // no params
    ACTION_SETTARGETFPS             // param[0]: fps
} AutomationEventType;

// Event type to config events flags
// TODO: Not used at the moment
typedef enum {
    EVENT_INPUT_KEYBOARD    = 0,
    EVENT_INPUT_MOUSE       = 1,
    EVENT_INPUT_GAMEPAD     = 2,
    EVENT_INPUT_TOUCH       = 4,
    EVENT_INPUT_GESTURE     = 8,
    EVENT_WINDOW            = 16,
    EVENT_CUSTOM            = 32
} EventType;

// Event type name strings, required for export
static const char *autoEventTypeName[] = {
    "EVENT_NONE",
    "INPUT_KEY_UP",
    "INPUT_KEY_DOWN",
    "INPUT_KEY_PRESSED",
    "INPUT_KEY_RELEASED",
    "INPUT_MOUSE_BUTTON_UP",
    "INPUT_MOUSE_BUTTON_DOWN",
    "INPUT_MOUSE_POSITION",
    "INPUT_MOUSE_WHEEL_MOTION",
    "INPUT_GAMEPAD_CONNECT",
    "INPUT_GAMEPAD_DISCONNECT",
    "INPUT_GAMEPAD_BUTTON_UP",
    "INPUT_GAMEPAD_BUTTON_DOWN",
    "INPUT_GAMEPAD_AXIS_MOTION",
    "INPUT_TOUCH_UP",
    "INPUT_TOUCH_DOWN",
    "INPUT_TOUCH_POSITION",
    "INPUT_GESTURE",
    "WINDOW_CLOSE",
    "WINDOW_MAXIMIZE",
    "WINDOW_MINIMIZE",
    "WINDOW_RESIZE",
    "ACTION_TAKE_SCREENSHOT",
    "ACTION_SETTARGETFPS"
};

/*
// Automation event (24 bytes)
// NOTE: Opaque struct, internal to raylib
struct rlAutomationEvent {
    unsigned int frame;                 // Event frame
    unsigned int type;                  // Event type (AutomationEventType)
    int params[4];                      // Event parameters (if required)
};
*/

static rlAutomationEventList *currentEventList = NULL;        // Current automation events list, set by user, keep internal pointer
static bool automationEventRecording = false;               // Recording automation events flag
//static short automationEventEnabled = 0b0000001111111111; // TODO: Automation events enabled for recording/playing
#endif
//-----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
// Module Functions Declaration
// NOTE: Those functions are common for all platforms!
//----------------------------------------------------------------------------------

#if defined(RL_SUPPORT_MODULE_RTEXT) && defined(RL_SUPPORT_DEFAULT_FONT)
extern void LoadFontDefault(void);      // [Module: text] Loads default font on rlInitWindow()
extern void UnloadFontDefault(void);    // [Module: text] Unloads default font from GPU memory
#endif

extern int InitPlatform(void);          // Initialize platform (graphics, inputs and more)
extern void ClosePlatform(void);        // Close platform

static void InitTimer(void);                                // Initialize timer, hi-resolution if available (required by InitPlatform())
static void SetupFramebuffer(int width, int height);        // Setup main framebuffer (required by InitPlatform())
static void SetupViewport(int width, int height);           // Set viewport for a provided width and height

static void ScanDirectoryFiles(const char *basePath, rlFilePathList *list, const char *filter);   // Scan all files and directories in a base path
static void ScanDirectoryFilesRecursively(const char *basePath, rlFilePathList *list, const char *filter);  // Scan all files and directories recursively from a base path

#if defined(RL_SUPPORT_AUTOMATION_EVENTS)
static void RecordAutomationEvent(void); // Record frame events (to internal events array)
#endif

#if defined(_WIN32)
    #ifdef __cplusplus
    extern "C" {
    #endif
        // NOTE: We declare Sleep() function symbol to avoid including windows.h (kernel32.lib linkage required)
        void __stdcall Sleep(unsigned long msTimeout);              // Required for: rlWaitTime()
    #ifdef __cplusplus
    }
    #endif
#endif

#if !defined(RL_SUPPORT_MODULE_RTEXT)
const char *rlTextFormat(const char *text, ...);              // Formatting of text with variables to 'embed'
#endif // !RL_SUPPORT_MODULE_RTEXT

// Include platform-specific submodules
#if defined(PLATFORM_DESKTOP)
    #include "platforms/rcore_desktop.c"
#elif defined(PLATFORM_DESKTOP_SDL)
    #include "platforms/rcore_desktop_sdl.c"
#elif defined(PLATFORM_WEB)
    #include "platforms/rcore_web.c"
#elif defined(PLATFORM_DRM)
    #include "platforms/rcore_drm.c"
#elif defined(PLATFORM_ANDROID)
    #include "platforms/rcore_android.c"
#else
    // TODO: Include your custom platform backend!
    // i.e software rendering backend or console backend!
#endif

//----------------------------------------------------------------------------------
// Module Functions Definition: Window and Graphics Device
//----------------------------------------------------------------------------------

// NOTE: Functions with a platform-specific implementation on rcore_<platform>.c
//bool rlWindowShouldClose(void)
//void rlToggleFullscreen(void)
//void rlToggleBorderlessWindowed(void)
//void rlMaximizeWindow(void)
//void rlMinimizeWindow(void)
//void rlRestoreWindow(void)

//void rlSetWindowState(unsigned int flags)
//void rlClearWindowState(unsigned int flags)

//void rlSetWindowIcon(rlImage image)
//void rlSetWindowIcons(rlImage *images, int count)
//void rlSetWindowTitle(const char *title)
//void rlSetWindowPosition(int x, int y)
//void rlSetWindowMonitor(int monitor)
//void rlSetWindowMinSize(int width, int height)
//void rlSetWindowMaxSize(int width, int height)
//void rlSetWindowSize(int width, int height)
//void rlSetWindowOpacity(float opacity)
//void rlSetWindowFocused(void)
//void *rlGetWindowHandle(void)
//rlVector2 rlGetWindowPosition(void)
//rlVector2 rlGetWindowScaleDPI(void)

//int rlGetMonitorCount(void)
//int rlGetCurrentMonitor(void)
//int rlGetMonitorWidth(int monitor)
//int rlGetMonitorHeight(int monitor)
//int rlGetMonitorPhysicalWidth(int monitor)
//int rlGetMonitorPhysicalHeight(int monitor)
//int rlGetMonitorRefreshRate(int monitor)
//rlVector2 rlGetMonitorPosition(int monitor)
//const char *rlGetMonitorName(int monitor)

//void rlSetClipboardText(const char *text)
//const char *rlGetClipboardText(void)

//void rlShowCursor(void)
//void rlHideCursor(void)
//void rlEnableCursor(void)
//void rlDisableCursor(void)

// Initialize window and OpenGL context
// NOTE: data parameter could be used to pass any kind of required data to the initialization
void rlInitWindow(int width, int height, const char *title)
{
    rlTRACELOG(RL_LOG_INFO, "Initializing raylib %s", RAYLIB_VERSION);

#if defined(PLATFORM_DESKTOP)
    rlTRACELOG(RL_LOG_INFO, "Platform backend: DESKTOP (GLFW)");
#elif defined(PLATFORM_DESKTOP_SDL)
    rlTRACELOG(RL_LOG_INFO, "Platform backend: DESKTOP (SDL)");
#elif defined(PLATFORM_WEB)
    rlTRACELOG(RL_LOG_INFO, "Platform backend: WEB (HTML5)");
#elif defined(PLATFORM_DRM)
    rlTRACELOG(RL_LOG_INFO, "Platform backend: NATIVE DRM");
#elif defined(PLATFORM_ANDROID)
    rlTRACELOG(RL_LOG_INFO, "Platform backend: ANDROID");
#else
    // TODO: Include your custom platform backend!
    // i.e software rendering backend or console backend!
    rlTRACELOG(RL_LOG_INFO, "Platform backend: CUSTOM");
#endif

    rlTRACELOG(RL_LOG_INFO, "Supported raylib modules:");
    rlTRACELOG(RL_LOG_INFO, "    > rcore:..... loaded (mandatory)");
    rlTRACELOG(RL_LOG_INFO, "    > rlgl:...... loaded (mandatory)");
#if defined(RL_SUPPORT_MODULE_RSHAPES)
    rlTRACELOG(RL_LOG_INFO, "    > rshapes:... loaded (optional)");
#else
    rlTRACELOG(RL_LOG_INFO, "    > rshapes:... not loaded (optional)");
#endif
#if defined(RL_SUPPORT_MODULE_RTEXTURES)
    rlTRACELOG(RL_LOG_INFO, "    > rtextures:. loaded (optional)");
#else
    rlTRACELOG(RL_LOG_INFO, "    > rtextures:. not loaded (optional)");
#endif
#if defined(RL_SUPPORT_MODULE_RTEXT)
    rlTRACELOG(RL_LOG_INFO, "    > rtext:..... loaded (optional)");
#else
    rlTRACELOG(RL_LOG_INFO, "    > rtext:..... not loaded (optional)");
#endif
#if defined(RL_SUPPORT_MODULE_RMODELS)
    rlTRACELOG(RL_LOG_INFO, "    > rmodels:... loaded (optional)");
#else
    rlTRACELOG(RL_LOG_INFO, "    > rmodels:... not loaded (optional)");
#endif
#if defined(RL_SUPPORT_MODULE_RAUDIO)
    rlTRACELOG(RL_LOG_INFO, "    > raudio:.... loaded (optional)");
#else
    rlTRACELOG(RL_LOG_INFO, "    > raudio:.... not loaded (optional)");
#endif

    // Initialize window data
    rlCORE.Window.screen.width = width;
    rlCORE.Window.screen.height = height;
    rlCORE.Window.eventWaiting = false;
    rlCORE.Window.screenScale = rlMatrixIdentity();     // No draw scaling required by default
    if ((title != NULL) && (title[0] != 0)) rlCORE.Window.title = title;

    // Initialize global input state
    memset(&rlCORE.Input, 0, sizeof(rlCORE.Input));     // Reset rlCORE.Input structure to 0
    rlCORE.Input.Keyboard.exitKey = RL_KEY_ESCAPE;
#if __cplusplus
    rlCORE.Input.Mouse.scale = rlVector2{ 1.0f, 1.0f };
#else
    rlCORE.Input.Mouse.scale = (rlVector2){ 1.0f, 1.0f };
#endif
    rlCORE.Input.Mouse.cursor = RL_MOUSE_CURSOR_ARROW;
    rlCORE.Input.Gamepad.lastButtonPressed = RL_GAMEPAD_BUTTON_UNKNOWN;

    // Initialize platform
    //--------------------------------------------------------------
    InitPlatform();
    //--------------------------------------------------------------

    // Initialize rlgl default data (buffers and shaders)
    // NOTE: rlCORE.Window.currentFbo.width and rlCORE.Window.currentFbo.height not used, just stored as globals in rlgl
    rlglInit(rlCORE.Window.currentFbo.width, rlCORE.Window.currentFbo.height);

    // Setup default viewport
    SetupViewport(rlCORE.Window.currentFbo.width, rlCORE.Window.currentFbo.height);

#if defined(RL_SUPPORT_MODULE_RTEXT) && defined(RL_SUPPORT_DEFAULT_FONT)
    // Load default font
    // WARNING: External function: Module required: rtext
    LoadFontDefault();
    #if defined(RL_SUPPORT_MODULE_RSHAPES)
    // Set font white rectangle for shapes drawing, so shapes and text can be batched together
    // WARNING: rshapes module is required, if not available, default internal white rectangle is used
    rlRectangle rec = rlGetFontDefault().recs[95];

    if (rlCORE.Window.flags & RL_FLAG_MSAA_4X_HINT)
    {
        // NOTE: We try to maxime rec padding to avoid pixel bleeding on MSAA filtering
        rlSetShapesTexture(rlGetFontDefault().texture, CAST(rlRectangle){ rec.x + 2, rec.y + 2, 1, 1 });
    }
    else
    {
        // NOTE: We set up a 1px padding on char rectangle to avoid pixel bleeding
        rlSetShapesTexture(rlGetFontDefault().texture, CAST(rlRectangle){ rec.x + 1, rec.y + 1, rec.width - 2, rec.height - 2 });
    }
    #endif
#else
    #if defined(RL_SUPPORT_MODULE_RSHAPES)
    // Set default texture and rectangle to be used for shapes drawing
    // NOTE: rlgl default texture is a 1x1 pixel UNCOMPRESSED_R8G8B8A8
    rlTexture2D texture = { rlglGetTextureIdDefault(), 1, 1, 1, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
    rlSetShapesTexture(texture, (rlRectangle){ 0.0f, 0.0f, 1.0f, 1.0f });    // WARNING: Module required: rshapes
    #endif
#endif
#if defined(RL_SUPPORT_MODULE_RTEXT) && defined(RL_SUPPORT_DEFAULT_FONT)
    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_HIGHDPI) > 0)
    {
        // Set default font texture filter for HighDPI (blurry)
        // RL_TEXTURE_FILTER_LINEAR - tex filter: BILINEAR, no mipmaps
        rlglTextureParameters(rlGetFontDefault().texture.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR);
        rlglTextureParameters(rlGetFontDefault().texture.id, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR);
    }
#endif

    rlCORE.Time.frameCounter = 0;
    rlCORE.Window.shouldClose = false;

    // Initialize random seed
    rlSetRandomSeed((unsigned int)time(NULL));
}

// Close window and unload OpenGL context
void rlCloseWindow(void)
{
#if defined(RL_SUPPORT_GIF_RECORDING)
    if (gifRecording)
    {
        MsfGifResult result = msf_gif_end(&gifState);
        msf_gif_free(result);
        gifRecording = false;
    }
#endif

#if defined(RL_SUPPORT_MODULE_RTEXT) && defined(RL_SUPPORT_DEFAULT_FONT)
    UnloadFontDefault();        // WARNING: Module required: rtext
#endif

    rlglClose();                // De-init rlgl

    // De-initialize platform
    //--------------------------------------------------------------
    ClosePlatform();
    //--------------------------------------------------------------

    rlCORE.Window.ready = false;
    rlTRACELOG(RL_LOG_INFO, "Window closed successfully");
}

// Check if window has been initialized successfully
bool rlIsWindowReady(void)
{
    return rlCORE.Window.ready;
}

// Check if window is currently fullscreen
bool rlIsWindowFullscreen(void)
{
    return rlCORE.Window.fullscreen;
}

// Check if window is currently hidden
bool rlIsWindowHidden(void)
{
    return ((rlCORE.Window.flags & RL_FLAG_WINDOW_HIDDEN) > 0);
}

// Check if window has been minimized
bool rlIsWindowMinimized(void)
{
    return ((rlCORE.Window.flags & RL_FLAG_WINDOW_MINIMIZED) > 0);
}

// Check if window has been maximized
bool rlIsWindowMaximized(void)
{
    return ((rlCORE.Window.flags & RL_FLAG_WINDOW_MAXIMIZED) > 0);
}

// Check if window has the focus
bool rlIsWindowFocused(void)
{
    return ((rlCORE.Window.flags & RL_FLAG_WINDOW_UNFOCUSED) == 0);
}

// Check if window has been resizedLastFrame
bool rlIsWindowResized(void)
{
    return rlCORE.Window.resizedLastFrame;
}

// Check if one specific window flag is enabled
bool rlIsWindowState(unsigned int flag)
{
    return ((rlCORE.Window.flags & flag) > 0);
}

// Get current screen width
int rlGetScreenWidth(void)
{
    return rlCORE.Window.screen.width;
}

// Get current screen height
int rlGetScreenHeight(void)
{
    return rlCORE.Window.screen.height;
}

// Get current render width which is equal to screen width*dpi scale
int rlGetRenderWidth(void)
{
    int width = 0;
#if defined(__APPLE__)
    rlVector2 scale = rlGetWindowScaleDPI();
    width = (int)((float)rlCORE.Window.render.width*scale.x);
#else
    width = rlCORE.Window.render.width;
#endif
    return width;
}

// Get current screen height which is equal to screen height*dpi scale
int rlGetRenderHeight(void)
{
    int height = 0;
#if defined(__APPLE__)
    rlVector2 scale = rlGetWindowScaleDPI();
    height = (int)((float)rlCORE.Window.render.height*scale.y);
#else
    height = rlCORE.Window.render.height;
#endif
    return height;
}

// Enable waiting for events on rlEndDrawing(), no automatic event polling
void rlEnableEventWaiting(void)
{
    rlCORE.Window.eventWaiting = true;
}

// Disable waiting for events on rlEndDrawing(), automatic events polling
void rlDisableEventWaiting(void)
{
    rlCORE.Window.eventWaiting = false;
}

// Check if cursor is not visible
bool rlIsCursorHidden(void)
{
    return rlCORE.Input.Mouse.cursorHidden;
}

// Check if cursor is on the current screen.
bool rlIsCursorOnScreen(void)
{
    return rlCORE.Input.Mouse.cursorOnScreen;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Screen Drawing
//----------------------------------------------------------------------------------

// Set background color (framebuffer clear color)
void rlClearBackground(rlColor color)
{
    rlglClearColor(color.r, color.g, color.b, color.a);   // Set clear color
    rlglClearScreenBuffers();                             // Clear current framebuffers
}

// Setup canvas (framebuffer) to start drawing
void rlBeginDrawing(void)
{
    // WARNING: Previously to rlBeginDrawing() other render textures drawing could happen,
    // consequently the measure for update vs draw is not accurate (only the total frame time is accurate)

    rlCORE.Time.current = rlGetTime();      // Number of elapsed seconds since InitTimer()
    rlCORE.Time.update = rlCORE.Time.current - rlCORE.Time.previous;
    rlCORE.Time.previous = rlCORE.Time.current;

    rlglLoadIdentity();                   // Reset current matrix (modelview)
    rlglMultMatrixf(rlMatrixToFloat(rlCORE.Window.screenScale)); // Apply screen scaling

    //rlglTranslatef(0.375, 0.375, 0);    // HACK to have 2D pixel-perfect drawing on OpenGL 1.1
                                        // NOTE: Not required with OpenGL 3.3+
}

// End canvas drawing and swap buffers (double buffering)
void rlEndDrawing(void)
{
    rlglDrawRenderBatchActive();      // Update and draw internal render batch

#if defined(RL_SUPPORT_GIF_RECORDING)
    // Draw record indicator
    if (gifRecording)
    {
        #define GIF_RECORD_FRAMERATE    10
        gifFrameCounter++;

        // NOTE: We record one gif frame every 10 game frames
        if ((gifFrameCounter%GIF_RECORD_FRAMERATE) == 0)
        {
            // Get image data for the current frame (from backbuffer)
            // NOTE: This process is quite slow... :(
            rlVector2 scale = rlGetWindowScaleDPI();
            unsigned char *screenData = rlglReadScreenPixels((int)((float)rlCORE.Window.render.width*scale.x), (int)((float)rlCORE.Window.render.height*scale.y));
            msf_gif_frame(&gifState, screenData, 10, 16, (int)((float)rlCORE.Window.render.width*scale.x)*4);

            RL_FREE(screenData);    // Free image data
        }

    #if defined(RL_SUPPORT_MODULE_RSHAPES) && defined(RL_SUPPORT_MODULE_RTEXT)
        if (((gifFrameCounter/15)%2) == 1)
        {
            rlDrawCircle(30, rlCORE.Window.screen.height - 20, 10, RL_MAROON);                 // WARNING: Module required: rshapes
            rlDrawText("GIF RECORDING", 50, rlCORE.Window.screen.height - 25, 10, RL_RED);     // WARNING: Module required: rtext
        }
    #endif

        rlglDrawRenderBatchActive();  // Update and draw internal render batch
    }
#endif

#if defined(RL_SUPPORT_AUTOMATION_EVENTS)
    if (automationEventRecording) RecordAutomationEvent();    // Event recording
#endif

#if !defined(RL_SUPPORT_CUSTOM_FRAME_CONTROL)
    rlSwapScreenBuffer();                  // Copy back buffer to front buffer (screen)

    // Frame time control system
    rlCORE.Time.current = rlGetTime();
    rlCORE.Time.draw = rlCORE.Time.current - rlCORE.Time.previous;
    rlCORE.Time.previous = rlCORE.Time.current;

    rlCORE.Time.frame = rlCORE.Time.update + rlCORE.Time.draw;

    // Wait for some milliseconds...
    if (rlCORE.Time.frame < rlCORE.Time.target)
    {
        rlWaitTime(rlCORE.Time.target - rlCORE.Time.frame);

        rlCORE.Time.current = rlGetTime();
        double waitTime = rlCORE.Time.current - rlCORE.Time.previous;
        rlCORE.Time.previous = rlCORE.Time.current;

        rlCORE.Time.frame += waitTime;    // Total frame time: update + draw + wait
    }

    rlPollInputEvents();      // Poll user events (before next frame update)
#endif

#if defined(RL_SUPPORT_SCREEN_CAPTURE)
    if (rlIsKeyPressed(RL_KEY_F12))
    {
#if defined(RL_SUPPORT_GIF_RECORDING)
        if (rlIsKeyDown(RL_KEY_LEFT_CONTROL))
        {
            if (gifRecording)
            {
                gifRecording = false;

                MsfGifResult result = msf_gif_end(&gifState);

                rlSaveFileData(rlTextFormat("%s/screenrec%03i.gif", rlCORE.Storage.basePath, screenshotCounter), result.data, (unsigned int)result.dataSize);
                msf_gif_free(result);

                rlTRACELOG(RL_LOG_INFO, "SYSTEM: Finish animated GIF recording");
            }
            else
            {
                gifRecording = true;
                gifFrameCounter = 0;

                rlVector2 scale = rlGetWindowScaleDPI();
                msf_gif_begin(&gifState, (int)((float)rlCORE.Window.render.width*scale.x), (int)((float)rlCORE.Window.render.height*scale.y));
                screenshotCounter++;

                rlTRACELOG(RL_LOG_INFO, "SYSTEM: Start animated GIF recording: %s", rlTextFormat("screenrec%03i.gif", screenshotCounter));
            }
        }
        else
#endif  // RL_SUPPORT_GIF_RECORDING
        {
            rlTakeScreenshot(rlTextFormat("screenshot%03i.png", screenshotCounter));
            screenshotCounter++;
        }
    }
#endif  // RL_SUPPORT_SCREEN_CAPTURE

    rlCORE.Time.frameCounter++;
}

// Initialize 2D mode with custom camera (2D)
void rlBeginMode2D(rlCamera2D camera)
{
    rlglDrawRenderBatchActive();      // Update and draw internal render batch

    rlglLoadIdentity();               // Reset current matrix (modelview)

    // Apply 2d camera transformation to modelview
    rlglMultMatrixf(rlMatrixToFloat(rlGetCameraMatrix2D(camera)));

    // Apply screen scaling if required
    rlglMultMatrixf(rlMatrixToFloat(rlCORE.Window.screenScale));
}

// Ends 2D mode with custom camera
void rlEndMode2D(void)
{
    rlglDrawRenderBatchActive();      // Update and draw internal render batch

    rlglLoadIdentity();               // Reset current matrix (modelview)
    rlglMultMatrixf(rlMatrixToFloat(rlCORE.Window.screenScale)); // Apply screen scaling if required
}

// Initializes 3D mode with custom camera (3D)
void rlBeginMode3D(rlCamera camera)
{
    rlglDrawRenderBatchActive();      // Update and draw internal render batch

    rlglMatrixMode(RL_PROJECTION);    // Switch to projection matrix
    rlglPushMatrix();                 // Save previous matrix, which contains the settings for the 2d ortho projection
    rlglLoadIdentity();               // Reset current matrix (projection)

    float aspect = (float)rlCORE.Window.currentFbo.width/(float)rlCORE.Window.currentFbo.height;

    // NOTE: zNear and zFar values are important when computing depth buffer values
    if (camera.projection == RL_CAMERA_PERSPECTIVE)
    {
        // Setup perspective projection
        double top = RL_CULL_DISTANCE_NEAR*tan(camera.fovy*0.5*RL_DEG2RAD);
        double right = top*aspect;

        rlglFrustum(-right, right, -top, top, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    }
    else if (camera.projection == RL_CAMERA_ORTHOGRAPHIC)
    {
        // Setup orthographic projection
        double top = camera.fovy/2.0;
        double right = top*aspect;

        rlglOrtho(-right, right, -top,top, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    }

    rlglMatrixMode(RL_MODELVIEW);     // Switch back to modelview matrix
    rlglLoadIdentity();               // Reset current matrix (modelview)

    // Setup rlCamera view
    rlMatrix matView = rlMatrixLookAt(camera.position, camera.target, camera.up);
    rlglMultMatrixf(rlMatrixToFloat(matView));      // Multiply modelview matrix by view matrix (camera)

    rlglEnableDepthTest();            // Enable DEPTH_TEST for 3D
}

// Ends 3D mode and returns to default 2D orthographic mode
void rlEndMode3D(void)
{
    rlglDrawRenderBatchActive();      // Update and draw internal render batch

    rlglMatrixMode(RL_PROJECTION);    // Switch to projection matrix
    rlglPopMatrix();                  // Restore previous matrix (projection) from matrix stack

    rlglMatrixMode(RL_MODELVIEW);     // Switch back to modelview matrix
    rlglLoadIdentity();               // Reset current matrix (modelview)

    rlglMultMatrixf(rlMatrixToFloat(rlCORE.Window.screenScale)); // Apply screen scaling if required

    rlglDisableDepthTest();           // Disable DEPTH_TEST for 2D
}

// Initializes render texture for drawing
void rlBeginTextureMode(rlRenderTexture2D target)
{
    rlglDrawRenderBatchActive();      // Update and draw internal render batch

    rlglEnableFramebuffer(target.id); // Enable render target

    // Set viewport and RLGL internal framebuffer size
    rlglViewport(0, 0, target.texture.width, target.texture.height);
    rlglSetFramebufferWidth(target.texture.width);
    rlglSetFramebufferHeight(target.texture.height);

    rlglMatrixMode(RL_PROJECTION);    // Switch to projection matrix
    rlglLoadIdentity();               // Reset current matrix (projection)

    // Set orthographic projection to current framebuffer size
    // NOTE: Configured top-left corner as (0, 0)
    rlglOrtho(0, target.texture.width, target.texture.height, 0, 0.0f, 1.0f);

    rlglMatrixMode(RL_MODELVIEW);     // Switch back to modelview matrix
    rlglLoadIdentity();               // Reset current matrix (modelview)

    //rlglScalef(0.0f, -1.0f, 0.0f);  // Flip Y-drawing (?)

    // Setup current width/height for proper aspect ratio
    // calculation when using rlBeginMode3D()
    rlCORE.Window.currentFbo.width = target.texture.width;
    rlCORE.Window.currentFbo.height = target.texture.height;
    rlCORE.Window.usingFbo = true;
}

// Ends drawing to render texture
void rlEndTextureMode(void)
{
    rlglDrawRenderBatchActive();      // Update and draw internal render batch

    rlglDisableFramebuffer();         // Disable render target (fbo)

    // Set viewport to default framebuffer size
    SetupViewport(rlCORE.Window.render.width, rlCORE.Window.render.height);

    // Reset current fbo to screen size
    rlCORE.Window.currentFbo.width = rlCORE.Window.render.width;
    rlCORE.Window.currentFbo.height = rlCORE.Window.render.height;
    rlCORE.Window.usingFbo = false;
}

// Begin custom shader mode
void rlBeginShaderMode(rlShader shader)
{
    rlglSetShader(shader.id, shader.locs);
}

// End custom shader mode (returns to default shader)
void rlEndShaderMode(void)
{
    rlglSetShader(rlglGetShaderIdDefault(), rlglGetShaderLocsDefault());
}

// Begin blending mode (alpha, additive, multiplied, subtract, custom)
// NOTE: Blend modes supported are enumerated in rlBlendMode enum
void rlBeginBlendMode(int mode)
{
    rlglSetBlendMode(mode);
}

// End blending mode (reset to default: alpha blending)
void rlEndBlendMode(void)
{
    rlglSetBlendMode(RL_BLEND_ALPHA);
}

// Begin scissor mode (define screen area for following drawing)
// NOTE: Scissor rec refers to bottom-left corner, we change it to upper-left
void rlBeginScissorMode(int x, int y, int width, int height)
{
    rlglDrawRenderBatchActive();      // Update and draw internal render batch

    rlglEnableScissorTest();

#if defined(__APPLE__)
    if (!rlCORE.Window.usingFbo)
    {
        rlVector2 scale = rlGetWindowScaleDPI();
        rlglScissor((int)(x*scale.x), (int)(rlGetScreenHeight()*scale.y - (((y + height)*scale.y))), (int)(width*scale.x), (int)(height*scale.y));
    }
#else
    if (!rlCORE.Window.usingFbo && ((rlCORE.Window.flags & RL_FLAG_WINDOW_HIGHDPI) > 0))
    {
        rlVector2 scale = rlGetWindowScaleDPI();
        rlglScissor((int)(x*scale.x), (int)(rlCORE.Window.currentFbo.height - (y + height)*scale.y), (int)(width*scale.x), (int)(height*scale.y));
    }
#endif
    else
    {
        rlglScissor(x, rlCORE.Window.currentFbo.height - (y + height), width, height);
    }
}

// End scissor mode
void rlEndScissorMode(void)
{
    rlglDrawRenderBatchActive();      // Update and draw internal render batch
    rlglDisableScissorTest();
}

//----------------------------------------------------------------------------------
// Module Functions Definition: VR Stereo Rendering
//----------------------------------------------------------------------------------

// Begin VR drawing configuration
void rlBeginVrStereoMode(rlVrStereoConfig config)
{
    rlglEnableStereoRender();

    // Set stereo render matrices
    rlglSetMatrixProjectionStereo(config.projection[0], config.projection[1]);
    rlglSetMatrixViewOffsetStereo(config.viewOffset[0], config.viewOffset[1]);
}

// End VR drawing process (and desktop mirror)
void rlEndVrStereoMode(void)
{
    rlglDisableStereoRender();
}

// Load VR stereo config for VR simulator device parameters
rlVrStereoConfig rlLoadVrStereoConfig(rlVrDeviceInfo device)
{
    rlVrStereoConfig config = { 0 };

    if (rlglGetVersion() != RL_OPENGL_11)
    {
        // Compute aspect ratio
        float aspect = ((float)device.hResolution*0.5f)/(float)device.vResolution;

        // Compute lens parameters
        float lensShift = (device.hScreenSize*0.25f - device.lensSeparationDistance*0.5f)/device.hScreenSize;
        config.leftLensCenter[0] = 0.25f + lensShift;
        config.leftLensCenter[1] = 0.5f;
        config.rightLensCenter[0] = 0.75f - lensShift;
        config.rightLensCenter[1] = 0.5f;
        config.leftScreenCenter[0] = 0.25f;
        config.leftScreenCenter[1] = 0.5f;
        config.rightScreenCenter[0] = 0.75f;
        config.rightScreenCenter[1] = 0.5f;

        // Compute distortion scale parameters
        // NOTE: To get lens max radius, lensShift must be normalized to [-1..1]
        float lensRadius = fabsf(-1.0f - 4.0f*lensShift);
        float lensRadiusSq = lensRadius*lensRadius;
        float distortionScale = device.lensDistortionValues[0] +
                                device.lensDistortionValues[1]*lensRadiusSq +
                                device.lensDistortionValues[2]*lensRadiusSq*lensRadiusSq +
                                device.lensDistortionValues[3]*lensRadiusSq*lensRadiusSq*lensRadiusSq;

        float normScreenWidth = 0.5f;
        float normScreenHeight = 1.0f;
        config.scaleIn[0] = 2.0f/normScreenWidth;
        config.scaleIn[1] = 2.0f/normScreenHeight/aspect;
        config.scale[0] = normScreenWidth*0.5f/distortionScale;
        config.scale[1] = normScreenHeight*0.5f*aspect/distortionScale;

        // Fovy is normally computed with: 2*atan2f(device.vScreenSize, 2*device.eyeToScreenDistance)
        // ...but with lens distortion it is increased (see Oculus SDK Documentation)
        float fovy = 2.0f*atan2f(device.vScreenSize*0.5f*distortionScale, device.eyeToScreenDistance);     // Really need distortionScale?
       // float fovy = 2.0f*(float)atan2f(device.vScreenSize*0.5f, device.eyeToScreenDistance);

        // Compute camera projection matrices
        float projOffset = 4.0f*lensShift;      // Scaled to projection space coordinates [-1..1]
        rlMatrix proj = rlMatrixPerspective(fovy, aspect, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);

        config.projection[0] = rlMatrixMultiply(proj, rlMatrixTranslate(projOffset, 0.0f, 0.0f));
        config.projection[1] = rlMatrixMultiply(proj, rlMatrixTranslate(-projOffset, 0.0f, 0.0f));

        // Compute camera transformation matrices
        // NOTE: rlCamera movement might seem more natural if we model the head.
        // Our axis of rotation is the base of our head, so we might want to add
        // some y (base of head to eye level) and -z (center of head to eye protrusion) to the camera positions.
        config.viewOffset[0] = rlMatrixTranslate(-device.interpupillaryDistance*0.5f, 0.075f, 0.045f);
        config.viewOffset[1] = rlMatrixTranslate(device.interpupillaryDistance*0.5f, 0.075f, 0.045f);

        // Compute eyes Viewports
        /*
        config.eyeViewportRight[0] = 0;
        config.eyeViewportRight[1] = 0;
        config.eyeViewportRight[2] = device.hResolution/2;
        config.eyeViewportRight[3] = device.vResolution;

        config.eyeViewportLeft[0] = device.hResolution/2;
        config.eyeViewportLeft[1] = 0;
        config.eyeViewportLeft[2] = device.hResolution/2;
        config.eyeViewportLeft[3] = device.vResolution;
        */
    }
    else rlTRACELOG(RL_LOG_WARNING, "RLGL: VR Simulator not supported on OpenGL 1.1");

    return config;
}

// Unload VR stereo config properties
void rlUnloadVrStereoConfig(rlVrStereoConfig config)
{
    rlTRACELOG(RL_LOG_INFO, "rlUnloadVrStereoConfig not implemented in rcore.c");
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Shaders Management
//----------------------------------------------------------------------------------

// Load shader from files and bind default locations
// NOTE: If shader string is NULL, using default vertex/fragment shaders
rlShader rlLoadShader(const char *vsFileName, const char *fsFileName)
{
    rlShader shader = { 0 };

    char *vShaderStr = NULL;
    char *fShaderStr = NULL;

    if (vsFileName != NULL) vShaderStr = rlLoadFileText(vsFileName);
    if (fsFileName != NULL) fShaderStr = rlLoadFileText(fsFileName);

    shader = rlLoadShaderFromMemory(vShaderStr, fShaderStr);

    rlUnloadFileText(vShaderStr);
    rlUnloadFileText(fShaderStr);

    return shader;
}

// Load shader from code strings and bind default locations
rlShader rlLoadShaderFromMemory(const char *vsCode, const char *fsCode)
{
    rlShader shader = { 0 };

    shader.id = rlglLoadShaderCode(vsCode, fsCode);

    // After shader loading, we TRY to set default location names
    if (shader.id > 0)
    {
        // Default shader attribute locations have been binded before linking:
        //          vertex position location    = 0
        //          vertex texcoord location    = 1
        //          vertex normal location      = 2
        //          vertex color location       = 3
        //          vertex tangent location     = 4
        //          vertex texcoord2 location   = 5

        // NOTE: If any location is not found, loc point becomes -1

        shader.locs = (int *)RL_CALLOC(RL_MAX_SHADER_LOCATIONS, sizeof(int));

        // All locations reset to -1 (no location)
        for (int i = 0; i < RL_MAX_SHADER_LOCATIONS; i++) shader.locs[i] = -1;

        // Get handles to GLSL input attribute locations
        shader.locs[RL_SHADER_LOC_VERTEX_POSITION] = rlglGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION);
        shader.locs[RL_SHADER_LOC_VERTEX_TEXCOORD01] = rlglGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD);
        shader.locs[RL_SHADER_LOC_VERTEX_TEXCOORD02] = rlglGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2);
        shader.locs[RL_SHADER_LOC_VERTEX_NORMAL] = rlglGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL);
        shader.locs[RL_SHADER_LOC_VERTEX_TANGENT] = rlglGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT);
        shader.locs[RL_SHADER_LOC_VERTEX_COLOR] = rlglGetLocationAttrib(shader.id, RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR);

        // Get handles to GLSL uniform locations (vertex shader)
        shader.locs[RL_SHADER_LOC_MATRIX_MVP] = rlglGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_MVP);
        shader.locs[RL_SHADER_LOC_MATRIX_VIEW] = rlglGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW);
        shader.locs[RL_SHADER_LOC_MATRIX_PROJECTION] = rlglGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION);
        shader.locs[RL_SHADER_LOC_MATRIX_MODEL] = rlglGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL);
        shader.locs[RL_SHADER_LOC_MATRIX_NORMAL] = rlglGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL);

        // Get handles to GLSL uniform locations (fragment shader)
        shader.locs[RL_SHADER_LOC_COLOR_DIFFUSE] = rlglGetLocationUniform(shader.id, RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR);
        shader.locs[RL_SHADER_LOC_MAP_DIFFUSE] = rlglGetLocationUniform(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0);  // RL_SHADER_LOC_MAP_ALBEDO
        shader.locs[RL_SHADER_LOC_MAP_SPECULAR] = rlglGetLocationUniform(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1); // RL_SHADER_LOC_MAP_METALNESS
        shader.locs[RL_SHADER_LOC_MAP_NORMAL] = rlglGetLocationUniform(shader.id, RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2);
    }

    return shader;
}

// Check if a shader is ready
bool rlIsShaderReady(rlShader shader)
{
    return ((shader.id > 0) &&          // Validate shader id (loaded successfully)
            (shader.locs != NULL));     // Validate memory has been allocated for default shader locations

    // The following locations are tried to be set automatically (locs[i] >= 0),
    // any of them can be checked for validation but the only mandatory one is, afaik, RL_SHADER_LOC_VERTEX_POSITION
    // NOTE: Users can also setup manually their own attributes/uniforms and do not used the default raylib ones

    // Vertex shader attribute locations (default)
    // shader.locs[RL_SHADER_LOC_VERTEX_POSITION]      // Set by default internal shader
    // shader.locs[RL_SHADER_LOC_VERTEX_TEXCOORD01]    // Set by default internal shader
    // shader.locs[RL_SHADER_LOC_VERTEX_TEXCOORD02]
    // shader.locs[RL_SHADER_LOC_VERTEX_NORMAL]
    // shader.locs[RL_SHADER_LOC_VERTEX_TANGENT]
    // shader.locs[RL_SHADER_LOC_VERTEX_COLOR]         // Set by default internal shader

    // Vertex shader uniform locations (default)
    // shader.locs[RL_SHADER_LOC_MATRIX_MVP]           // Set by default internal shader
    // shader.locs[RL_SHADER_LOC_MATRIX_VIEW]
    // shader.locs[RL_SHADER_LOC_MATRIX_PROJECTION]
    // shader.locs[RL_SHADER_LOC_MATRIX_MODEL]
    // shader.locs[RL_SHADER_LOC_MATRIX_NORMAL]

    // Fragment shader uniform locations (default)
    // shader.locs[RL_SHADER_LOC_COLOR_DIFFUSE]        // Set by default internal shader
    // shader.locs[RL_SHADER_LOC_MAP_DIFFUSE]          // Set by default internal shader
    // shader.locs[RL_SHADER_LOC_MAP_SPECULAR]
    // shader.locs[RL_SHADER_LOC_MAP_NORMAL]
}

// Unload shader from GPU memory (VRAM)
void rlUnloadShader(rlShader shader)
{
    if (shader.id != rlglGetShaderIdDefault())
    {
        rlglUnloadShaderProgram(shader.id);

        // NOTE: If shader loading failed, it should be 0
        RL_FREE(shader.locs);
    }
}

// Get shader uniform location
int rlGetShaderLocation(rlShader shader, const char *uniformName)
{
    return rlglGetLocationUniform(shader.id, uniformName);
}

// Get shader attribute location
int rlGetShaderLocationAttrib(rlShader shader, const char *attribName)
{
    return rlglGetLocationAttrib(shader.id, attribName);
}

// Set shader uniform value
void rlSetShaderValue(rlShader shader, int locIndex, const void *value, int uniformType)
{
    rlSetShaderValueV(shader, locIndex, value, uniformType, 1);
}

// Set shader uniform value vector
void rlSetShaderValueV(rlShader shader, int locIndex, const void *value, int uniformType, int count)
{
    if (locIndex > -1)
    {
        rlglEnableShader(shader.id);
        rlglSetUniform(locIndex, value, uniformType, count);
        //rlglDisableShader();      // Avoid resetting current shader program, in case other uniforms are set
    }
}

// Set shader uniform value (matrix 4x4)
void rlSetShaderValueMatrix(rlShader shader, int locIndex, rlMatrix mat)
{
    if (locIndex > -1)
    {
        rlglEnableShader(shader.id);
        rlglSetUniformMatrix(locIndex, mat);
        //rlglDisableShader();
    }
}

// Set shader uniform value for texture
void rlSetShaderValueTexture(rlShader shader, int locIndex, rlTexture2D texture)
{
    if (locIndex > -1)
    {
        rlglEnableShader(shader.id);
        rlglSetUniformSampler(locIndex, texture.id);
        //rlglDisableShader();
    }
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Screen-space Queries
//----------------------------------------------------------------------------------

// Get a ray trace from mouse position
rlRay rlGetMouseRay(rlVector2 mouse, rlCamera camera)
{
    rlRay ray = { 0 };

    // Calculate normalized device coordinates
    // NOTE: y value is negative
    float x = (2.0f*mouse.x)/(float)rlGetScreenWidth() - 1.0f;
    float y = 1.0f - (2.0f*mouse.y)/(float)rlGetScreenHeight();
    float z = 1.0f;

    // Store values in a vector
    rlVector3 deviceCoords = { x, y, z };

    // Calculate view matrix from camera look at
    rlMatrix matView = rlMatrixLookAt(camera.position, camera.target, camera.up);

    rlMatrix matProj = rlMatrixIdentity();

    if (camera.projection == RL_CAMERA_PERSPECTIVE)
    {
        // Calculate projection matrix from perspective
        matProj = rlMatrixPerspective(camera.fovy*RL_DEG2RAD, ((double)rlGetScreenWidth()/(double)rlGetScreenHeight()), RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    }
    else if (camera.projection == RL_CAMERA_ORTHOGRAPHIC)
    {
        double aspect = (double)rlCORE.Window.screen.width/(double)rlCORE.Window.screen.height;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate projection matrix from orthographic
        matProj = rlMatrixOrtho(-right, right, -top, top, 0.01, 1000.0);
    }

    // Unproject far/near points
    rlVector3 nearPoint = rlVector3Unproject(CAST(rlVector3){ deviceCoords.x, deviceCoords.y, 0.0f }, matProj, matView);
    rlVector3 farPoint = rlVector3Unproject(CAST(rlVector3){ deviceCoords.x, deviceCoords.y, 1.0f }, matProj, matView);

    // Unproject the mouse cursor in the near plane.
    // We need this as the source position because orthographic projects, compared to perspective doesn't have a
    // convergence point, meaning that the "eye" of the camera is more like a plane than a point.
    rlVector3 cameraPlanePointerPos = rlVector3Unproject(CAST(rlVector3){ deviceCoords.x, deviceCoords.y, -1.0f }, matProj, matView);

    // Calculate normalized direction vector
    rlVector3 direction = rlVector3Normalize(rlVector3Subtract(farPoint, nearPoint));

    if (camera.projection == RL_CAMERA_PERSPECTIVE) ray.position = camera.position;
    else if (camera.projection == RL_CAMERA_ORTHOGRAPHIC) ray.position = cameraPlanePointerPos;

    // Apply calculated vectors to ray
    ray.direction = direction;

    return ray;
}

// Get transform matrix for camera
rlMatrix rlGetCameraMatrix(rlCamera camera)
{
    return rlMatrixLookAt(camera.position, camera.target, camera.up);
}

// Get camera 2d transform matrix
rlMatrix rlGetCameraMatrix2D(rlCamera2D camera)
{
    rlMatrix matTransform = { 0 };
    // The camera in world-space is set by
    //   1. Move it to target
    //   2. Rotate by -rotation and scale by (1/zoom)
    //      When setting higher scale, it's more intuitive for the world to become bigger (= camera become smaller),
    //      not for the camera getting bigger, hence the invert. Same deal with rotation.
    //   3. Move it by (-offset);
    //      Offset defines target transform relative to screen, but since we're effectively "moving" screen (camera)
    //      we need to do it into opposite direction (inverse transform)

    // Having camera transform in world-space, inverse of it gives the modelview transform.
    // Since (A*B*C)' = C'*B'*A', the modelview is
    //   1. Move to offset
    //   2. Rotate and Scale
    //   3. Move by -target
    rlMatrix matOrigin = rlMatrixTranslate(-camera.target.x, -camera.target.y, 0.0f);
    rlMatrix matRotation = rlMatrixRotate(CAST(rlVector3){ 0.0f, 0.0f, 1.0f }, camera.rotation*RL_DEG2RAD);
    rlMatrix matScale = rlMatrixScale(camera.zoom, camera.zoom, 1.0f);
    rlMatrix matTranslation = rlMatrixTranslate(camera.offset.x, camera.offset.y, 0.0f);

    matTransform = rlMatrixMultiply(rlMatrixMultiply(matOrigin, rlMatrixMultiply(matScale, matRotation)), matTranslation);

    return matTransform;
}

// Get the screen space position from a 3d world space position
rlVector2 rlGetWorldToScreen(rlVector3 position, rlCamera camera)
{
    rlVector2 screenPosition = rlGetWorldToScreenEx(position, camera, rlGetScreenWidth(), rlGetScreenHeight());

    return screenPosition;
}

// Get size position for a 3d world space position (useful for texture drawing)
rlVector2 rlGetWorldToScreenEx(rlVector3 position, rlCamera camera, int width, int height)
{
    // Calculate projection matrix (from perspective instead of frustum
    rlMatrix matProj = rlMatrixIdentity();

    if (camera.projection == RL_CAMERA_PERSPECTIVE)
    {
        // Calculate projection matrix from perspective
        matProj = rlMatrixPerspective(camera.fovy*RL_DEG2RAD, ((double)width/(double)height), RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    }
    else if (camera.projection == RL_CAMERA_ORTHOGRAPHIC)
    {
        double aspect = (double)width/(double)height;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate projection matrix from orthographic
        matProj = rlMatrixOrtho(-right, right, -top, top, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    }

    // Calculate view matrix from camera look at (and transpose it)
    rlMatrix matView = rlMatrixLookAt(camera.position, camera.target, camera.up);

    // TODO: Why not use rlVector3Transform(rlVector3 v, rlMatrix mat)?

    // Convert world position vector to quaternion
    rlQuaternion worldPos = { position.x, position.y, position.z, 1.0f };

    // rlTransform world position to view
    worldPos = rlQuaternionTransform(worldPos, matView);

    // rlTransform result to projection (clip space position)
    worldPos = rlQuaternionTransform(worldPos, matProj);

    // Calculate normalized device coordinates (inverted y)
    rlVector3 ndcPos = { worldPos.x/worldPos.w, -worldPos.y/worldPos.w, worldPos.z/worldPos.w };

    // Calculate 2d screen position vector
    rlVector2 screenPosition = { (ndcPos.x + 1.0f)/2.0f*(float)width, (ndcPos.y + 1.0f)/2.0f*(float)height };

    return screenPosition;
}

// Get the screen space position for a 2d camera world space position
rlVector2 rlGetWorldToScreen2D(rlVector2 position, rlCamera2D camera)
{
    rlMatrix matCamera = rlGetCameraMatrix2D(camera);
    rlVector3 transform = rlVector3Transform(CAST(rlVector3){ position.x, position.y, 0 }, matCamera);

    return CAST(rlVector2){ transform.x, transform.y };
}

// Get the world space position for a 2d camera screen space position
rlVector2 rlGetScreenToWorld2D(rlVector2 position, rlCamera2D camera)
{
    rlMatrix invMatCamera = rlMatrixInvert(rlGetCameraMatrix2D(camera));
    rlVector3 transform = rlVector3Transform( CAST(rlVector3){ position.x, position.y, 0 }, invMatCamera);

    return CAST(rlVector2){ transform.x, transform.y };
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Timming
//----------------------------------------------------------------------------------

// NOTE: Functions with a platform-specific implementation on rcore_<platform>.c
//double rlGetTime(void)

// Set target FPS (maximum)
void rlSetTargetFPS(int fps)
{
    if (fps < 1) rlCORE.Time.target = 0.0;
    else rlCORE.Time.target = 1.0/(double)fps;

    rlTRACELOG(RL_LOG_INFO, "TIMER: Target time per frame: %02.03f milliseconds", (float)rlCORE.Time.target*1000.0f);
}

// Get current FPS
// NOTE: We calculate an average framerate
int rlGetFPS(void)
{
    int fps = 0;

#if !defined(RL_SUPPORT_CUSTOM_FRAME_CONTROL)
    #define FPS_CAPTURE_FRAMES_COUNT    30      // 30 captures
    #define FPS_AVERAGE_TIME_SECONDS   0.5f     // 500 milliseconds
    #define FPS_STEP (FPS_AVERAGE_TIME_SECONDS/FPS_CAPTURE_FRAMES_COUNT)

    static int index = 0;
    static float history[FPS_CAPTURE_FRAMES_COUNT] = { 0 };
    static float average = 0, last = 0;
    float fpsFrame = rlGetFrameTime();

    // if we reset the window, reset the FPS info
    if (rlCORE.Time.frameCounter == 0)
    {
        average = 0;
        last = 0;
        index = 0;

        for (int i = 0; i < FPS_CAPTURE_FRAMES_COUNT; i++) history[i] = 0;
    }

    if (fpsFrame == 0) return 0;

    if ((rlGetTime() - last) > FPS_STEP)
    {
        last = (float)rlGetTime();
        index = (index + 1)%FPS_CAPTURE_FRAMES_COUNT;
        average -= history[index];
        history[index] = fpsFrame/FPS_CAPTURE_FRAMES_COUNT;
        average += history[index];
    }

    fps = (int)roundf(1.0f/average);
#endif

    return fps;
}

// Get time in seconds for last frame drawn (delta time)
float rlGetFrameTime(void)
{
    return (float)rlCORE.Time.frame;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Custom frame control
//----------------------------------------------------------------------------------

// NOTE: Functions with a platform-specific implementation on rcore_<platform>.c
//void rlSwapScreenBuffer(void);
//void rlPollInputEvents(void);

// Wait for some time (stop program execution)
// NOTE: Sleep() granularity could be around 10 ms, it means, Sleep() could
// take longer than expected... for that reason we use the busy wait loop
// Ref: http://stackoverflow.com/questions/43057578/c-programming-win32-games-sleep-taking-longer-than-expected
// Ref: http://www.geisswerks.com/ryan/FAQS/timing.html --> All about timing on Win32!
void rlWaitTime(double seconds)
{
    if (seconds < 0) return;

#if defined(RL_SUPPORT_BUSY_WAIT_LOOP) || defined(RL_SUPPORT_PARTIALBUSY_WAIT_LOOP)
    double destinationTime = rlGetTime() + seconds;
#endif

#if defined(RL_SUPPORT_BUSY_WAIT_LOOP)
    while (rlGetTime() < destinationTime) { }
#else
    #if defined(RL_SUPPORT_PARTIALBUSY_WAIT_LOOP)
        double sleepSeconds = seconds - seconds*0.05;  // NOTE: We reserve a percentage of the time for busy waiting
    #else
        double sleepSeconds = seconds;
    #endif

    // System halt functions
    #if defined(_WIN32)
        Sleep((unsigned long)(sleepSeconds*1000.0));
    #endif
    #if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__EMSCRIPTEN__)
        struct timespec req = { 0 };
        time_t sec = sleepSeconds;
        long nsec = (sleepSeconds - sec)*1000000000L;
        req.tv_sec = sec;
        req.tv_nsec = nsec;

        // NOTE: Use nanosleep() on Unix platforms... usleep() it's deprecated.
        while (nanosleep(&req, &req) == -1) continue;
    #endif
    #if defined(__APPLE__)
        usleep(sleepSeconds*1000000.0);
    #endif

    #if defined(RL_SUPPORT_PARTIALBUSY_WAIT_LOOP)
        while (rlGetTime() < destinationTime) { }
    #endif
#endif
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

// NOTE: Functions with a platform-specific implementation on rcore_<platform>.c
//void rlOpenURL(const char *url)


// Set the seed for the random number generator
void rlSetRandomSeed(unsigned int seed)
{
#if defined(RL_SUPPORT_RPRAND_GENERATOR)
    rprand_set_seed(seed);
#else
    srand(seed);
#endif
}

// Get a random value between min and max included
int rlGetRandomValue(int min, int max)
{
    int value = 0;

    if (min > max)
    {
        int tmp = max;
        max = min;
        min = tmp;
    }

#if defined(RL_SUPPORT_RPRAND_GENERATOR)
    value = rprand_get_value(min, max);
#else
    // WARNING: Ranges higher than RAND_MAX will return invalid results
    // More specifically, if (max - min) > INT_MAX there will be an overflow,
    // and otherwise if (max - min) > RAND_MAX the random value will incorrectly never exceed a certain threshold
    // NOTE: Depending on the library it can be as low as 32767
    if ((unsigned int)(max - min) > (unsigned int)RAND_MAX)
    {
        rlTRACELOG(RL_LOG_WARNING, "Invalid rlGetRandomValue() arguments, range should not be higher than %i", RAND_MAX);
    }

    value = (rand()%(abs(max - min) + 1) + min);
#endif
    return value;
}

// Load random values sequence, no values repeated, min and max included
int *LoadRandomSequence(unsigned int count, int min, int max)
{
    int *values = NULL;

#if defined(RL_SUPPORT_RPRAND_GENERATOR)
    values = rprand_load_sequence(count, min, max);
#else
    if (count > ((unsigned int)abs(max - min) + 1)) return values;

    values = (int *)RL_CALLOC(count, sizeof(int));

    int value = 0;
    bool dupValue = false;

    for (int i = 0; i < (int)count;)
    {
        value = (rand()%(abs(max - min) + 1) + min);
        dupValue = false;

        for (int j = 0; j < i; j++)
        {
            if (values[j] == value)
            {
                dupValue = true;
                break;
            }
        }

        if (!dupValue)
        {
            values[i] = value;
            i++;
        }
    }
#endif
    return values;
}

// Unload random values sequence
void UnloadRandomSequence(int *sequence)
{
#if defined(RL_SUPPORT_RPRAND_GENERATOR)
    rprand_unload_sequence(sequence);
#else
    RL_FREE(sequence);
#endif
}

// Takes a screenshot of current screen
// NOTE: Provided fileName should not contain paths, saving to working directory
void rlTakeScreenshot(const char *fileName)
{
#if defined(RL_SUPPORT_MODULE_RTEXTURES)
    // Security check to (partially) avoid malicious code
    if (strchr(fileName, '\'') != NULL) { rlTRACELOG(RL_LOG_WARNING, "SYSTEM: Provided fileName could be potentially malicious, avoid [\'] character"); return; }

    rlVector2 scale = rlGetWindowScaleDPI();
    unsigned char *imgData = rlglReadScreenPixels((int)((float)rlCORE.Window.render.width*scale.x), (int)((float)rlCORE.Window.render.height*scale.y));
    rlImage image = { imgData, (int)((float)rlCORE.Window.render.width*scale.x), (int)((float)rlCORE.Window.render.height*scale.y), 1, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };

    char path[512] = { 0 };
    strcpy(path, rlTextFormat("%s/%s", rlCORE.Storage.basePath, rlGetFileName(fileName)));

    rlExportImage(image, path);           // WARNING: Module required: rtextures
    RL_FREE(imgData);

    if (rlFileExists(path)) rlTRACELOG(RL_LOG_INFO, "SYSTEM: [%s] Screenshot taken successfully", path);
    else rlTRACELOG(RL_LOG_WARNING, "SYSTEM: [%s] Screenshot could not be saved", path);
#else
    rlTRACELOG(RL_LOG_WARNING,"IMAGE: rlExportImage() requires module: rtextures");
#endif
}

// Setup window configuration flags (view FLAGS)
// NOTE: This function is expected to be called before window creation,
// because it sets up some flags for the window creation process.
// To configure window states after creation, just use rlSetWindowState()
void rlSetConfigFlags(unsigned int flags)
{
    // Selected flags are set but not evaluated at this point,
    // flag evaluation happens at rlInitWindow() or rlSetWindowState()
    rlCORE.Window.flags |= flags;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: File system
//----------------------------------------------------------------------------------

// Check if the file exists
bool rlFileExists(const char *fileName)
{
    bool result = false;

#if defined(_WIN32)
    if (_access(fileName, 0) != -1) result = true;
#else
    if (access(fileName, F_OK) != -1) result = true;
#endif

    // NOTE: Alternatively, stat() can be used instead of access()
    //#include <sys/stat.h>
    //struct stat statbuf;
    //if (stat(filename, &statbuf) == 0) result = true;

    return result;
}

// Check file extension
// NOTE: Extensions checking is not case-sensitive
bool rlIsFileExtension(const char *fileName, const char *ext)
{
    #define RL_MAX_FILE_EXTENSION_SIZE  16

    bool result = false;
    const char *fileExt = rlGetFileExtension(fileName);

    if (fileExt != NULL)
    {
#if defined(RL_SUPPORT_MODULE_RTEXT) && defined(RL_SUPPORT_TEXT_MANIPULATION)
        int extCount = 0;
        const char **checkExts = TextSplit(ext, ';', &extCount); // WARNING: Module required: rtext

        char fileExtLower[RL_MAX_FILE_EXTENSION_SIZE + 1] = { 0 };
        strncpy(fileExtLower, rlTextToLower(fileExt), RL_MAX_FILE_EXTENSION_SIZE); // WARNING: Module required: rtext

        for (int i = 0; i < extCount; i++)
        {
            if (strcmp(fileExtLower, rlTextToLower(checkExts[i])) == 0)
            {
                result = true;
                break;
            }
        }
#else
        if (strcmp(fileExt, ext) == 0) result = true;
#endif
    }

    return result;
}

// Check if a directory path exists
bool rlDirectoryExists(const char *dirPath)
{
    bool result = false;
    DIR *dir = opendir(dirPath);

    if (dir != NULL)
    {
        result = true;
        closedir(dir);
    }

    return result;
}

// Get file length in bytes
// NOTE: GetFileSize() conflicts with windows.h
int rlGetFileLength(const char *fileName)
{
    int size = 0;

    // NOTE: On Unix-like systems, it can by used the POSIX system call: stat(),
    // but depending on the platform that call could not be available
    //struct stat result = { 0 };
    //stat(fileName, &result);
    //return result.st_size;

    FILE *file = fopen(fileName, "rb");

    if (file != NULL)
    {
        fseek(file, 0L, SEEK_END);
        long int fileSize = ftell(file);

        // Check for size overflow (INT_MAX)
        if (fileSize > 2147483647) rlTRACELOG(RL_LOG_WARNING, "[%s] File size overflows expected limit, do not use rlGetFileLength()", fileName);
        else size = (int)fileSize;

        fclose(file);
    }

    return size;
}

// Get pointer to extension for a filename string (includes the dot: .png)
const char *rlGetFileExtension(const char *fileName)
{
    const char *dot = strrchr(fileName, '.');

    if (!dot || dot == fileName) return NULL;

    return dot;
}

// String pointer reverse break: returns right-most occurrence of charset in s
static const char *strprbrk(const char *s, const char *charset)
{
    const char *latestMatch = NULL;
    for (; s = strpbrk(s, charset), s != NULL; latestMatch = s++) { }
    return latestMatch;
}

// Get pointer to filename for a path string
const char *rlGetFileName(const char *filePath)
{
    const char *fileName = NULL;
    if (filePath != NULL) fileName = strprbrk(filePath, "\\/");

    if (!fileName) return filePath;

    return fileName + 1;
}

// Get filename string without extension (uses static string)
const char *rlGetFileNameWithoutExt(const char *filePath)
{
    #define RL_MAX_FILENAMEWITHOUTEXT_LENGTH   256

    static char fileName[RL_MAX_FILENAMEWITHOUTEXT_LENGTH] = { 0 };
    memset(fileName, 0, RL_MAX_FILENAMEWITHOUTEXT_LENGTH);

    if (filePath != NULL) strcpy(fileName, rlGetFileName(filePath));   // Get filename with extension

    int size = (int)strlen(fileName);   // Get size in bytes

    for (int i = 0; (i < size) && (i < RL_MAX_FILENAMEWITHOUTEXT_LENGTH); i++)
    {
        if (fileName[i] == '.')
        {
            // NOTE: We break on first '.' found
            fileName[i] = '\0';
            break;
        }
    }

    return fileName;
}

// Get directory for a given filePath
const char *rlGetDirectoryPath(const char *filePath)
{
    /*
    // NOTE: Directory separator is different in Windows and other platforms,
    // fortunately, Windows also support the '/' separator, that's the one should be used
    #if defined(_WIN32)
        char separator = '\\';
    #else
        char separator = '/';
    #endif
    */
    const char *lastSlash = NULL;
    static char dirPath[RL_MAX_FILEPATH_LENGTH] = { 0 };
    memset(dirPath, 0, RL_MAX_FILEPATH_LENGTH);

    // In case provided path does not contain a root drive letter (C:\, D:\) nor leading path separator (\, /),
    // we add the current directory path to dirPath
    if (filePath[1] != ':' && filePath[0] != '\\' && filePath[0] != '/')
    {
        // For security, we set starting path to current directory,
        // obtained path will be concatenated to this
        dirPath[0] = '.';
        dirPath[1] = '/';
    }

    lastSlash = strprbrk(filePath, "\\/");
    if (lastSlash)
    {
        if (lastSlash == filePath)
        {
            // The last and only slash is the leading one: path is in a root directory
            dirPath[0] = filePath[0];
            dirPath[1] = '\0';
        }
        else
        {
            // NOTE: Be careful, strncpy() is not safe, it does not care about '\0'
            char *dirPathPtr = dirPath;
            if ((filePath[1] != ':') && (filePath[0] != '\\') && (filePath[0] != '/')) dirPathPtr += 2;     // Skip drive letter, "C:"
            memcpy(dirPathPtr, filePath, strlen(filePath) - (strlen(lastSlash) - 1));
            dirPath[strlen(filePath) - strlen(lastSlash) + (((filePath[1] != ':') && (filePath[0] != '\\') && (filePath[0] != '/'))? 2 : 0)] = '\0';  // Add '\0' manually
        }
    }

    return dirPath;
}

// Get previous directory path for a given path
const char *rlGetPrevDirectoryPath(const char *dirPath)
{
    static char prevDirPath[RL_MAX_FILEPATH_LENGTH] = { 0 };
    memset(prevDirPath, 0, RL_MAX_FILEPATH_LENGTH);
    int pathLen = (int)strlen(dirPath);

    if (pathLen <= 3) strcpy(prevDirPath, dirPath);

    for (int i = (pathLen - 1); (i >= 0) && (pathLen > 3); i--)
    {
        if ((dirPath[i] == '\\') || (dirPath[i] == '/'))
        {
            // Check for root: "C:\" or "/"
            if (((i == 2) && (dirPath[1] ==':')) || (i == 0)) i++;

            strncpy(prevDirPath, dirPath, i);
            break;
        }
    }

    return prevDirPath;
}

// Get current working directory
const char *rlGetWorkingDirectory(void)
{
    static char currentDir[RL_MAX_FILEPATH_LENGTH] = { 0 };
    memset(currentDir, 0, RL_MAX_FILEPATH_LENGTH);

    char *path = GETCWD(currentDir, RL_MAX_FILEPATH_LENGTH - 1);

    return path;
}

const char *rlGetApplicationDirectory(void)
{
    static char appDir[RL_MAX_FILEPATH_LENGTH] = { 0 };
    memset(appDir, 0, RL_MAX_FILEPATH_LENGTH);

#if defined(_WIN32)
    int len = 0;
#if defined(UNICODE)
    unsigned short widePath[RL_MAX_PATH];
    len = GetModuleFileNameW(NULL, widePath, RL_MAX_PATH);
    len = WideCharToMultiByte(0, 0, widePath, len, appDir, RL_MAX_PATH, NULL, NULL);
#else
    len = GetModuleFileNameA(NULL, appDir, RL_MAX_PATH);
#endif
    if (len > 0)
    {
        for (int i = len; i >= 0; --i)
        {
            if (appDir[i] == '\\')
            {
                appDir[i + 1] = '\0';
                break;
            }
        }
    }
    else
    {
        appDir[0] = '.';
        appDir[1] = '\\';
    }

#elif defined(__linux__)
    unsigned int size = sizeof(appDir);
    ssize_t len = readlink("/proc/self/exe", appDir, size);

    if (len > 0)
    {
        for (int i = len; i >= 0; --i)
        {
            if (appDir[i] == '/')
            {
                appDir[i + 1] = '\0';
                break;
            }
        }
    }
    else
    {
        appDir[0] = '.';
        appDir[1] = '/';
    }
#elif defined(__APPLE__)
    uint32_t size = sizeof(appDir);

    if (_NSGetExecutablePath(appDir, &size) == 0)
    {
        int len = strlen(appDir);
        for (int i = len; i >= 0; --i)
        {
            if (appDir[i] == '/')
            {
                appDir[i + 1] = '\0';
                break;
            }
        }
    }
    else
    {
        appDir[0] = '.';
        appDir[1] = '/';
    }
#endif

    return appDir;
}

// Load directory filepaths
// NOTE: Base path is prepended to the scanned filepaths
// WARNING: Directory is scanned twice, first time to get files count
// No recursive scanning is done!
rlFilePathList rlLoadDirectoryFiles(const char *dirPath)
{
    rlFilePathList files = { 0 };
    unsigned int fileCounter = 0;

    struct dirent *entity;
    DIR *dir = opendir(dirPath);

    if (dir != NULL) // It's a directory
    {
        // SCAN 1: Count files
        while ((entity = readdir(dir)) != NULL)
        {
            // NOTE: We skip '.' (current dir) and '..' (parent dir) filepaths
            if ((strcmp(entity->d_name, ".") != 0) && (strcmp(entity->d_name, "..") != 0)) fileCounter++;
        }

        // Memory allocation for dirFileCount
        files.capacity = fileCounter;
        files.paths = (char **)RL_MALLOC(files.capacity*sizeof(char *));
        for (unsigned int i = 0; i < files.capacity; i++) files.paths[i] = (char *)RL_MALLOC(RL_MAX_FILEPATH_LENGTH*sizeof(char));

        closedir(dir);

        // SCAN 2: Read filepaths
        // NOTE: Directory paths are also registered
        ScanDirectoryFiles(dirPath, &files, NULL);

        // Security check: read files.count should match fileCounter
        if (files.count != files.capacity) rlTRACELOG(RL_LOG_WARNING, "FILEIO: Read files count do not match capacity allocated");
    }
    else rlTRACELOG(RL_LOG_WARNING, "FILEIO: Failed to open requested directory");  // Maybe it's a file...

    return files;
}

// Load directory filepaths with extension filtering and recursive directory scan
// NOTE: On recursive loading we do not pre-scan for file count, we use RL_MAX_FILEPATH_CAPACITY
rlFilePathList rlLoadDirectoryFilesEx(const char *basePath, const char *filter, bool scanSubdirs)
{
    rlFilePathList files = { 0 };

    files.capacity = RL_MAX_FILEPATH_CAPACITY;
    files.paths = (char **)RL_CALLOC(files.capacity, sizeof(char *));
    for (unsigned int i = 0; i < files.capacity; i++) files.paths[i] = (char *)RL_CALLOC(RL_MAX_FILEPATH_LENGTH, sizeof(char));

    // WARNING: basePath is always prepended to scanned paths
    if (scanSubdirs) ScanDirectoryFilesRecursively(basePath, &files, filter);
    else ScanDirectoryFiles(basePath, &files, filter);

    return files;
}

// Unload directory filepaths
// WARNING: files.count is not reseted to 0 after unloading
void rlUnloadDirectoryFiles(rlFilePathList files)
{
    for (unsigned int i = 0; i < files.capacity; i++) RL_FREE(files.paths[i]);

    RL_FREE(files.paths);
}

// Change working directory, returns true on success
bool rlChangeDirectory(const char *dir)
{
    bool result = CHDIR(dir);

    if (result != 0) rlTRACELOG(RL_LOG_WARNING, "SYSTEM: Failed to change to directory: %s", dir);

    return (result == 0);
}

// Check if a given path point to a file
bool rlIsPathFile(const char *path)
{
    struct stat result = { 0 };
    stat(path, &result);

    return S_ISREG(result.st_mode);
}

// Check if a file has been dropped into window
bool rlIsFileDropped(void)
{
    if (rlCORE.Window.dropFileCount > 0) return true;
    else return false;
}

// Load dropped filepaths
rlFilePathList rlLoadDroppedFiles(void)
{
    rlFilePathList files = { 0 };

    files.count = rlCORE.Window.dropFileCount;
    files.paths = rlCORE.Window.dropFilepaths;

    return files;
}

// Unload dropped filepaths
void rlUnloadDroppedFiles(rlFilePathList files)
{
    // WARNING: files pointers are the same as internal ones

    if (files.count > 0)
    {
        for (unsigned int i = 0; i < files.count; i++) RL_FREE(files.paths[i]);

        RL_FREE(files.paths);

        rlCORE.Window.dropFileCount = 0;
        rlCORE.Window.dropFilepaths = NULL;
    }
}

// Get file modification time (last write time)
long rlGetFileModTime(const char *fileName)
{
    struct stat result = { 0 };

    if (stat(fileName, &result) == 0)
    {
        time_t mod = result.st_mtime;

        return (long)mod;
    }

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Compression and Encoding
//----------------------------------------------------------------------------------

// Compress data (DEFLATE algorithm)
unsigned char *rlCompressData(const unsigned char *data, int dataSize, int *compDataSize)
{
    #define COMPRESSION_QUALITY_DEFLATE  8

    unsigned char *compData = NULL;

#if defined(RL_SUPPORT_COMPRESSION_API)
    // Compress data and generate a valid DEFLATE stream
    struct sdefl *sdefl = (struct sdefl*)RL_CALLOC(1, sizeof(struct sdefl));   // WARNING: Possible stack overflow, struct sdefl is almost 1MB
    int bounds = sdefl_bound(dataSize);
    compData = (unsigned char *)RL_CALLOC(bounds, 1);

    *compDataSize = sdeflate(sdefl, compData, data, dataSize, COMPRESSION_QUALITY_DEFLATE);   // Compression level 8, same as stbiw
    RL_FREE(sdefl);

    rlTRACELOG(RL_LOG_INFO, "SYSTEM: Compress data: Original size: %i -> Comp. size: %i", dataSize, *compDataSize);
#endif

    return compData;
}

// Decompress data (DEFLATE algorithm)
unsigned char *rlDecompressData(const unsigned char *compData, int compDataSize, int *dataSize)
{
    unsigned char *data = NULL;

#if defined(RL_SUPPORT_COMPRESSION_API)
    // Decompress data from a valid DEFLATE stream
    data = (unsigned char *)RL_CALLOC(RL_MAX_DECOMPRESSION_SIZE*1024*1024, 1);
    int length = sinflate(data, RL_MAX_DECOMPRESSION_SIZE*1024*1024, compData, compDataSize);

    // WARNING: RL_REALLOC can make (and leave) data copies in memory, be careful with sensitive compressed data!
    // TODO: Use a different approach, create another buffer, copy data manually to it and wipe original buffer memory
    unsigned char *temp = (unsigned char *)RL_REALLOC(data, length);

    if (temp != NULL) data = temp;
    else rlTRACELOG(RL_LOG_WARNING, "SYSTEM: Failed to re-allocate required decompression memory");

    *dataSize = length;

    rlTRACELOG(RL_LOG_INFO, "SYSTEM: Decompress data: Comp. size: %i -> Original size: %i", compDataSize, *dataSize);
#endif

    return data;
}

// Encode data to Base64 string
char *rlEncodeDataBase64(const unsigned char *data, int dataSize, int *outputSize)
{
    static const unsigned char base64encodeTable[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
    };

    static const int modTable[] = { 0, 2, 1 };

    *outputSize = 4*((dataSize + 2)/3);

    char *encodedData = (char *)RL_MALLOC(*outputSize);

    if (encodedData == NULL) return NULL;

    for (int i = 0, j = 0; i < dataSize;)
    {
        unsigned int octetA = (i < dataSize)? (unsigned char)data[i++] : 0;
        unsigned int octetB = (i < dataSize)? (unsigned char)data[i++] : 0;
        unsigned int octetC = (i < dataSize)? (unsigned char)data[i++] : 0;

        unsigned int triple = (octetA << 0x10) + (octetB << 0x08) + octetC;

        encodedData[j++] = base64encodeTable[(triple >> 3*6) & 0x3F];
        encodedData[j++] = base64encodeTable[(triple >> 2*6) & 0x3F];
        encodedData[j++] = base64encodeTable[(triple >> 1*6) & 0x3F];
        encodedData[j++] = base64encodeTable[(triple >> 0*6) & 0x3F];
    }

    for (int i = 0; i < modTable[dataSize%3]; i++) encodedData[*outputSize - 1 - i] = '=';  // Padding character

    return encodedData;
}

// Decode Base64 string data
unsigned char *rlDecodeDataBase64(const unsigned char *data, int *outputSize)
{
    static const unsigned char base64decodeTable[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 62, 0, 0, 0, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 0, 0, 0, 0, 0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
        37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
    };

    // Get output size of Base64 input data
    int outSize = 0;
    for (int i = 0; data[4*i] != 0; i++)
    {
        if (data[4*i + 3] == '=')
        {
            if (data[4*i + 2] == '=') outSize += 1;
            else outSize += 2;
        }
        else outSize += 3;
    }

    // Allocate memory to store decoded Base64 data
    unsigned char *decodedData = (unsigned char *)RL_MALLOC(outSize);

    for (int i = 0; i < outSize/3; i++)
    {
        unsigned char a = base64decodeTable[(int)data[4*i]];
        unsigned char b = base64decodeTable[(int)data[4*i + 1]];
        unsigned char c = base64decodeTable[(int)data[4*i + 2]];
        unsigned char d = base64decodeTable[(int)data[4*i + 3]];

        decodedData[3*i] = (a << 2) | (b >> 4);
        decodedData[3*i + 1] = (b << 4) | (c >> 2);
        decodedData[3*i + 2] = (c << 6) | d;
    }

    if (outSize%3 == 1)
    {
        int n = outSize/3;
        unsigned char a = base64decodeTable[(int)data[4*n]];
        unsigned char b = base64decodeTable[(int)data[4*n + 1]];
        decodedData[outSize - 1] = (a << 2) | (b >> 4);
    }
    else if (outSize%3 == 2)
    {
        int n = outSize/3;
        unsigned char a = base64decodeTable[(int)data[4*n]];
        unsigned char b = base64decodeTable[(int)data[4*n + 1]];
        unsigned char c = base64decodeTable[(int)data[4*n + 2]];
        decodedData[outSize - 2] = (a << 2) | (b >> 4);
        decodedData[outSize - 1] = (b << 4) | (c >> 2);
    }

    *outputSize = outSize;
    return decodedData;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Automation Events Recording and Playing
//----------------------------------------------------------------------------------

// Load automation events list from file, NULL for empty list, capacity = RL_MAX_AUTOMATION_EVENTS
rlAutomationEventList rlLoadAutomationEventList(const char *fileName)
{
    rlAutomationEventList list = { 0 };

    // Allocate and empty automation event list, ready to record new events
    list.events = (rlAutomationEvent *)RL_CALLOC(RL_MAX_AUTOMATION_EVENTS, sizeof(rlAutomationEvent));
    list.capacity = RL_MAX_AUTOMATION_EVENTS;

#if defined(RL_SUPPORT_AUTOMATION_EVENTS)
    if (fileName == NULL) rlTRACELOG(RL_LOG_INFO, "AUTOMATION: New empty events list loaded successfully");
    else
    {
        // Load automation events file (binary)
        /*
        //int dataSize = 0;
        //unsigned char *data = rlLoadFileData(fileName, &dataSize);

        FILE *raeFile = fopen(fileName, "rb");
        unsigned char fileId[4] = { 0 };

        fread(fileId, 1, 4, raeFile);

        if ((fileId[0] == 'r') && (fileId[1] == 'A') && (fileId[2] == 'E') && (fileId[1] == ' '))
        {
            fread(&eventCount, sizeof(int), 1, raeFile);
            rlTRACELOG(RL_LOG_WARNING, "Events loaded: %i\n", eventCount);
            fread(events, sizeof(rlAutomationEvent), eventCount, raeFile);
        }

        fclose(raeFile);
        */

        // Load events file (text)
        //unsigned char *buffer = rlLoadFileText(fileName);
        FILE *raeFile = fopen(fileName, "rt");

        if (raeFile != NULL)
        {
            unsigned int counter = 0;
            char buffer[256] = { 0 };
            char eventDesc[64] = { 0 };

            fgets(buffer, 256, raeFile);

            while (!feof(raeFile))
            {
                switch (buffer[0])
                {
                    case 'c': sscanf(buffer, "c %i", &list.count); break;
                    case 'e':
                    {
                        sscanf(buffer, "e %d %d %d %d %d %d %[^\n]s", &list.events[counter].frame, &list.events[counter].type,
                               &list.events[counter].params[0], &list.events[counter].params[1], &list.events[counter].params[2], &list.events[counter].params[3], eventDesc);

                        counter++;
                    } break;
                    default: break;
                }

                fgets(buffer, 256, raeFile);
            }

            if (counter != list.count)
            {
                rlTRACELOG(RL_LOG_WARNING, "AUTOMATION: Events read from file [%i] do not mach event count specified [%i]", counter, list.count);
                list.count = counter;
            }

            fclose(raeFile);

            rlTRACELOG(RL_LOG_INFO, "AUTOMATION: Events file loaded successfully");
        }

        rlTRACELOG(RL_LOG_INFO, "AUTOMATION: Events loaded from file: %i", list.count);
    }
#endif
    return list;
}

// Unload automation events list from file
void rlUnloadAutomationEventList(rlAutomationEventList *list)
{
#if defined(RL_SUPPORT_AUTOMATION_EVENTS)
    RL_FREE(list->events);
    list->events = NULL;
    list->count = 0;
    list->capacity = 0;
#endif
}

// Export automation events list as text file
bool rlExportAutomationEventList(rlAutomationEventList list, const char *fileName)
{
    bool success = false;

#if defined(RL_SUPPORT_AUTOMATION_EVENTS)
    // Export events as binary file
    // TODO: Save to memory buffer and rlSaveFileData()
    /*
    unsigned char fileId[4] = "rAE ";
    FILE *raeFile = fopen(fileName, "wb");
    fwrite(fileId, sizeof(unsigned char), 4, raeFile);
    fwrite(&eventCount, sizeof(int), 1, raeFile);
    fwrite(events, sizeof(rlAutomationEvent), eventCount, raeFile);
    fclose(raeFile);
    */

    // Export events as text
    // TODO: Save to memory buffer and rlSaveFileText()
    char *txtData = (char *)RL_CALLOC(256*list.count + 2048, sizeof(char)); // 256 characters per line plus some header

    int byteCount = 0;
    byteCount += sprintf(txtData + byteCount, "#\n");
    byteCount += sprintf(txtData + byteCount, "# Automation events exporter v1.0 - raylib automation events list\n");
    byteCount += sprintf(txtData + byteCount, "#\n");
    byteCount += sprintf(txtData + byteCount, "#    c <events_count>\n");
    byteCount += sprintf(txtData + byteCount, "#    e <frame> <event_type> <param0> <param1> <param2> <param3> // <event_type_name>\n");
    byteCount += sprintf(txtData + byteCount, "#\n");
    byteCount += sprintf(txtData + byteCount, "# more info and bugs-report:  github.com/raysan5/raylib\n");
    byteCount += sprintf(txtData + byteCount, "# feedback and support:       ray[at]raylib.com\n");
    byteCount += sprintf(txtData + byteCount, "#\n");
    byteCount += sprintf(txtData + byteCount, "# Copyright (c) 2023-2024 Ramon Santamaria (@raysan5)\n");
    byteCount += sprintf(txtData + byteCount, "#\n\n");

    // Add events data
    byteCount += sprintf(txtData + byteCount, "c %i\n", list.count);
    for (unsigned int i = 0; i < list.count; i++)
    {
        byteCount += snprintf(txtData + byteCount, 256, "e %i %i %i %i %i %i // Event: %s\n", list.events[i].frame, list.events[i].type,
            list.events[i].params[0], list.events[i].params[1], list.events[i].params[2], list.events[i].params[3], autoEventTypeName[list.events[i].type]);
    }

    // NOTE: Text data size exported is determined by '\0' (NULL) character
    success = rlSaveFileText(fileName, txtData);

    RL_FREE(txtData);
#endif

    return success;
}

// Setup automation event list to record to
void rlSetAutomationEventList(rlAutomationEventList *list)
{
#if defined(RL_SUPPORT_AUTOMATION_EVENTS)
    currentEventList = list;
#endif
}

// Set automation event internal base frame to start recording
void rlSetAutomationEventBaseFrame(int frame)
{
    rlCORE.Time.frameCounter = frame;
}

// Start recording automation events (rlAutomationEventList must be set)
void rlStartAutomationEventRecording(void)
{
#if defined(RL_SUPPORT_AUTOMATION_EVENTS)
    automationEventRecording = true;
#endif
}

// Stop recording automation events
void rlStopAutomationEventRecording(void)
{
#if defined(RL_SUPPORT_AUTOMATION_EVENTS)
    automationEventRecording = false;
#endif
}

// Play a recorded automation event
void rlPlayAutomationEvent(rlAutomationEvent event)
{
#if defined(RL_SUPPORT_AUTOMATION_EVENTS)
    // WARNING: When should event be played? After/before/replace rlPollInputEvents()? -> Up to the user!

    if (!automationEventRecording)      // TODO: Allow recording events while playing?
    {
        switch (event.type)
        {
            // Input event
            case INPUT_KEY_UP: rlCORE.Input.Keyboard.currentKeyState[event.params[0]] = false; break;             // param[0]: key
            case INPUT_KEY_DOWN: {                                                                              // param[0]: key
                rlCORE.Input.Keyboard.currentKeyState[event.params[0]] = true;

                if (rlCORE.Input.Keyboard.previousKeyState[event.params[0]] == false)
                {
                    if (rlCORE.Input.Keyboard.keyPressedQueueCount < RL_MAX_KEY_PRESSED_QUEUE)
                    {
                        // Add character to the queue
                        rlCORE.Input.Keyboard.keyPressedQueue[rlCORE.Input.Keyboard.keyPressedQueueCount] = event.params[0];
                        rlCORE.Input.Keyboard.keyPressedQueueCount++;
                    }
                }
            } break;
            case INPUT_MOUSE_BUTTON_UP: rlCORE.Input.Mouse.currentButtonState[event.params[0]] = false; break;    // param[0]: key
            case INPUT_MOUSE_BUTTON_DOWN: rlCORE.Input.Mouse.currentButtonState[event.params[0]] = true; break;   // param[0]: key
            case INPUT_MOUSE_POSITION:      // param[0]: x, param[1]: y
            {
                rlCORE.Input.Mouse.currentPosition.x = (float)event.params[0];
                rlCORE.Input.Mouse.currentPosition.y = (float)event.params[1];
            } break;
            case INPUT_MOUSE_WHEEL_MOTION:  // param[0]: x delta, param[1]: y delta
            {
                rlCORE.Input.Mouse.currentWheelMove.x = (float)event.params[0]; break;
                rlCORE.Input.Mouse.currentWheelMove.y = (float)event.params[1]; break;
            } break;
            case INPUT_TOUCH_UP: rlCORE.Input.Touch.currentTouchState[event.params[0]] = false; break;            // param[0]: id
            case INPUT_TOUCH_DOWN: rlCORE.Input.Touch.currentTouchState[event.params[0]] = true; break;           // param[0]: id
            case INPUT_TOUCH_POSITION:      // param[0]: id, param[1]: x, param[2]: y
            {
                rlCORE.Input.Touch.position[event.params[0]].x = (float)event.params[1];
                rlCORE.Input.Touch.position[event.params[0]].y = (float)event.params[2];
            } break;
            case INPUT_GAMEPAD_CONNECT: rlCORE.Input.Gamepad.ready[event.params[0]] = true; break;                // param[0]: gamepad
            case INPUT_GAMEPAD_DISCONNECT: rlCORE.Input.Gamepad.ready[event.params[0]] = false; break;            // param[0]: gamepad
            case INPUT_GAMEPAD_BUTTON_UP: rlCORE.Input.Gamepad.currentButtonState[event.params[0]][event.params[1]] = false; break;    // param[0]: gamepad, param[1]: button
            case INPUT_GAMEPAD_BUTTON_DOWN: rlCORE.Input.Gamepad.currentButtonState[event.params[0]][event.params[1]] = true; break;   // param[0]: gamepad, param[1]: button
            case INPUT_GAMEPAD_AXIS_MOTION: // param[0]: gamepad, param[1]: axis, param[2]: delta
            {
                rlCORE.Input.Gamepad.axisState[event.params[0]][event.params[1]] = ((float)event.params[2]/32768.0f);
            } break;
            case INPUT_GESTURE: RL_GESTURES.current = event.params[0]; break;     // param[0]: gesture (enum rlGesture) -> rgestures.h: RL_GESTURES.current

            // Window event
            case WINDOW_CLOSE: rlCORE.Window.shouldClose = true; break;
            case WINDOW_MAXIMIZE: rlMaximizeWindow(); break;
            case WINDOW_MINIMIZE: rlMinimizeWindow(); break;
            case WINDOW_RESIZE: rlSetWindowSize(event.params[0], event.params[1]); break;

            // Custom event
            case ACTION_TAKE_SCREENSHOT:
            {
                rlTakeScreenshot(rlTextFormat("screenshot%03i.png", screenshotCounter));
                screenshotCounter++;
            } break;
            case ACTION_SETTARGETFPS: rlSetTargetFPS(event.params[0]); break;
            default: break;
        }
    }
#endif
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Input Handling: Keyboard
//----------------------------------------------------------------------------------

// Check if a key has been pressed once
bool rlIsKeyPressed(int key)
{

    bool pressed = false;

    if ((key > 0) && (key < RL_MAX_KEYBOARD_KEYS))
    {
        if ((rlCORE.Input.Keyboard.previousKeyState[key] == 0) && (rlCORE.Input.Keyboard.currentKeyState[key] == 1)) pressed = true;
    }

    return pressed;
}

// Check if a key has been pressed again
bool rlIsKeyPressedRepeat(int key)
{
    bool repeat = false;

    if ((key > 0) && (key < RL_MAX_KEYBOARD_KEYS))
    {
        if (rlCORE.Input.Keyboard.keyRepeatInFrame[key] == 1) repeat = true;
    }

    return repeat;
}

// Check if a key is being pressed (key held down)
bool rlIsKeyDown(int key)
{
    bool down = false;

    if ((key > 0) && (key < RL_MAX_KEYBOARD_KEYS))
    {
        if (rlCORE.Input.Keyboard.currentKeyState[key] == 1) down = true;
    }

    return down;
}

// Check if a key has been released once
bool rlIsKeyReleased(int key)
{
    bool released = false;

    if ((key > 0) && (key < RL_MAX_KEYBOARD_KEYS))
    {
        if ((rlCORE.Input.Keyboard.previousKeyState[key] == 1) && (rlCORE.Input.Keyboard.currentKeyState[key] == 0)) released = true;
    }

    return released;
}

// Check if a key is NOT being pressed (key not held down)
bool rlIsKeyUp(int key)
{
    bool up = false;

    if ((key > 0) && (key < RL_MAX_KEYBOARD_KEYS))
    {
        if (rlCORE.Input.Keyboard.currentKeyState[key] == 0) up = true;
    }

    return up;
}

// Get the last key pressed
int rlGetKeyPressed(void)
{
    int value = 0;

    if (rlCORE.Input.Keyboard.keyPressedQueueCount > 0)
    {
        // Get character from the queue head
        value = rlCORE.Input.Keyboard.keyPressedQueue[0];

        // Shift elements 1 step toward the head
        for (int i = 0; i < (rlCORE.Input.Keyboard.keyPressedQueueCount - 1); i++)
            rlCORE.Input.Keyboard.keyPressedQueue[i] = rlCORE.Input.Keyboard.keyPressedQueue[i + 1];

        // Reset last character in the queue
        rlCORE.Input.Keyboard.keyPressedQueue[rlCORE.Input.Keyboard.keyPressedQueueCount - 1] = 0;
        rlCORE.Input.Keyboard.keyPressedQueueCount--;
    }

    return value;
}

// Get the last char pressed
int rlGetCharPressed(void)
{
    int value = 0;

    if (rlCORE.Input.Keyboard.charPressedQueueCount > 0)
    {
        // Get character from the queue head
        value = rlCORE.Input.Keyboard.charPressedQueue[0];

        // Shift elements 1 step toward the head
        for (int i = 0; i < (rlCORE.Input.Keyboard.charPressedQueueCount - 1); i++)
            rlCORE.Input.Keyboard.charPressedQueue[i] = rlCORE.Input.Keyboard.charPressedQueue[i + 1];

        // Reset last character in the queue
        rlCORE.Input.Keyboard.charPressedQueue[rlCORE.Input.Keyboard.charPressedQueueCount - 1] = 0;
        rlCORE.Input.Keyboard.charPressedQueueCount--;
    }

    return value;
}

// Set a custom key to exit program
// NOTE: default exitKey is set to ESCAPE
void rlSetExitKey(int key)
{
    rlCORE.Input.Keyboard.exitKey = key;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Input Handling: Gamepad
//----------------------------------------------------------------------------------

// NOTE: Functions with a platform-specific implementation on rcore_<platform>.c
//int rlSetGamepadMappings(const char *mappings)

// Check if a gamepad is available
bool rlIsGamepadAvailable(int gamepad)
{
    bool result = false;

    if ((gamepad < RL_MAX_GAMEPADS) && rlCORE.Input.Gamepad.ready[gamepad]) result = true;

    return result;
}

// Get gamepad internal name id
const char *rlGetGamepadName(int gamepad)
{
    return rlCORE.Input.Gamepad.name[gamepad];
}

// Check if a gamepad button has been pressed once
bool rlIsGamepadButtonPressed(int gamepad, int button)
{
    bool pressed = false;

    if ((gamepad < RL_MAX_GAMEPADS) && rlCORE.Input.Gamepad.ready[gamepad] && (button < RL_MAX_GAMEPAD_BUTTONS) &&
        (rlCORE.Input.Gamepad.previousButtonState[gamepad][button] == 0) && (rlCORE.Input.Gamepad.currentButtonState[gamepad][button] == 1)) pressed = true;

    return pressed;
}

// Check if a gamepad button is being pressed
bool rlIsGamepadButtonDown(int gamepad, int button)
{
    bool down = false;

    if ((gamepad < RL_MAX_GAMEPADS) && rlCORE.Input.Gamepad.ready[gamepad] && (button < RL_MAX_GAMEPAD_BUTTONS) &&
        (rlCORE.Input.Gamepad.currentButtonState[gamepad][button] == 1)) down = true;

    return down;
}

// Check if a gamepad button has NOT been pressed once
bool rlIsGamepadButtonReleased(int gamepad, int button)
{
    bool released = false;

    if ((gamepad < RL_MAX_GAMEPADS) && rlCORE.Input.Gamepad.ready[gamepad] && (button < RL_MAX_GAMEPAD_BUTTONS) &&
        (rlCORE.Input.Gamepad.previousButtonState[gamepad][button] == 1) && (rlCORE.Input.Gamepad.currentButtonState[gamepad][button] == 0)) released = true;

    return released;
}

// Check if a gamepad button is NOT being pressed
bool rlIsGamepadButtonUp(int gamepad, int button)
{
    bool up = false;

    if ((gamepad < RL_MAX_GAMEPADS) && rlCORE.Input.Gamepad.ready[gamepad] && (button < RL_MAX_GAMEPAD_BUTTONS) &&
        (rlCORE.Input.Gamepad.currentButtonState[gamepad][button] == 0)) up = true;

    return up;
}

// Get the last gamepad button pressed
int rlGetGamepadButtonPressed(void)
{
    return rlCORE.Input.Gamepad.lastButtonPressed;
}

// Get gamepad axis count
int rlGetGamepadAxisCount(int gamepad)
{
    return rlCORE.Input.Gamepad.axisCount[gamepad];
}

// Get axis movement vector for a gamepad
float rlGetGamepadAxisMovement(int gamepad, int axis)
{
    float value = 0;

    if ((gamepad < RL_MAX_GAMEPADS) && rlCORE.Input.Gamepad.ready[gamepad] && (axis < RL_MAX_GAMEPAD_AXIS) &&
        (fabsf(rlCORE.Input.Gamepad.axisState[gamepad][axis]) > 0.1f)) value = rlCORE.Input.Gamepad.axisState[gamepad][axis];      // 0.1f = RL_GAMEPAD_AXIS_MINIMUM_DRIFT/DELTA

    return value;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Input Handling: Mouse
//----------------------------------------------------------------------------------

// NOTE: Functions with a platform-specific implementation on rcore_<platform>.c
//void rlSetMousePosition(int x, int y)
//void rlSetMouseCursor(int cursor)

// Check if a mouse button has been pressed once
bool rlIsMouseButtonPressed(int button)
{
    bool pressed = false;

    if ((rlCORE.Input.Mouse.currentButtonState[button] == 1) && (rlCORE.Input.Mouse.previousButtonState[button] == 0)) pressed = true;

    // Map touches to mouse buttons checking
    if ((rlCORE.Input.Touch.currentTouchState[button] == 1) && (rlCORE.Input.Touch.previousTouchState[button] == 0)) pressed = true;

    return pressed;
}

// Check if a mouse button is being pressed
bool rlIsMouseButtonDown(int button)
{
    bool down = false;

    if (rlCORE.Input.Mouse.currentButtonState[button] == 1) down = true;

    // NOTE: Touches are considered like mouse buttons
    if (rlCORE.Input.Touch.currentTouchState[button] == 1) down = true;

    return down;
}

// Check if a mouse button has been released once
bool rlIsMouseButtonReleased(int button)
{
    bool released = false;

    if ((rlCORE.Input.Mouse.currentButtonState[button] == 0) && (rlCORE.Input.Mouse.previousButtonState[button] == 1)) released = true;

    // Map touches to mouse buttons checking
    if ((rlCORE.Input.Touch.currentTouchState[button] == 0) && (rlCORE.Input.Touch.previousTouchState[button] == 1)) released = true;

    return released;
}

// Check if a mouse button is NOT being pressed
bool rlIsMouseButtonUp(int button)
{
    bool up = false;

    if (rlCORE.Input.Mouse.currentButtonState[button] == 0) up = true;

    // NOTE: Touches are considered like mouse buttons
    if (rlCORE.Input.Touch.currentTouchState[button] == 0) up = true;

    return up;
}

// Get mouse position X
int rlGetMouseX(void)
{
    return (int)((rlCORE.Input.Mouse.currentPosition.x + rlCORE.Input.Mouse.offset.x)*rlCORE.Input.Mouse.scale.x);
}

// Get mouse position Y
int rlGetMouseY(void)
{
    return (int)((rlCORE.Input.Mouse.currentPosition.y + rlCORE.Input.Mouse.offset.y)*rlCORE.Input.Mouse.scale.y);
}

// Get mouse position XY
rlVector2 rlGetMousePosition(void)
{
    rlVector2 position = { 0 };

    position.x = (rlCORE.Input.Mouse.currentPosition.x + rlCORE.Input.Mouse.offset.x)*rlCORE.Input.Mouse.scale.x;
    position.y = (rlCORE.Input.Mouse.currentPosition.y + rlCORE.Input.Mouse.offset.y)*rlCORE.Input.Mouse.scale.y;

    return position;
}

// Get mouse delta between frames
rlVector2 rlGetMouseDelta(void)
{
    rlVector2 delta = { 0 };

    delta.x = rlCORE.Input.Mouse.currentPosition.x - rlCORE.Input.Mouse.previousPosition.x;
    delta.y = rlCORE.Input.Mouse.currentPosition.y - rlCORE.Input.Mouse.previousPosition.y;

    return delta;
}

// Set mouse offset
// NOTE: Useful when rendering to different size targets
void rlSetMouseOffset(int offsetX, int offsetY)
{
    rlCORE.Input.Mouse.offset = CAST(rlVector2){ (float)offsetX, (float)offsetY };
}

// Set mouse scaling
// NOTE: Useful when rendering to different size targets
void rlSetMouseScale(float scaleX, float scaleY)
{
    rlCORE.Input.Mouse.scale = CAST(rlVector2){ scaleX, scaleY };
}

// Get mouse wheel movement Y
float rlGetMouseWheelMove(void)
{
    float result = 0.0f;

    if (fabsf(rlCORE.Input.Mouse.currentWheelMove.x) > fabsf(rlCORE.Input.Mouse.currentWheelMove.y)) result = (float)rlCORE.Input.Mouse.currentWheelMove.x;
    else result = (float)rlCORE.Input.Mouse.currentWheelMove.y;

    return result;
}

// Get mouse wheel movement X/Y as a vector
rlVector2 rlGetMouseWheelMoveV(void)
{
    rlVector2 result = { 0 };

    result = rlCORE.Input.Mouse.currentWheelMove;

    return result;
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Input Handling: Touch
//----------------------------------------------------------------------------------

// Get touch position X for touch point 0 (relative to screen size)
int rlGetTouchX(void)
{
    return (int)rlCORE.Input.Touch.position[0].x;
}

// Get touch position Y for touch point 0 (relative to screen size)
int rlGetTouchY(void)
{
    return (int)rlCORE.Input.Touch.position[0].y;
}

// Get touch position XY for a touch point index (relative to screen size)
// TODO: Touch position should be scaled depending on display size and render size
rlVector2 rlGetTouchPosition(int index)
{
    rlVector2 position = { -1.0f, -1.0f };

    if (index < RL_MAX_TOUCH_POINTS) position = rlCORE.Input.Touch.position[index];
    else rlTRACELOG(RL_LOG_WARNING, "INPUT: Required touch point out of range (Max touch points: %i)", RL_MAX_TOUCH_POINTS);

    return position;
}

// Get touch point identifier for given index
int rlGetTouchPointId(int index)
{
    int id = -1;

    if (index < RL_MAX_TOUCH_POINTS) id = rlCORE.Input.Touch.pointId[index];

    return id;
}

// Get number of touch points
int rlGetTouchPointCount(void)
{
    return rlCORE.Input.Touch.pointCount;
}

//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

// NOTE: Functions with a platform-specific implementation on rcore_<platform>.c
//int InitPlatform(void)
//void ClosePlatform(void)

// Initialize hi-resolution timer
void InitTimer(void)
{
    // Setting a higher resolution can improve the accuracy of time-out intervals in wait functions.
    // However, it can also reduce overall system performance, because the thread scheduler switches tasks more often.
    // High resolutions can also prevent the CPU power management system from entering power-saving modes.
    // Setting a higher resolution does not improve the accuracy of the high-resolution performance counter.
#if defined(_WIN32) && defined(RL_SUPPORT_WINMM_HIGHRES_TIMER) && !defined(RL_SUPPORT_BUSY_WAIT_LOOP) && !defined(PLATFORM_DESKTOP_SDL)
    timeBeginPeriod(1);                 // Setup high-resolution timer to 1ms (granularity of 1-2 ms)
#endif

#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__EMSCRIPTEN__)
    struct timespec now = { 0 };

    if (clock_gettime(CLOCK_MONOTONIC, &now) == 0)  // Success
    {
        rlCORE.Time.base = (unsigned long long int)now.tv_sec*1000000000LLU + (unsigned long long int)now.tv_nsec;
    }
    else rlTRACELOG(RL_LOG_WARNING, "TIMER: Hi-resolution timer not available");
#endif

    rlCORE.Time.previous = rlGetTime();     // Get time as double
}

// Set viewport for a provided width and height
void SetupViewport(int width, int height)
{
    rlCORE.Window.render.width = width;
    rlCORE.Window.render.height = height;

    // Set viewport width and height
    // NOTE: We consider render size (scaled) and offset in case black bars are required and
    // render area does not match full display area (this situation is only applicable on fullscreen mode)
#if defined(__APPLE__)
    rlVector2 scale = rlGetWindowScaleDPI();
    rlglViewport(rlCORE.Window.renderOffset.x/2*scale.x, rlCORE.Window.renderOffset.y/2*scale.y, (rlCORE.Window.render.width)*scale.x, (rlCORE.Window.render.height)*scale.y);
#else
    rlglViewport(rlCORE.Window.renderOffset.x/2, rlCORE.Window.renderOffset.y/2, rlCORE.Window.render.width, rlCORE.Window.render.height);
#endif

    rlglMatrixMode(RL_PROJECTION);        // Switch to projection matrix
    rlglLoadIdentity();                   // Reset current matrix (projection)

    // Set orthographic projection to current framebuffer size
    // NOTE: Configured top-left corner as (0, 0)
    rlglOrtho(0, rlCORE.Window.render.width, rlCORE.Window.render.height, 0, 0.0f, 1.0f);

    rlglMatrixMode(RL_MODELVIEW);         // Switch back to modelview matrix
    rlglLoadIdentity();                   // Reset current matrix (modelview)
}

// Compute framebuffer size relative to screen size and display size
// NOTE: Global variables rlCORE.Window.render.width/rlCORE.Window.render.height and rlCORE.Window.renderOffset.x/rlCORE.Window.renderOffset.y can be modified
void SetupFramebuffer(int width, int height)
{
    // Calculate rlCORE.Window.render.width and rlCORE.Window.render.height, we have the display size (input params) and the desired screen size (global var)
    if ((rlCORE.Window.screen.width > rlCORE.Window.display.width) || (rlCORE.Window.screen.height > rlCORE.Window.display.height))
    {
        rlTRACELOG(RL_LOG_WARNING, "DISPLAY: Downscaling required: Screen size (%ix%i) is bigger than display size (%ix%i)", rlCORE.Window.screen.width, rlCORE.Window.screen.height, rlCORE.Window.display.width, rlCORE.Window.display.height);

        // Downscaling to fit display with border-bars
        float widthRatio = (float)rlCORE.Window.display.width/(float)rlCORE.Window.screen.width;
        float heightRatio = (float)rlCORE.Window.display.height/(float)rlCORE.Window.screen.height;

        if (widthRatio <= heightRatio)
        {
            rlCORE.Window.render.width = rlCORE.Window.display.width;
            rlCORE.Window.render.height = (int)round((float)rlCORE.Window.screen.height*widthRatio);
            rlCORE.Window.renderOffset.x = 0;
            rlCORE.Window.renderOffset.y = (rlCORE.Window.display.height - rlCORE.Window.render.height);
        }
        else
        {
            rlCORE.Window.render.width = (int)round((float)rlCORE.Window.screen.width*heightRatio);
            rlCORE.Window.render.height = rlCORE.Window.display.height;
            rlCORE.Window.renderOffset.x = (rlCORE.Window.display.width - rlCORE.Window.render.width);
            rlCORE.Window.renderOffset.y = 0;
        }

        // Screen scaling required
        float scaleRatio = (float)rlCORE.Window.render.width/(float)rlCORE.Window.screen.width;
        rlCORE.Window.screenScale = rlMatrixScale(scaleRatio, scaleRatio, 1.0f);

        // NOTE: We render to full display resolution!
        // We just need to calculate above parameters for downscale matrix and offsets
        rlCORE.Window.render.width = rlCORE.Window.display.width;
        rlCORE.Window.render.height = rlCORE.Window.display.height;

        rlTRACELOG(RL_LOG_WARNING, "DISPLAY: Downscale matrix generated, content will be rendered at (%ix%i)", rlCORE.Window.render.width, rlCORE.Window.render.height);
    }
    else if ((rlCORE.Window.screen.width < rlCORE.Window.display.width) || (rlCORE.Window.screen.height < rlCORE.Window.display.height))
    {
        // Required screen size is smaller than display size
        rlTRACELOG(RL_LOG_INFO, "DISPLAY: Upscaling required: Screen size (%ix%i) smaller than display size (%ix%i)", rlCORE.Window.screen.width, rlCORE.Window.screen.height, rlCORE.Window.display.width, rlCORE.Window.display.height);

        if ((rlCORE.Window.screen.width == 0) || (rlCORE.Window.screen.height == 0))
        {
            rlCORE.Window.screen.width = rlCORE.Window.display.width;
            rlCORE.Window.screen.height = rlCORE.Window.display.height;
        }

        // Upscaling to fit display with border-bars
        float displayRatio = (float)rlCORE.Window.display.width/(float)rlCORE.Window.display.height;
        float screenRatio = (float)rlCORE.Window.screen.width/(float)rlCORE.Window.screen.height;

        if (displayRatio <= screenRatio)
        {
            rlCORE.Window.render.width = rlCORE.Window.screen.width;
            rlCORE.Window.render.height = (int)round((float)rlCORE.Window.screen.width/displayRatio);
            rlCORE.Window.renderOffset.x = 0;
            rlCORE.Window.renderOffset.y = (rlCORE.Window.render.height - rlCORE.Window.screen.height);
        }
        else
        {
            rlCORE.Window.render.width = (int)round((float)rlCORE.Window.screen.height*displayRatio);
            rlCORE.Window.render.height = rlCORE.Window.screen.height;
            rlCORE.Window.renderOffset.x = (rlCORE.Window.render.width - rlCORE.Window.screen.width);
            rlCORE.Window.renderOffset.y = 0;
        }
    }
    else
    {
        rlCORE.Window.render.width = rlCORE.Window.screen.width;
        rlCORE.Window.render.height = rlCORE.Window.screen.height;
        rlCORE.Window.renderOffset.x = 0;
        rlCORE.Window.renderOffset.y = 0;
    }
}

// Scan all files and directories in a base path
// WARNING: files.paths[] must be previously allocated and
// contain enough space to store all required paths
static void ScanDirectoryFiles(const char *basePath, rlFilePathList *files, const char *filter)
{
    static char path[RL_MAX_FILEPATH_LENGTH] = { 0 };
    memset(path, 0, RL_MAX_FILEPATH_LENGTH);

    struct dirent *dp = NULL;
    DIR *dir = opendir(basePath);

    if (dir != NULL)
    {
        while ((dp = readdir(dir)) != NULL)
        {
            if ((strcmp(dp->d_name, ".") != 0) &&
                (strcmp(dp->d_name, "..") != 0))
            {
            #if defined(_WIN32)
                sprintf(path, "%s\\%s", basePath, dp->d_name);
            #else
                sprintf(path, "%s/%s", basePath, dp->d_name);
            #endif

                if (filter != NULL)
                {
                    if (rlIsFileExtension(path, filter))
                    {
                        strcpy(files->paths[files->count], path);
                        files->count++;
                    }
                }
                else
                {
                    strcpy(files->paths[files->count], path);
                    files->count++;
                }
            }
        }

        closedir(dir);
    }
    else rlTRACELOG(RL_LOG_WARNING, "FILEIO: Directory cannot be opened (%s)", basePath);
}

// Scan all files and directories recursively from a base path
static void ScanDirectoryFilesRecursively(const char *basePath, rlFilePathList *files, const char *filter)
{
    char path[RL_MAX_FILEPATH_LENGTH] = { 0 };
    memset(path, 0, RL_MAX_FILEPATH_LENGTH);

    struct dirent *dp = NULL;
    DIR *dir = opendir(basePath);

    if (dir != NULL)
    {
        while (((dp = readdir(dir)) != NULL) && (files->count < files->capacity))
        {
            if ((strcmp(dp->d_name, ".") != 0) && (strcmp(dp->d_name, "..") != 0))
            {
                // Construct new path from our base path
            #if defined(_WIN32)
                sprintf(path, "%s\\%s", basePath, dp->d_name);
            #else
                sprintf(path, "%s/%s", basePath, dp->d_name);
            #endif

                if (rlIsPathFile(path))
                {
                    if (filter != NULL)
                    {
                        if (rlIsFileExtension(path, filter))
                        {
                            strcpy(files->paths[files->count], path);
                            files->count++;
                        }
                    }
                    else
                    {
                        strcpy(files->paths[files->count], path);
                        files->count++;
                    }

                    if (files->count >= files->capacity)
                    {
                        rlTRACELOG(RL_LOG_WARNING, "FILEIO: Maximum filepath scan capacity reached (%i files)", files->capacity);
                        break;
                    }
                }
                else ScanDirectoryFilesRecursively(path, files, filter);
            }
        }

        closedir(dir);
    }
    else rlTRACELOG(RL_LOG_WARNING, "FILEIO: Directory cannot be opened (%s)", basePath);
}

#if defined(RL_SUPPORT_AUTOMATION_EVENTS)
// Automation event recording
// NOTE: Recording is by default done at rlEndDrawing(), after rlPollInputEvents()
static void RecordAutomationEvent(void)
{
    // Checking events in current frame and save them into currentEventList
    // TODO: How important is the current frame? Could it be modified?

    if (currentEventList->count == currentEventList->capacity) return;    // Security check

    // Keyboard input events recording
    //-------------------------------------------------------------------------------------
    for (int key = 0; key < RL_MAX_KEYBOARD_KEYS; key++)
    {
        // Event type: INPUT_KEY_UP (only saved once)
        if (rlCORE.Input.Keyboard.previousKeyState[key] && !rlCORE.Input.Keyboard.currentKeyState[key])
        {
            currentEventList->events[currentEventList->count].frame = rlCORE.Time.frameCounter;
            currentEventList->events[currentEventList->count].type = INPUT_KEY_UP;
            currentEventList->events[currentEventList->count].params[0] = key;
            currentEventList->events[currentEventList->count].params[1] = 0;
            currentEventList->events[currentEventList->count].params[2] = 0;

            rlTRACELOG(RL_LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_KEY_UP | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
            currentEventList->count++;
        }

        if (currentEventList->count == currentEventList->capacity) return;    // Security check

        // Event type: INPUT_KEY_DOWN
        if (rlCORE.Input.Keyboard.currentKeyState[key])
        {
            currentEventList->events[currentEventList->count].frame = rlCORE.Time.frameCounter;
            currentEventList->events[currentEventList->count].type = INPUT_KEY_DOWN;
            currentEventList->events[currentEventList->count].params[0] = key;
            currentEventList->events[currentEventList->count].params[1] = 0;
            currentEventList->events[currentEventList->count].params[2] = 0;

            rlTRACELOG(RL_LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_KEY_DOWN | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
            currentEventList->count++;
        }

        if (currentEventList->count == currentEventList->capacity) return;    // Security check
    }
    //-------------------------------------------------------------------------------------

    // Mouse input currentEventList->events recording
    //-------------------------------------------------------------------------------------
    for (int button = 0; button < RL_MAX_MOUSE_BUTTONS; button++)
    {
        // Event type: INPUT_MOUSE_BUTTON_UP
        if (rlCORE.Input.Mouse.previousButtonState[button] && !rlCORE.Input.Mouse.currentButtonState[button])
        {
            currentEventList->events[currentEventList->count].frame = rlCORE.Time.frameCounter;
            currentEventList->events[currentEventList->count].type = INPUT_MOUSE_BUTTON_UP;
            currentEventList->events[currentEventList->count].params[0] = button;
            currentEventList->events[currentEventList->count].params[1] = 0;
            currentEventList->events[currentEventList->count].params[2] = 0;

            rlTRACELOG(RL_LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_MOUSE_BUTTON_UP | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
            currentEventList->count++;
        }

        if (currentEventList->count == currentEventList->capacity) return;    // Security check

        // Event type: INPUT_MOUSE_BUTTON_DOWN
        if (rlCORE.Input.Mouse.currentButtonState[button])
        {
            currentEventList->events[currentEventList->count].frame = rlCORE.Time.frameCounter;
            currentEventList->events[currentEventList->count].type = INPUT_MOUSE_BUTTON_DOWN;
            currentEventList->events[currentEventList->count].params[0] = button;
            currentEventList->events[currentEventList->count].params[1] = 0;
            currentEventList->events[currentEventList->count].params[2] = 0;

            rlTRACELOG(RL_LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_MOUSE_BUTTON_DOWN | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
            currentEventList->count++;
        }

        if (currentEventList->count == currentEventList->capacity) return;    // Security check
    }

    // Event type: INPUT_MOUSE_POSITION (only saved if changed)
    if (((int)rlCORE.Input.Mouse.currentPosition.x != (int)rlCORE.Input.Mouse.previousPosition.x) ||
        ((int)rlCORE.Input.Mouse.currentPosition.y != (int)rlCORE.Input.Mouse.previousPosition.y))
    {
        currentEventList->events[currentEventList->count].frame = rlCORE.Time.frameCounter;
        currentEventList->events[currentEventList->count].type = INPUT_MOUSE_POSITION;
        currentEventList->events[currentEventList->count].params[0] = (int)rlCORE.Input.Mouse.currentPosition.x;
        currentEventList->events[currentEventList->count].params[1] = (int)rlCORE.Input.Mouse.currentPosition.y;
        currentEventList->events[currentEventList->count].params[2] = 0;

        rlTRACELOG(RL_LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_MOUSE_POSITION | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
        currentEventList->count++;

        if (currentEventList->count == currentEventList->capacity) return;    // Security check
    }

    // Event type: INPUT_MOUSE_WHEEL_MOTION
    if (((int)rlCORE.Input.Mouse.currentWheelMove.x != (int)rlCORE.Input.Mouse.previousWheelMove.x) ||
        ((int)rlCORE.Input.Mouse.currentWheelMove.y != (int)rlCORE.Input.Mouse.previousWheelMove.y))
    {
        currentEventList->events[currentEventList->count].frame = rlCORE.Time.frameCounter;
        currentEventList->events[currentEventList->count].type = INPUT_MOUSE_WHEEL_MOTION;
        currentEventList->events[currentEventList->count].params[0] = (int)rlCORE.Input.Mouse.currentWheelMove.x;
        currentEventList->events[currentEventList->count].params[1] = (int)rlCORE.Input.Mouse.currentWheelMove.y;;
        currentEventList->events[currentEventList->count].params[2] = 0;

        rlTRACELOG(RL_LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_MOUSE_WHEEL_MOTION | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
        currentEventList->count++;

        if (currentEventList->count == currentEventList->capacity) return;    // Security check
    }
    //-------------------------------------------------------------------------------------

    // Touch input currentEventList->events recording
    //-------------------------------------------------------------------------------------
    for (int id = 0; id < RL_MAX_TOUCH_POINTS; id++)
    {
        // Event type: INPUT_TOUCH_UP
        if (rlCORE.Input.Touch.previousTouchState[id] && !rlCORE.Input.Touch.currentTouchState[id])
        {
            currentEventList->events[currentEventList->count].frame = rlCORE.Time.frameCounter;
            currentEventList->events[currentEventList->count].type = INPUT_TOUCH_UP;
            currentEventList->events[currentEventList->count].params[0] = id;
            currentEventList->events[currentEventList->count].params[1] = 0;
            currentEventList->events[currentEventList->count].params[2] = 0;

            rlTRACELOG(RL_LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_TOUCH_UP | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
            currentEventList->count++;
        }

        if (currentEventList->count == currentEventList->capacity) return;    // Security check

        // Event type: INPUT_TOUCH_DOWN
        if (rlCORE.Input.Touch.currentTouchState[id])
        {
            currentEventList->events[currentEventList->count].frame = rlCORE.Time.frameCounter;
            currentEventList->events[currentEventList->count].type = INPUT_TOUCH_DOWN;
            currentEventList->events[currentEventList->count].params[0] = id;
            currentEventList->events[currentEventList->count].params[1] = 0;
            currentEventList->events[currentEventList->count].params[2] = 0;

            rlTRACELOG(RL_LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_TOUCH_DOWN | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
            currentEventList->count++;
        }

        if (currentEventList->count == currentEventList->capacity) return;    // Security check

        // Event type: INPUT_TOUCH_POSITION
        // TODO: It requires the id!
        /*
        if (((int)rlCORE.Input.Touch.currentPosition[id].x != (int)rlCORE.Input.Touch.previousPosition[id].x) ||
            ((int)rlCORE.Input.Touch.currentPosition[id].y != (int)rlCORE.Input.Touch.previousPosition[id].y))
        {
            currentEventList->events[currentEventList->count].frame = rlCORE.Time.frameCounter;
            currentEventList->events[currentEventList->count].type = INPUT_TOUCH_POSITION;
            currentEventList->events[currentEventList->count].params[0] = id;
            currentEventList->events[currentEventList->count].params[1] = (int)rlCORE.Input.Touch.currentPosition[id].x;
            currentEventList->events[currentEventList->count].params[2] = (int)rlCORE.Input.Touch.currentPosition[id].y;

            rlTRACELOG(RL_LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_TOUCH_POSITION | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
            currentEventList->count++;
        }
        */

        if (currentEventList->count == currentEventList->capacity) return;    // Security check
    }
    //-------------------------------------------------------------------------------------

    // Gamepad input currentEventList->events recording
    //-------------------------------------------------------------------------------------
    for (int gamepad = 0; gamepad < RL_MAX_GAMEPADS; gamepad++)
    {
        // Event type: INPUT_GAMEPAD_CONNECT
        /*
        if ((rlCORE.Input.Gamepad.currentState[gamepad] != rlCORE.Input.Gamepad.previousState[gamepad]) &&
            (rlCORE.Input.Gamepad.currentState[gamepad])) // Check if changed to ready
        {
            // TODO: Save gamepad connect event
        }
        */

        // Event type: INPUT_GAMEPAD_DISCONNECT
        /*
        if ((rlCORE.Input.Gamepad.currentState[gamepad] != rlCORE.Input.Gamepad.previousState[gamepad]) &&
            (!rlCORE.Input.Gamepad.currentState[gamepad])) // Check if changed to not-ready
        {
            // TODO: Save gamepad disconnect event
        }
        */

        for (int button = 0; button < RL_MAX_GAMEPAD_BUTTONS; button++)
        {
            // Event type: INPUT_GAMEPAD_BUTTON_UP
            if (rlCORE.Input.Gamepad.previousButtonState[gamepad][button] && !rlCORE.Input.Gamepad.currentButtonState[gamepad][button])
            {
                currentEventList->events[currentEventList->count].frame = rlCORE.Time.frameCounter;
                currentEventList->events[currentEventList->count].type = INPUT_GAMEPAD_BUTTON_UP;
                currentEventList->events[currentEventList->count].params[0] = gamepad;
                currentEventList->events[currentEventList->count].params[1] = button;
                currentEventList->events[currentEventList->count].params[2] = 0;

                rlTRACELOG(RL_LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_GAMEPAD_BUTTON_UP | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
                currentEventList->count++;
            }

            if (currentEventList->count == currentEventList->capacity) return;    // Security check

            // Event type: INPUT_GAMEPAD_BUTTON_DOWN
            if (rlCORE.Input.Gamepad.currentButtonState[gamepad][button])
            {
                currentEventList->events[currentEventList->count].frame = rlCORE.Time.frameCounter;
                currentEventList->events[currentEventList->count].type = INPUT_GAMEPAD_BUTTON_DOWN;
                currentEventList->events[currentEventList->count].params[0] = gamepad;
                currentEventList->events[currentEventList->count].params[1] = button;
                currentEventList->events[currentEventList->count].params[2] = 0;

                rlTRACELOG(RL_LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_GAMEPAD_BUTTON_DOWN | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
                currentEventList->count++;
            }

            if (currentEventList->count == currentEventList->capacity) return;    // Security check
        }

        for (int axis = 0; axis < RL_MAX_GAMEPAD_AXIS; axis++)
        {
            // Event type: INPUT_GAMEPAD_AXIS_MOTION
            if (rlCORE.Input.Gamepad.axisState[gamepad][axis] > 0.1f)
            {
                currentEventList->events[currentEventList->count].frame = rlCORE.Time.frameCounter;
                currentEventList->events[currentEventList->count].type = INPUT_GAMEPAD_AXIS_MOTION;
                currentEventList->events[currentEventList->count].params[0] = gamepad;
                currentEventList->events[currentEventList->count].params[1] = axis;
                currentEventList->events[currentEventList->count].params[2] = (int)(rlCORE.Input.Gamepad.axisState[gamepad][axis]*32768.0f);

                rlTRACELOG(RL_LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_GAMEPAD_AXIS_MOTION | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
                currentEventList->count++;
            }

            if (currentEventList->count == currentEventList->capacity) return;    // Security check
        }
    }
    //-------------------------------------------------------------------------------------

    // Gestures input currentEventList->events recording
    //-------------------------------------------------------------------------------------
    if (RL_GESTURES.current != RL_GESTURE_NONE)
    {
        // Event type: INPUT_GESTURE
        currentEventList->events[currentEventList->count].frame = rlCORE.Time.frameCounter;
        currentEventList->events[currentEventList->count].type = INPUT_GESTURE;
        currentEventList->events[currentEventList->count].params[0] = RL_GESTURES.current;
        currentEventList->events[currentEventList->count].params[1] = 0;
        currentEventList->events[currentEventList->count].params[2] = 0;

        rlTRACELOG(RL_LOG_INFO, "AUTOMATION: Frame: %i | Event type: INPUT_GESTURE | Event parameters: %i, %i, %i", currentEventList->events[currentEventList->count].frame, currentEventList->events[currentEventList->count].params[0], currentEventList->events[currentEventList->count].params[1], currentEventList->events[currentEventList->count].params[2]);
        currentEventList->count++;

        if (currentEventList->count == currentEventList->capacity) return;    // Security check
    }
    //-------------------------------------------------------------------------------------

    // Window events recording
    //-------------------------------------------------------------------------------------
    // TODO.
    //-------------------------------------------------------------------------------------

    // Custom actions events recording
    //-------------------------------------------------------------------------------------
    // TODO.
    //-------------------------------------------------------------------------------------
}
#endif

#if !defined(RL_SUPPORT_MODULE_RTEXT)
// Formatting of text with variables to 'embed'
// WARNING: String returned will expire after this function is called RL_MAX_TEXTFORMAT_BUFFERS times
const char *rlTextFormat(const char *text, ...)
{
#ifndef RL_MAX_TEXTFORMAT_BUFFERS
    #define RL_MAX_TEXTFORMAT_BUFFERS      4        // Maximum number of static buffers for text formatting
#endif
#ifndef RL_MAX_TEXT_BUFFER_LENGTH
    #define RL_MAX_TEXT_BUFFER_LENGTH   1024        // Maximum size of static text buffer
#endif

    // We create an array of buffers so strings don't expire until RL_MAX_TEXTFORMAT_BUFFERS invocations
    static char buffers[RL_MAX_TEXTFORMAT_BUFFERS][RL_MAX_TEXT_BUFFER_LENGTH] = { 0 };
    static int index = 0;

    char *currentBuffer = buffers[index];
    memset(currentBuffer, 0, RL_MAX_TEXT_BUFFER_LENGTH);   // Clear buffer before using

    va_list args;
    va_start(args, text);
    int requiredByteCount = vsnprintf(currentBuffer, RL_MAX_TEXT_BUFFER_LENGTH, text, args);
    va_end(args);

    // If requiredByteCount is larger than the RL_MAX_TEXT_BUFFER_LENGTH, then overflow occured
    if (requiredByteCount >= RL_MAX_TEXT_BUFFER_LENGTH)
    {
        // Inserting "..." at the end of the string to mark as truncated
        char *truncBuffer = buffers[index] + RL_MAX_TEXT_BUFFER_LENGTH - 4; // Adding 4 bytes = "...\0"
        sprintf(truncBuffer, "...");
    }

    index += 1;     // Move to next buffer for next function call
    if (index >= RL_MAX_TEXTFORMAT_BUFFERS) index = 0;

    return currentBuffer;
}

#endif // !RL_SUPPORT_MODULE_RTEXT

RL_NS_END
