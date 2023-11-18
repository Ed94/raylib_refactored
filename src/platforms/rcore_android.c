/**********************************************************************************************
*
*   rcore_android - Functions to manage window, graphics device and inputs
*
*   PLATFORM: ANDROID
*       - Android (ARM, ARM64)
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
*       - Android NDK: Provides C API to access Android functionality
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

#include <android_native_app_glue.h>    // Required for: android_app struct and activity management
#include <android/window.h>             // Required for: AWINDOW_FLAG_FULLSCREEN definition and others
//#include <android/sensor.h>           // Required for: Android sensors functions (accelerometer, gyroscope, light...)
#include <jni.h>                        // Required for: JNIEnv and JavaVM [Used in rlOpenURL()]

#include <EGL/egl.h>                    // Native platform windowing system interface

#ifndef RL_NS_BEGIN
#define RL_NS_BEGIN
#define RL_NS_END
#endif

RL_NS_BEGIN

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    // Application data
    struct android_app *app;            // Android activity
    struct android_poll_source *source; // Android events polling source
    bool appEnabled;                    // Flag to detect if app is active ** = true
    bool contextRebindRequired;         // Used to know context rebind required

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
void ClosePlatform(void);        // Close platform

static void AndroidCommandCallback(struct android_app *app, int32_t cmd);           // Process Android activity lifecycle commands
static int32_t AndroidInputCallback(struct android_app *app, AInputEvent *event);   // Process Android inputs
static rlGamepadButton AndroidTranslateGamepadButton(int button);                     // Map Android gamepad button to raylib gamepad button

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
// NOTE: Functions declaration is provided by raylib.h

//----------------------------------------------------------------------------------
// Module Functions Definition: Application
//----------------------------------------------------------------------------------

// To allow easier porting to android, we allow the user to define a
// main function which we call from android_main, defined by ourselves
extern int main(int argc, char *argv[]);

// Android main function
void android_main(struct android_app *app)
{
    char arg0[] = "raylib";     // NOTE: argv[] are mutable
    platform.app = app;

    // NOTE: Return from main is ignored
    (void)main(1, (char *[]) { arg0, NULL });

    // Request to end the native activity
    ANativeActivity_finish(app->activity);

    // Android ALooper_pollAll() variables
    int pollResult = 0;
    int pollEvents = 0;

    // Waiting for application events before complete finishing
    while (!app->destroyRequested)
    {
        while ((pollResult = ALooper_pollAll(0, NULL, &pollEvents, (void **)&platform.source)) >= 0)
        {
            if (platform.source != NULL) platform.source->process(app, platform.source);
        }
    }
}

// NOTE: Add this to header (if apps really need it)
struct android_app *GetAndroidApp(void)
{
    return platform.app;
}

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
    // Security check to (partially) avoid malicious code
    if (strchr(url, '\'') != NULL) rlTRACELOG(RL_LOG_WARNING, "SYSTEM: Provided URL could be potentially malicious, avoid [\'] character");
    else
    {
        JNIEnv *env = NULL;
        JavaVM *vm = platform.app->activity->vm;
        (*vm)->AttachCurrentThread(vm, &env, NULL);

        jstring urlString = (*env)->NewStringUTF(env, url);
        jclass uriClass = (*env)->FindClass(env, "android/net/Uri");
        jmethodID uriParse = (*env)->GetStaticMethodID(env, uriClass, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
        jobject uri = (*env)->CallStaticObjectMethod(env, uriClass, uriParse, urlString);

        jclass intentClass = (*env)->FindClass(env, "android/content/Intent");
        jfieldID actionViewId = (*env)->GetStaticFieldID(env, intentClass, "ACTION_VIEW", "Ljava/lang/String;");
        jobject actionView = (*env)->GetStaticObjectField(env, intentClass, actionViewId);
        jmethodID newIntent = (*env)->GetMethodID(env, intentClass, "<init>", "(Ljava/lang/String;Landroid/net/Uri;)V");
        jobject intent = (*env)->AllocObject(env, intentClass);

        (*env)->CallVoidMethod(env, intent, newIntent, actionView, uri);
        jclass activityClass = (*env)->FindClass(env, "android/app/Activity");
        jmethodID startActivity = (*env)->GetMethodID(env, activityClass, "startActivity", "(Landroid/content/Intent;)V");
        (*env)->CallVoidMethod(env, platform.app->activity->clazz, startActivity, intent);

        (*vm)->DetachCurrentThread(vm);
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
    rlCORE.Input.Gamepad.lastButtonPressed = 0;       // RL_GAMEPAD_BUTTON_UNKNOWN
    //rlCORE.Input.Gamepad.axisCount = 0;

    // Register previous touch states
    for (int i = 0; i < RL_MAX_TOUCH_POINTS; i++) rlCORE.Input.Touch.previousTouchState[i] = rlCORE.Input.Touch.currentTouchState[i];

    // Reset touch positions
    //for (int i = 0; i < RL_MAX_TOUCH_POINTS; i++) rlCORE.Input.Touch.position[i] = (rlVector2){ 0, 0 };

    // Register previous keys states
    // NOTE: Android supports up to 260 keys
    for (int i = 0; i < 260; i++)
    {
        rlCORE.Input.Keyboard.previousKeyState[i] = rlCORE.Input.Keyboard.currentKeyState[i];
        rlCORE.Input.Keyboard.keyRepeatInFrame[i] = 0;
    }

    // Android ALooper_pollAll() variables
    int pollResult = 0;
    int pollEvents = 0;

    // Poll Events (registered events)
    // NOTE: Activity is paused if not enabled (platform.appEnabled)
    while ((pollResult = ALooper_pollAll(platform.appEnabled? 0 : -1, NULL, &pollEvents, (void**)&platform.source)) >= 0)
    {
        // Process this event
        if (platform.source != NULL) platform.source->process(platform.app, platform.source);

        // NOTE: Never close window, native activity is controlled by the system!
        if (platform.app->destroyRequested != 0)
        {
            //rlCORE.Window.shouldClose = true;
            //ANativeActivity_finish(platform.app->activity);
        }
    }
}


//----------------------------------------------------------------------------------
// Module Internal Functions Definition
//----------------------------------------------------------------------------------

// Initialize platform: graphics, inputs and more
int InitPlatform(void)
{
    // Initialize display basic configuration
    //----------------------------------------------------------------------------
    rlCORE.Window.currentFbo.width = rlCORE.Window.screen.width;
    rlCORE.Window.currentFbo.height = rlCORE.Window.screen.height;

    // Set desired windows flags before initializing anything
    ANativeActivity_setWindowFlags(platform.app->activity, AWINDOW_FLAG_FULLSCREEN, 0);  //AWINDOW_FLAG_SCALED, AWINDOW_FLAG_DITHER

    int orientation = AConfiguration_getOrientation(platform.app->config);

    if (orientation == ACONFIGURATION_ORIENTATION_PORT) rlTRACELOG(RL_LOG_INFO, "ANDROID: Window orientation set as portrait");
    else if (orientation == ACONFIGURATION_ORIENTATION_LAND) rlTRACELOG(RL_LOG_INFO, "ANDROID: Window orientation set as landscape");

    // TODO: Automatic orientation doesn't seem to work
    if (rlCORE.Window.screen.width <= rlCORE.Window.screen.height)
    {
        AConfiguration_setOrientation(platform.app->config, ACONFIGURATION_ORIENTATION_PORT);
        rlTRACELOG(RL_LOG_WARNING, "ANDROID: Window orientation changed to portrait");
    }
    else
    {
        AConfiguration_setOrientation(platform.app->config, ACONFIGURATION_ORIENTATION_LAND);
        rlTRACELOG(RL_LOG_WARNING, "ANDROID: Window orientation changed to landscape");
    }

    //AConfiguration_getDensity(platform.app->config);
    //AConfiguration_getKeyboard(platform.app->config);
    //AConfiguration_getScreenSize(platform.app->config);
    //AConfiguration_getScreenLong(platform.app->config);

    // Set some default window flags
    rlCORE.Window.flags &= ~RL_FLAG_WINDOW_HIDDEN;       // false
    rlCORE.Window.flags &= ~RL_FLAG_WINDOW_MINIMIZED;    // false
    rlCORE.Window.flags |= RL_FLAG_WINDOW_MAXIMIZED;     // true
    rlCORE.Window.flags &= ~RL_FLAG_WINDOW_UNFOCUSED;    // false
    //----------------------------------------------------------------------------

    // Initialize App command system
    // NOTE: On APP_CMD_INIT_WINDOW -> InitGraphicsDevice(), InitTimer(), LoadFontDefault()...
    //----------------------------------------------------------------------------
    platform.app->onAppCmd = AndroidCommandCallback;
    //----------------------------------------------------------------------------

    // Initialize input events system
    //----------------------------------------------------------------------------
    platform.app->onInputEvent = AndroidInputCallback;
    //----------------------------------------------------------------------------

    // Initialize storage system
    //----------------------------------------------------------------------------
    InitAssetManager(platform.app->activity->assetManager, platform.app->activity->internalDataPath);   // Initialize assets manager

    rlCORE.Storage.basePath = platform.app->activity->internalDataPath;   // Define base path for storage
    //----------------------------------------------------------------------------

    rlTRACELOG(RL_LOG_INFO, "PLATFORM: ANDROID: Initialized successfully");

    // Android ALooper_pollAll() variables
    int pollResult = 0;
    int pollEvents = 0;

    // Wait for window to be initialized (display and context)
    while (!rlCORE.Window.ready)
    {
        // Process events loop
        while ((pollResult = ALooper_pollAll(0, NULL, &pollEvents, (void**)&platform.source)) >= 0)
        {
            // Process this event
            if (platform.source != NULL) platform.source->process(platform.app, platform.source);

            // NOTE: Never close window, native activity is controlled by the system!
            //if (platform.app->destroyRequested != 0) rlCORE.Window.shouldClose = true;
        }
    }

    return 0;
}

// Close platform
void ClosePlatform(void)
{
    // Close surface, context and display
    if (platform.device != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(platform.device, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (platform.surface != EGL_NO_SURFACE)
        {
            eglDestroySurface(platform.device, platform.surface);
            platform.surface = EGL_NO_SURFACE;
        }

        if (platform.context != EGL_NO_CONTEXT)
        {
            eglDestroyContext(platform.device, platform.context);
            platform.context = EGL_NO_CONTEXT;
        }

        eglTerminate(platform.device);
        platform.device = EGL_NO_DISPLAY;
    }
}

// Initialize display device and framebuffer
// NOTE: width and height represent the screen (framebuffer) desired size, not actual display size
// If width or height are 0, default display size will be used for framebuffer size
// NOTE: returns false in case graphic device could not be created
static int InitGraphicsDevice(void)
{
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
        return -1;
    }

    // Initialize the EGL device connection
    if (eglInitialize(platform.device, NULL, NULL) == EGL_FALSE)
    {
        // If all of the calls to eglInitialize returned EGL_FALSE then an error has occurred.
        rlTRACELOG(RL_LOG_WARNING, "DISPLAY: Failed to initialize EGL device");
        return -1;
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
    //---------------------------------------------------------------------------------
    EGLint displayFormat = 0;

    // EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is guaranteed to be accepted by ANativeWindow_setBuffersGeometry()
    // As soon as we picked a EGLConfig, we can safely reconfigure the ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID
    eglGetConfigAttrib(platform.device, platform.config, EGL_NATIVE_VISUAL_ID, &displayFormat);

    // At this point we need to manage render size vs screen size
    // NOTE: This function use and modify global module variables:
    //  -> rlCORE.Window.screen.width/rlCORE.Window.screen.height
    //  -> rlCORE.Window.render.width/rlCORE.Window.render.height
    //  -> rlCORE.Window.screenScale
    SetupFramebuffer(rlCORE.Window.display.width, rlCORE.Window.display.height);

    ANativeWindow_setBuffersGeometry(platform.app->window, rlCORE.Window.render.width, rlCORE.Window.render.height, displayFormat);
    //ANativeWindow_setBuffersGeometry(platform.app->window, 0, 0, displayFormat);       // Force use of native display size

    platform.surface = eglCreateWindowSurface(platform.device, platform.config, platform.app->window, NULL);

    // There must be at least one frame displayed before the buffers are swapped
    //eglSwapInterval(platform.device, 1);

    if (eglMakeCurrent(platform.device, platform.surface, platform.surface, platform.context) == EGL_FALSE)
    {
        rlTRACELOG(RL_LOG_WARNING, "DISPLAY: Failed to attach EGL rendering context to EGL surface");
        return -1;
    }
    else
    {
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

    // Load OpenGL extensions
    // NOTE: GL procedures address loader is required to load extensions
    rlglLoadExtensions(eglGetProcAddress);

    rlCORE.Window.ready = true;

    if ((rlCORE.Window.flags & RL_FLAG_WINDOW_MINIMIZED) > 0) rlMinimizeWindow();

    return 0;
}

// ANDROID: Process activity lifecycle commands
static void AndroidCommandCallback(struct android_app *app, int32_t cmd)
{
    switch (cmd)
    {
        case APP_CMD_START:
        {
            //rendering = true;
        } break;
        case APP_CMD_RESUME: break;
        case APP_CMD_INIT_WINDOW:
        {
            if (app->window != NULL)
            {
                if (platform.contextRebindRequired)
                {
                    // Reset screen scaling to full display size
                    EGLint displayFormat = 0;
                    eglGetConfigAttrib(platform.device, platform.config, EGL_NATIVE_VISUAL_ID, &displayFormat);

                    // Adding renderOffset here feels rather hackish, but the viewport scaling is wrong after the
                    // context rebinding if the screen is scaled unless offsets are added. There's probably a more
                    // appropriate way to fix this
                    ANativeWindow_setBuffersGeometry(app->window,
                        rlCORE.Window.render.width + rlCORE.Window.renderOffset.x,
                        rlCORE.Window.render.height + rlCORE.Window.renderOffset.y,
                        displayFormat);

                    // Recreate display surface and re-attach OpenGL context
                    platform.surface = eglCreateWindowSurface(platform.device, platform.config, app->window, NULL);
                    eglMakeCurrent(platform.device, platform.surface, platform.surface, platform.context);

                    platform.contextRebindRequired = false;
                }
                else
                {
                    rlCORE.Window.display.width = ANativeWindow_getWidth(platform.app->window);
                    rlCORE.Window.display.height = ANativeWindow_getHeight(platform.app->window);

                    // Initialize graphics device (display device and OpenGL context)
                    InitGraphicsDevice();

                    // Initialize OpenGL context (states and resources)
                    // NOTE: rlCORE.Window.currentFbo.width and rlCORE.Window.currentFbo.height not used, just stored as globals in rlgl
                    rlglInit(rlCORE.Window.currentFbo.width, rlCORE.Window.currentFbo.height);

                    // Setup default viewport
                    // NOTE: It updated rlCORE.Window.render.width and rlCORE.Window.render.height
                    SetupViewport(rlCORE.Window.currentFbo.width, rlCORE.Window.currentFbo.height);

                    // Initialize hi-res timer
                    InitTimer();

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
                        rlSetShapesTexture(rlGetFontDefault().texture, (rlRectangle){ rec.x + 2, rec.y + 2, 1, 1 });
                    }
                    else
                    {
                        // NOTE: We set up a 1px padding on char rectangle to avoid pixel bleeding
                        rlSetShapesTexture(rlGetFontDefault().texture, (rlRectangle){ rec.x + 1, rec.y + 1, rec.width - 2, rec.height - 2 });
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

                    // Initialize random seed
                    rlSetRandomSeed((unsigned int)time(NULL));

                    // TODO: GPU assets reload in case of lost focus (lost context)
                    // NOTE: This problem has been solved just unbinding and rebinding context from display
                    /*
                    if (assetsReloadRequired)
                    {
                        for (int i = 0; i < assetCount; i++)
                        {
                            // TODO: Unload old asset if required

                            // Load texture again to pointed texture
                            (*textureAsset + i) = rlLoadTexture(assetPath[i]);
                        }
                    }
                    */
                }
            }
        } break;
        case APP_CMD_GAINED_FOCUS:
        {
            platform.appEnabled = true;
            rlCORE.Window.flags &= ~RL_FLAG_WINDOW_UNFOCUSED;
            //rlResumeMusicStream();
        } break;
        case APP_CMD_PAUSE: break;
        case APP_CMD_LOST_FOCUS:
        {
            platform.appEnabled = false;
            rlCORE.Window.flags |= RL_FLAG_WINDOW_UNFOCUSED;
            //rlPauseMusicStream();
        } break;
        case APP_CMD_TERM_WINDOW:
        {
            // Detach OpenGL context and destroy display surface
            // NOTE 1: This case is used when the user exits the app without closing it. We detach the context to ensure everything is recoverable upon resuming.
            // NOTE 2: Detaching context before destroying display surface avoids losing our resources (textures, shaders, VBOs...)
            // NOTE 3: In some cases (too many context loaded), OS could unload context automatically... :(
            if (platform.device != EGL_NO_DISPLAY)
            {
                eglMakeCurrent(platform.device, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

                if (platform.surface != EGL_NO_SURFACE)
                {
                    eglDestroySurface(platform.device, platform.surface);
                    platform.surface = EGL_NO_SURFACE;
                }

                platform.contextRebindRequired = true;
            }
            // If 'platform.device' is already set to 'EGL_NO_DISPLAY'
            // this means that the user has already called 'rlCloseWindow()'

        } break;
        case APP_CMD_SAVE_STATE: break;
        case APP_CMD_STOP: break;
        case APP_CMD_DESTROY: break;
        case APP_CMD_CONFIG_CHANGED:
        {
            //AConfiguration_fromAssetManager(platform.app->config, platform.app->activity->assetManager);
            //print_cur_config(platform.app);

            // Check screen orientation here!
        } break;
        default: break;
    }
}

