/**********************************************************************************************
*
*   rcore_<platform> template - Functions to manage window, graphics device and inputs
*
*   PLATFORM: <PLATFORM>
*       - TODO: Define the target platform for the core
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
*       - <platform-specific SDK dependency>
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

// TODO: Include the platform specific libraries

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    // TODO: Define the platform specific variables required

    // Display data
    EGLDisplay device;                  // Native display device (physical screen connection)
    EGLSurface surface;                 // Surface to draw on, framebuffers (connected to context)
    EGLContext context;                 // Graphic context, mode in which drawing can be done
    EGLConfig config;                   // Graphic config
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
bool InitGraphicsDevice(void);   // Initialize graphics device

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
    rlTRACELOG(RL_LOG_WARNING, "rlToggleFullscreen() not available on target platform");
}

// Toggle borderless windowed mode
void rlToggleBorderlessWindowed(void)
{
    rlTRACELOG(RL_LOG_WARNING, "rlToggleBorderlessWindowed() not available on target platform");
}

// Set window state: maximized, if resizable
void rlMaximizeWindow(void)
{
    rlTRACELOG(RL_LOG_WARNING, "rlMaximizeWindow() not available on target platform");
}

// Set window state: minimized
void rlMinimizeWindow(void)
{
    rlTRACELOG(RL_LOG_WARNING, "rlMinimizeWindow() not available on target platform");
}

// Set window state: not minimized/maximized
void rlRestoreWindow(void)
{
    rlTRACELOG(RL_LOG_WARNING, "rlRestoreWindow() not available on target platform");
}

// Set window configuration state using flags
void rlSetWindowState(unsigned int flags)
{
    rlTRACELOG(RL_LOG_WARNING, "rlSetWindowState() not available on target platform");
}

// Clear window configuration state flags
void rlClearWindowState(unsigned int flags)
{
    rlTRACELOG(RL_LOG_WARNING, "rlClearWindowState() not available on target platform");
}

// Set icon for window
void rlSetWindowIcon(rlImage image)
{
    rlTRACELOG(RL_LOG_WARNING, "rlSetWindowIcon() not available on target platform");
}

// Set icon for window
void rlSetWindowIcons(rlImage *images, int count)
{
    rlTRACELOG(RL_LOG_WARNING, "rlSetWindowIcons() not available on target platform");
}

// Set title for window
void rlSetWindowTitle(const char *title)
{
    rlCORE.Window.title = title;
}

// Set window position on screen (windowed mode)
void rlSetWindowPosition(int x, int y)
{
    rlTRACELOG(RL_LOG_WARNING, "rlSetWindowPosition() not available on target platform");
}

// Set monitor for the current window
void rlSetWindowMonitor(int monitor)
{
    rlTRACELOG(RL_LOG_WARNING, "rlSetWindowMonitor() not available on target platform");
}

// Set window minimum dimensions (RL_FLAG_WINDOW_RESIZABLE)
void rlSetWindowMinSize(int width, int height)
{
    rlCORE.Window.screenMin.width = width;
    rlCORE.Window.screenMin.height = height;
}

// Set window maximum dimensions (RL_FLAG_WINDOW_RESIZABLE)
void rlSetWindowMaxSize(int width, int height)
{
    rlCORE.Window.screenMax.width = width;
    rlCORE.Window.screenMax.height = height;
}

// Set window dimensions
void rlSetWindowSize(int width, int height)
{
    rlTRACELOG(RL_LOG_WARNING, "rlSetWindowSize() not available on target platform");
}

// Set window opacity, value opacity is between 0.0 and 1.0
void rlSetWindowOpacity(float opacity)
{
    rlTRACELOG(RL_LOG_WARNING, "rlSetWindowOpacity() not available on target platform");
}

// Set window focused
void rlSetWindowFocused(void)
{
    rlTRACELOG(RL_LOG_WARNING, "rlSetWindowFocused() not available on target platform");
}

// Get native window handle
void *rlGetWindowHandle(void)
{
    rlTRACELOG(RL_LOG_WARNING, "rlGetWindowHandle() not implemented on target platform");
    return NULL;
}

// Get number of monitors
int rlGetMonitorCount(void)
{
    rlTRACELOG(RL_LOG_WARNING, "rlGetMonitorCount() not implemented on target platform");
    return 1;
}

// Get number of monitors
int rlGetCurrentMonitor(void)
{
    rlTRACELOG(RL_LOG_WARNING, "rlGetCurrentMonitor() not implemented on target platform");
    return 0;
}

// Get selected monitor position
rlVector2 rlGetMonitorPosition(int monitor)
{
    rlTRACELOG(RL_LOG_WARNING, "rlGetMonitorPosition() not implemented on target platform");
    return (rlVector2){ 0, 0 };
}

// Get selected monitor width (currently used by monitor)
int rlGetMonitorWidth(int monitor)
{
    rlTRACELOG(RL_LOG_WARNING, "rlGetMonitorWidth() not implemented on target platform");
    return 0;
}

// Get selected monitor height (currently used by monitor)
int rlGetMonitorHeight(int monitor)
{
    rlTRACELOG(RL_LOG_WARNING, "rlGetMonitorHeight() not implemented on target platform");
    return 0;
}

// Get selected monitor physical width in millimetres
int rlGetMonitorPhysicalWidth(int monitor)
{
    rlTRACELOG(RL_LOG_WARNING, "rlGetMonitorPhysicalWidth() not implemented on target platform");
    return 0;
}

// Get selected monitor physical height in millimetres
int rlGetMonitorPhysicalHeight(int monitor)
{
    rlTRACELOG(RL_LOG_WARNING, "rlGetMonitorPhysicalHeight() not implemented on target platform");
    return 0;
}

// Get selected monitor refresh rate
int rlGetMonitorRefreshRate(int monitor)
{
    rlTRACELOG(RL_LOG_WARNING, "rlGetMonitorRefreshRate() not implemented on target platform");
    return 0;
}

// Get the human-readable, UTF-8 encoded name of the selected monitor
const char *rlGetMonitorName(int monitor)
{
    rlTRACELOG(RL_LOG_WARNING, "rlGetMonitorName() not implemented on target platform");
    return "";
}

// Get window position XY on monitor
rlVector2 rlGetWindowPosition(void)
{
    rlTRACELOG(RL_LOG_WARNING, "rlGetWindowPosition() not implemented on target platform");
    return (rlVector2){ 0, 0 };
}

// Get window scale DPI factor for current monitor
rlVector2 rlGetWindowScaleDPI(void)
{
    rlTRACELOG(RL_LOG_WARNING, "rlGetWindowScaleDPI() not implemented on target platform");
    return (rlVector2){ 1.0f, 1.0f };
}

// Set clipboard text content
void rlSetClipboardText(const char *text)
{
    rlTRACELOG(RL_LOG_WARNING, "rlSetClipboardText() not implemented on target platform");
}

// Get clipboard text content
// NOTE: returned string is allocated and freed by GLFW
const char *rlGetClipboardText(void)
{
    rlTRACELOG(RL_LOG_WARNING, "rlGetClipboardText() not implemented on target platform");
    return NULL;
}

// Show mouse cursor
void rlShowCursor(void)
{
    rlCORE.Input.Mouse.cursorHidden = false;
}

// Hides mouse cursor
void rlHideCursor(void)
{
    rlCORE.Input.Mouse.cursorHidden = true;
}

// Enables cursor (unlock cursor)
void rlEnableCursor(void)
{
    // Set cursor position in the middle
    rlSetMousePosition(rlCORE.Window.screen.width/2, rlCORE.Window.screen.height/2);

    rlCORE.Input.Mouse.cursorHidden = false;
}

// Disables cursor (lock cursor)
void rlDisableCursor(void)
{
    // Set cursor position in the middle
    rlSetMousePosition(rlCORE.Window.screen.width/2, rlCORE.Window.screen.height/2);

    rlCORE.Input.Mouse.cursorHidden = true;
}

// Swap back buffer with front buffer (screen drawing)
void rlSwapScreenBuffer(void)
{
    eglSwapBuffers(platform.device, platform.surface);
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Misc
//----------------------------------------------------------------------------------

// Get elapsed time measure in seconds since InitTimer()
double rlGetTime(void)
{
    double time = 0.0;
    struct timespec ts = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &ts);
    unsigned long long int nanoSeconds = (unsigned long long int)ts.tv_sec*1000000000LLU + (unsigned long long int)ts.tv_nsec;

    time = (double)(nanoSeconds - rlCORE.Time.base)*1e-9;  // Elapsed time since InitTimer()

    return time;
}

// Open URL with default system browser (if available)
// NOTE: This function is only safe to use if you control the URL given.
// A user could craft a malicious string performing another action.
// Only call this function yourself not with user input or make sure to check the string yourself.
// Ref: https://github.com/raysan5/raylib/issues/686
void rlOpenURL(const char *url)
{
    // Security check to (partially) avoid malicious code on target platform
    if (strchr(url, '\'') != NULL) rlTRACELOG(RL_LOG_WARNING, "SYSTEM: Provided URL could be potentially malicious, avoid [\'] character");
    else
    {
        // TODO:
    }
}

//----------------------------------------------------------------------------------
// Module Functions Definition: Inputs
//----------------------------------------------------------------------------------

// Set internal gamepad mappings
int rlSetGamepadMappings(const char *mappings)
{
    rlTRACELOG(RL_LOG_WARNING, "rlSetGamepadMappings() not implemented on target platform");
    return 0;
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
    rlTRACELOG(RL_LOG_WARNING, "rlSetMouseCursor() not implemented on target platform");
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

    // Reset last gamepad button/axis registered state
    rlCORE.Input.Gamepad.lastButtonPressed = 0; // RL_GAMEPAD_BUTTON_UNKNOWN
    //rlCORE.Input.Gamepad.axisCount = 0;

    // Register previous touch states
    for (int i = 0; i < RL_MAX_TOUCH_POINTS; i++) rlCORE.Input.Touch.previousTouchState[i] = rlCORE.Input.Touch.currentTouchState[i];

    // Reset touch positions
    // TODO: It resets on target platform the mouse position and not filled again until a move-event,
    // so, if mouse is not moved it returns a (0, 0) position... this behaviour should be reviewed!
    //for (int i = 0; i < RL_MAX_TOUCH_POINTS; i++) rlCORE.Input.Touch.position[i] = (rlVector2){ 0, 0 };

    // Register previous keys states
    // NOTE: Android supports up to 260 keys
    for (int i = 0; i < 260; i++)
    {
        rlCORE.Input.Keyboard.previousKeyState[i] = rlCORE.Input.Keyboard.currentKeyState[i];
        rlCORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    // TODO: Poll input events for current plaform
}


//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

// Initialize platform: graphics, inputs and more
int InitPlatform(void)
{
    // TODO: Initialize graphic device: display/window
    // It usually requires setting up the platform display system configuration
    // and connexion with the GPU through some system graphic API
    // raylib uses OpenGL so, platform should create that kind of connection
    // Below example illustrates that process using EGL library
    //----------------------------------------------------------------------------
    rlCORE.Window.fullscreen = true;
    rlCORE.Window.flags |= RL_FLAG_FULLSCREEN_MODE;

    EGLint samples = 0;
    EGLint sampleBuffer = 0;
    if (rlCORE.Window.flags & RL_FLAG_MSAA_4X_HINT)
    {
        samples = 4;
        sampleBuffer = 1;
        rlTRACELOG(RL_LOG_INFO, "DISPLAY: Trying to enable MSAA x4");
    }

    const EGLint framebufferAttribs[] =
    {
        EGL_RENDERABLE_TYPE, (rlglGetVersion() == RL_OPENGL_ES_30)? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT,      // Type of context support
        EGL_RED_SIZE, 8,            // RL_RED color bit depth (alternative: 5)
        EGL_GREEN_SIZE, 8,          // RL_GREEN color bit depth (alternative: 6)
        EGL_BLUE_SIZE, 8,           // RL_BLUE color bit depth (alternative: 5)
        //EGL_TRANSPARENT_TYPE, EGL_NONE, // Request transparent framebuffer (EGL_TRANSPARENT_RGB does not work on RPI)
        EGL_DEPTH_SIZE, 16,         // Depth buffer size (Required to use Depth testing!)
        //EGL_STENCIL_SIZE, 8,      // Stencil buffer size
        EGL_SAMPLE_BUFFERS, sampleBuffer,    // Activate MSAA
        EGL_SAMPLES, samples,       // 4x Antialiasing if activated (Free on MALI GPUs)
        EGL_NONE
    };

    const EGLint contextAttribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLint numConfigs = 0;

    // Get an EGL device connection
    platform.device = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (platform.device == EGL_NO_DISPLAY)
    {
        rlTRACELOG(RL_LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
        return false;
    }

    // Initialize the EGL device connection
    if (eglInitialize(platform.device, NULL, NULL) == EGL_FALSE)
    {
        // If all of the calls to eglInitialize returned EGL_FALSE then an error has occurred.
        rlTRACELOG(RL_LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
        return false;
    }

    // Get an appropriate EGL framebuffer configuration
    eglChooseConfig(platform.device, framebufferAttribs, &platform.config, 1, &numConfigs);

    // Set rendering API
    eglBindAPI(EGL_OPENGL_ES_API);

    // Create an EGL rendering context
    platform.context = eglCreateContext(platform.device, platform.config, EGL_NO_CONTEXT, contextAttribs);
    if (platform.context == EGL_NO_CONTEXT)
    {
        rlTRACELOG(RL_LOG_WARNING, "DISPLAY: Failed to create EGL context");
        return -1;
    }

    // Create an EGL window surface
    EGLint displayFormat = 0;

    // EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is guaranteed to be accepted by ANativeWindow_setBuffersGeometry()
    // As soon as we picked a EGLConfig, we can safely reconfigure the ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID
    eglGetConfigAttrib(platform.device, platform.config, EGL_NATIVE_VISUAL_ID, &displayFormat);

    // Android specific call
    ANativeWindow_setBuffersGeometry(platform.app->window, 0, 0, displayFormat);       // Force use of native display size

    platform.surface = eglCreateWindowSurface(platform.device, platform.config, platform.app->window, NULL);

    // There must be at least one frame displayed before the buffers are swapped
    eglSwapInterval(platform.device, 1);

    EGLBoolean result = eglMakeCurrent(platform.device, platform.surface, platform.surface, platform.context);

    // Check surface and context activation
    if (result != EGL_FALSE)
    {
        rlCORE.Window.ready = true;

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
    //----------------------------------------------------------------------------

    // If everything work as expected, we can continue
    rlCORE.Window.render.width = rlCORE.Window.screen.width;
    rlCORE.Window.render.height = rlCORE.Window.screen.height;
    rlCORE.Window.currentFbo.width = rlCORE.Window.render.width;
    rlCORE.Window.currentFbo.height = rlCORE.Window.render.height;

    rlTRACELOG(RL_LOG_INFO, "DISPLAY: Device initialized successfully");
    rlTRACELOG(RL_LOG_INFO, "    > Display size: %i x %i", rlCORE.Window.display.width, rlCORE.Window.display.height);
    rlTRACELOG(RL_LOG_INFO, "    > Screen size:  %i x %i", rlCORE.Window.screen.width, rlCORE.Window.screen.height);
    rlTRACELOG(RL_LOG_INFO, "    > Render size:  %i x %i", rlCORE.Window.render.width, rlCORE.Window.render.height);
    rlTRACELOG(RL_LOG_INFO, "    > Viewport offsets: %i, %i", rlCORE.Window.renderOffset.x, rlCORE.Window.renderOffset.y);

    // TODO: Load OpenGL extensions
    // NOTE: GL procedures address loader is required to load extensions
    //----------------------------------------------------------------------------
    rlglLoadExtensions(eglGetProcAddress);
    //----------------------------------------------------------------------------

    // TODO: Initialize input events system
    // It could imply keyboard, mouse, gamepad, touch...
    // Depending on the platform libraries/SDK it could use a callbacks mechanims
    // For system events and inputs evens polling on a per-frame basis, use rlPollInputEvents()
    //----------------------------------------------------------------------------
    // ...
    //----------------------------------------------------------------------------

    // TODO: Initialize timming system
    //----------------------------------------------------------------------------
    InitTimer();
    //----------------------------------------------------------------------------

    // TODO: Initialize storage system
    //----------------------------------------------------------------------------
    rlCORE.Storage.basePath = rlGetWorkingDirectory();
    //----------------------------------------------------------------------------

    rlTRACELOG(RL_LOG_INFO, "PLATFORM: CUSTOM: Initialized successfully");

    return 0;
}

// Close platform
void ClosePlatform(void)
{
    // TODO: De-initialize graphics, inputs and more
}

// EOF