// ANDROID: Map Android gamepad button to raylib gamepad button
static rlGamepadButton AndroidTranslateGamepadButton(int button)
{
    switch (button)
    {
        case AKEYCODE_BUTTON_A: return RL_GAMEPAD_BUTTON_RIGHT_FACE_DOWN;
        case AKEYCODE_BUTTON_B: return RL_GAMEPAD_BUTTON_RIGHT_FACE_RIGHT;
        case AKEYCODE_BUTTON_X: return RL_GAMEPAD_BUTTON_RIGHT_FACE_LEFT;
        case AKEYCODE_BUTTON_Y: return RL_GAMEPAD_BUTTON_RIGHT_FACE_UP;
        case AKEYCODE_BUTTON_L1: return RL_GAMEPAD_BUTTON_LEFT_TRIGGER_1;
        case AKEYCODE_BUTTON_R1: return RL_GAMEPAD_BUTTON_RIGHT_TRIGGER_1;
        case AKEYCODE_BUTTON_L2: return RL_GAMEPAD_BUTTON_LEFT_TRIGGER_2;
        case AKEYCODE_BUTTON_R2: return RL_GAMEPAD_BUTTON_RIGHT_TRIGGER_2;
        case AKEYCODE_BUTTON_THUMBL: return RL_GAMEPAD_BUTTON_LEFT_THUMB;
        case AKEYCODE_BUTTON_THUMBR: return RL_GAMEPAD_BUTTON_RIGHT_THUMB;
        case AKEYCODE_BUTTON_START: return RL_GAMEPAD_BUTTON_MIDDLE_RIGHT;
        case AKEYCODE_BUTTON_SELECT: return RL_GAMEPAD_BUTTON_MIDDLE_LEFT;
        case AKEYCODE_BUTTON_MODE: return RL_GAMEPAD_BUTTON_MIDDLE;
        // On some (most?) gamepads dpad events are reported as axis motion instead
        case AKEYCODE_DPAD_DOWN: return RL_GAMEPAD_BUTTON_LEFT_FACE_DOWN;
        case AKEYCODE_DPAD_RIGHT: return RL_GAMEPAD_BUTTON_LEFT_FACE_RIGHT;
        case AKEYCODE_DPAD_LEFT: return RL_GAMEPAD_BUTTON_LEFT_FACE_LEFT;
        case AKEYCODE_DPAD_UP: return RL_GAMEPAD_BUTTON_LEFT_FACE_UP;
        default: return RL_GAMEPAD_BUTTON_UNKNOWN;
    }
}

// ANDROID: Get input events
static int32_t AndroidInputCallback(struct android_app *app, AInputEvent *event)
{
    // If additional inputs are required check:
    // https://developer.android.com/ndk/reference/group/input
    // https://developer.android.com/training/game-controllers/controller-input

    int type = AInputEvent_getType(event);
    int source = AInputEvent_getSource(event);

    if (type == AINPUT_EVENT_TYPE_MOTION)
    {
        if (((source & AINPUT_SOURCE_JOYSTICK) == AINPUT_SOURCE_JOYSTICK) ||
            ((source & AINPUT_SOURCE_GAMEPAD) == AINPUT_SOURCE_GAMEPAD))
        {
            // For now we'll assume a single gamepad which we "detect" on its input event
            rlCORE.Input.Gamepad.ready[0] = true;

            rlCORE.Input.Gamepad.axisState[0][RL_GAMEPAD_AXIS_LEFT_X] = AMotionEvent_getAxisValue(
                    event, AMOTION_EVENT_AXIS_X, 0);
            rlCORE.Input.Gamepad.axisState[0][RL_GAMEPAD_AXIS_LEFT_Y] = AMotionEvent_getAxisValue(
                    event, AMOTION_EVENT_AXIS_Y, 0);
            rlCORE.Input.Gamepad.axisState[0][RL_GAMEPAD_AXIS_RIGHT_X] = AMotionEvent_getAxisValue(
                    event, AMOTION_EVENT_AXIS_Z, 0);
            rlCORE.Input.Gamepad.axisState[0][RL_GAMEPAD_AXIS_RIGHT_Y] = AMotionEvent_getAxisValue(
                    event, AMOTION_EVENT_AXIS_RZ, 0);
            rlCORE.Input.Gamepad.axisState[0][RL_GAMEPAD_AXIS_LEFT_TRIGGER] = AMotionEvent_getAxisValue(
                    event, AMOTION_EVENT_AXIS_BRAKE, 0) * 2.0f - 1.0f;
            rlCORE.Input.Gamepad.axisState[0][RL_GAMEPAD_AXIS_RIGHT_TRIGGER] = AMotionEvent_getAxisValue(
                    event, AMOTION_EVENT_AXIS_GAS, 0) * 2.0f - 1.0f;

            // dpad is reported as an axis on android
            float dpadX = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_X, 0);
            float dpadY = AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_Y, 0);

            if (dpadX == 1.0f)
            {
                rlCORE.Input.Gamepad.currentButtonState[0][RL_GAMEPAD_BUTTON_LEFT_FACE_RIGHT] = 1;
                rlCORE.Input.Gamepad.currentButtonState[0][RL_GAMEPAD_BUTTON_LEFT_FACE_LEFT] = 0;
            }
            else if (dpadX == -1.0f)
            {
                rlCORE.Input.Gamepad.currentButtonState[0][RL_GAMEPAD_BUTTON_LEFT_FACE_RIGHT] = 0;
                rlCORE.Input.Gamepad.currentButtonState[0][RL_GAMEPAD_BUTTON_LEFT_FACE_LEFT] = 1;
            }
            else
            {
                rlCORE.Input.Gamepad.currentButtonState[0][RL_GAMEPAD_BUTTON_LEFT_FACE_RIGHT] = 0;
                rlCORE.Input.Gamepad.currentButtonState[0][RL_GAMEPAD_BUTTON_LEFT_FACE_LEFT] = 0;
            }

            if (dpadY == 1.0f)
            {
                rlCORE.Input.Gamepad.currentButtonState[0][RL_GAMEPAD_BUTTON_LEFT_FACE_DOWN] = 1;
                rlCORE.Input.Gamepad.currentButtonState[0][RL_GAMEPAD_BUTTON_LEFT_FACE_UP] = 0;
            }
            else if (dpadY == -1.0f)
            {
                rlCORE.Input.Gamepad.currentButtonState[0][RL_GAMEPAD_BUTTON_LEFT_FACE_DOWN] = 0;
                rlCORE.Input.Gamepad.currentButtonState[0][RL_GAMEPAD_BUTTON_LEFT_FACE_UP] = 1;
            }
            else
            {
                rlCORE.Input.Gamepad.currentButtonState[0][RL_GAMEPAD_BUTTON_LEFT_FACE_DOWN] = 0;
                rlCORE.Input.Gamepad.currentButtonState[0][RL_GAMEPAD_BUTTON_LEFT_FACE_UP] = 0;
            }

            return 1; // Handled gamepad axis motion
        }
    }
    else if (type == AINPUT_EVENT_TYPE_KEY)
    {
        int32_t keycode = AKeyEvent_getKeyCode(event);
        //int32_t AKeyEvent_getMetaState(event);

        // Handle gamepad button presses and releases
        if (((source & AINPUT_SOURCE_JOYSTICK) == AINPUT_SOURCE_JOYSTICK) ||
            ((source & AINPUT_SOURCE_GAMEPAD) == AINPUT_SOURCE_GAMEPAD))
        {
            // For now we'll assume a single gamepad which we "detect" on its input event
            rlCORE.Input.Gamepad.ready[0] = true;

            rlGamepadButton button = AndroidTranslateGamepadButton(keycode);

            if (button == RL_GAMEPAD_BUTTON_UNKNOWN) return 1;

            if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN)
            {
                rlCORE.Input.Gamepad.currentButtonState[0][button] = 1;
            }
            else rlCORE.Input.Gamepad.currentButtonState[0][button] = 0;  // Key up

            return 1; // Handled gamepad button
        }

        // Save current button and its state
        // NOTE: Android key action is 0 for down and 1 for up
        if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_DOWN)
        {
            rlCORE.Input.Keyboard.currentKeyState[keycode] = 1;   // Key down

            rlCORE.Input.Keyboard.keyPressedQueue[rlCORE.Input.Keyboard.keyPressedQueueCount] = keycode;
            rlCORE.Input.Keyboard.keyPressedQueueCount++;
        }
        else if (AKeyEvent_getAction(event) == AKEY_EVENT_ACTION_MULTIPLE) rlCORE.Input.Keyboard.keyRepeatInFrame[keycode] = 1;
        else rlCORE.Input.Keyboard.currentKeyState[keycode] = 0;  // Key up

        if (keycode == AKEYCODE_POWER)
        {
            // Let the OS handle input to avoid app stuck. Behaviour: CMD_PAUSE -> CMD_SAVE_STATE -> CMD_STOP -> CMD_CONFIG_CHANGED -> CMD_LOST_FOCUS
            // Resuming Behaviour: CMD_START -> CMD_RESUME -> CMD_CONFIG_CHANGED -> CMD_CONFIG_CHANGED -> CMD_GAINED_FOCUS
            // It seems like locking mobile, screen size (CMD_CONFIG_CHANGED) is affected.
            // NOTE: AndroidManifest.xml must have <activity android:configChanges="orientation|keyboardHidden|screenSize" >
            // Before that change, activity was calling CMD_TERM_WINDOW and CMD_DESTROY when locking mobile, so that was not a normal behaviour
            return 0;
        }
        else if ((keycode == AKEYCODE_BACK) || (keycode == AKEYCODE_MENU))
        {
            // Eat BACK_BUTTON and AKEYCODE_MENU, just do nothing... and don't let to be handled by OS!
            return 1;
        }
        else if ((keycode == AKEYCODE_VOLUME_UP) || (keycode == AKEYCODE_VOLUME_DOWN))
        {
            // Set default OS behaviour
            return 0;
        }

        return 0;
    }

    // Register touch points count
    rlCORE.Input.Touch.pointCount = AMotionEvent_getPointerCount(event);

    for (int i = 0; (i < rlCORE.Input.Touch.pointCount) && (i < RL_MAX_TOUCH_POINTS); i++)
    {
        // Register touch points id
        rlCORE.Input.Touch.pointId[i] = AMotionEvent_getPointerId(event, i);

        // Register touch points position
        rlCORE.Input.Touch.position[i] = (rlVector2){ AMotionEvent_getX(event, i), AMotionEvent_getY(event, i) };

        // rlNormalize rlCORE.Input.Touch.position[i] for rlCORE.Window.screen.width and rlCORE.Window.screen.height
        float widthRatio = (float)(rlCORE.Window.screen.width + rlCORE.Window.renderOffset.x) / (float)rlCORE.Window.display.width;
        float heightRatio = (float)(rlCORE.Window.screen.height + rlCORE.Window.renderOffset.y) / (float)rlCORE.Window.display.height;
        rlCORE.Input.Touch.position[i].x = rlCORE.Input.Touch.position[i].x * widthRatio - (float)rlCORE.Window.renderOffset.x / 2;
        rlCORE.Input.Touch.position[i].y = rlCORE.Input.Touch.position[i].y * heightRatio - (float)rlCORE.Window.renderOffset.y / 2;
    }

    int32_t action = AMotionEvent_getAction(event);
    unsigned int flags = action & AMOTION_EVENT_ACTION_MASK;

#if defined(RL_SUPPORT_GESTURES_SYSTEM)
    rlGestureEvent gestureEvent = { 0 };

    gestureEvent.pointCount = rlCORE.Input.Touch.pointCount;

    // Register touch actions
    if (flags == AMOTION_EVENT_ACTION_DOWN) gestureEvent.touchAction = TOUCH_ACTION_DOWN;
    else if (flags == AMOTION_EVENT_ACTION_UP) gestureEvent.touchAction = TOUCH_ACTION_UP;
    else if (flags == AMOTION_EVENT_ACTION_MOVE) gestureEvent.touchAction = TOUCH_ACTION_MOVE;
    else if (flags == AMOTION_EVENT_ACTION_CANCEL) gestureEvent.touchAction = TOUCH_ACTION_CANCEL;

    for (int i = 0; (i < gestureEvent.pointCount) && (i < RL_MAX_TOUCH_POINTS); i++)
    {
        gestureEvent.pointId[i] = rlCORE.Input.Touch.pointId[i];
        gestureEvent.position[i] = rlCORE.Input.Touch.position[i];
        gestureEvent.position[i].x /= (float)rlGetScreenWidth();
        gestureEvent.position[i].y /= (float)rlGetScreenHeight();
    }

    // rlGesture data is sent to gestures system for processing
    rlProcessGestureEvent(gestureEvent);
#endif

    int32_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

    if (flags == AMOTION_EVENT_ACTION_POINTER_UP || flags == AMOTION_EVENT_ACTION_UP)
    {
        // One of the touchpoints is released, remove it from touch point arrays
        for (int i = pointerIndex; (i < rlCORE.Input.Touch.pointCount - 1) && (i < RL_MAX_TOUCH_POINTS); i++)
        {
            rlCORE.Input.Touch.pointId[i] = rlCORE.Input.Touch.pointId[i+1];
            rlCORE.Input.Touch.position[i] = rlCORE.Input.Touch.position[i+1];
        }

        rlCORE.Input.Touch.pointCount--;
    }

    // When all touchpoints are tapped and released really quickly, this event is generated
    if (flags == AMOTION_EVENT_ACTION_CANCEL) rlCORE.Input.Touch.pointCount = 0;

    if (rlCORE.Input.Touch.pointCount > 0) rlCORE.Input.Touch.currentTouchState[RL_MOUSE_BUTTON_LEFT] = 1;
    else rlCORE.Input.Touch.currentTouchState[RL_MOUSE_BUTTON_LEFT] = 0;

    // Stores the previous position of touch[0] only while it's active to calculate the delta.
    if (flags == AMOTION_EVENT_ACTION_MOVE)
    {
        rlCORE.Input.Mouse.previousPosition = rlCORE.Input.Mouse.currentPosition;
    }
    else
    {
        rlCORE.Input.Mouse.previousPosition = rlCORE.Input.Touch.position[0];
    }

    // Map touch[0] as mouse input for convenience
    rlCORE.Input.Mouse.currentPosition = rlCORE.Input.Touch.position[0];
    rlCORE.Input.Mouse.currentWheelMove = (rlVector2){ 0.0f, 0.0f };

    return 0;
}

RL_NS_END

// EOF
