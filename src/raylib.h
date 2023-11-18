/**********************************************************************************************
*
*   raylib v5.0 - A simple and easy-to-use library to enjoy videogames programming (www.raylib.com)
*
*   FEATURES:
*       - NO external dependencies, all required libraries included with raylib
*       - Multiplatform: Windows, Linux, FreeBSD, OpenBSD, NetBSD, DragonFly,
*                        MacOS, Haiku, Android, Raspberry Pi, DRM native, HTML5.
*       - Written in plain C code (C99) in PascalCase/camelCase notation
*       - Hardware accelerated with OpenGL (1.1, 2.1, 3.3, 4.3 or ES2 - choose at compile)
*       - Unique OpenGL abstraction layer (usable as standalone module): [rlgl]
*       - Multiple Fonts formats supported (TTF, XNA fonts, AngelCode fonts)
*       - Outstanding texture formats support, including compressed formats (DXT, ETC, ASTC)
*       - Full 3d support for 3d Shapes, Models, Billboards, Heightmaps and more!
*       - Flexible Materials system, supporting classic maps and PBR maps
*       - Animated 3D models supported (skeletal bones animation) (IQM)
*       - Shaders support, including rlModel shaders and Postprocessing shaders
*       - Powerful math module for Vector, rlMatrix and rlQuaternion operations: [raymath]
*       - Audio loading and playing with streaming support (WAV, OGG, MP3, FLAC, XM, MOD)
*       - VR stereo rendering with configurable HMD device parameters
*       - Bindings to multiple programming languages available!
*
*   NOTES:
*       - One default rlFont is loaded on rlInitWindow()->LoadFontDefault() [core, text]
*       - One default rlTexture2D is loaded on rlglInit(), 1x1 white pixel R8G8B8A8 [rlgl] (OpenGL 3.3 or ES2)
*       - One default rlShader is loaded on rlglInit()->rlglLoadShaderDefault() [rlgl] (OpenGL 3.3 or ES2)
*       - One default RenderBatch is loaded on rlglInit()->rlglLoadRenderBatch() [rlgl] (OpenGL 3.3 or ES2)
*
*   DEPENDENCIES (included):
*       [rcore] rglfw (Camilla Löwy - github.com/glfw/glfw) for window/context management and input (PLATFORM_DESKTOP)
*       [rlgl] glad (David Herberth - github.com/Dav1dde/glad) for OpenGL 3.3 extensions loading (PLATFORM_DESKTOP)
*       [raudio] miniaudio (David Reid - github.com/mackron/miniaudio) for audio device/context management
*
*   OPTIONAL DEPENDENCIES (included):
*       [rcore] msf_gif (Miles Fogle) for GIF recording
*       [rcore] sinfl (Micha Mettke) for DEFLATE decompression algorithm
*       [rcore] sdefl (Micha Mettke) for DEFLATE compression algorithm
*       [rtextures] stb_image (Sean Barret) for images loading (BMP, TGA, PNG, JPEG, HDR...)
*       [rtextures] stb_image_write (Sean Barret) for image writing (BMP, TGA, PNG, JPG)
*       [rtextures] stb_image_resize (Sean Barret) for image resizing algorithms
*       [rtext] stb_truetype (Sean Barret) for ttf fonts loading
*       [rtext] stb_rect_pack (Sean Barret) for rectangles packing
*       [rmodels] par_shapes (Philip Rideout) for parametric 3d shapes generation
*       [rmodels] tinyobj_loader_c (Syoyo Fujita) for models loading (OBJ, MTL)
*       [rmodels] cgltf (Johannes Kuhlmann) for models loading (glTF)
*       [rmodels] Model3D (bzt) for models loading (M3D, https://bztsrc.gitlab.io/model3d)
*       [raudio] dr_wav (David Reid) for WAV audio file loading
*       [raudio] dr_flac (David Reid) for FLAC audio file loading
*       [raudio] dr_mp3 (David Reid) for MP3 audio file loading
*       [raudio] stb_vorbis (Sean Barret) for OGG audio loading
*       [raudio] jar_xm (Joshua Reisenauer) for XM audio module loading
*       [raudio] jar_mod (Joshua Reisenauer) for MOD audio module loading
*
*
*   LICENSE: zlib/libpng
*
*   raylib is licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software:
*
*   Copyright (c) 2013-2023 Ramon Santamaria (@raysan5)
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

#ifndef RAYLIB_H
#define RAYLIB_H

#include <stdarg.h>     // Required for: va_list - Only used by rlTraceLogCallback

#define RAYLIB_VERSION_MAJOR 5
#define RAYLIB_VERSION_MINOR 0
#define RAYLIB_VERSION_PATCH 0
#define RAYLIB_VERSION  "5.0"

// Function specifiers in case library is build/used as a shared library (Windows)
// NOTE: Microsoft specifiers to tell compiler that symbols are imported/exported from a .dll
#if defined(_WIN32)
    #if defined(RL_BUILD_LIBTYPE_SHARED)
        #if defined(__TINYC__)
            #define __declspec(x) __attribute__((x))
        #endif
        #define RLAPI __declspec(dllexport)     // We are building the library as a Win32 shared library (.dll)
    #elif defined(RL_USE_LIBTYPE_SHARED)
        #define RLAPI __declspec(dllimport)     // We are using the library as a Win32 shared library (.dll)
    #endif
#endif

#ifndef RLAPI
    #define RLAPI       // Functions defined as 'extern' by default (implicit specifiers)
#endif

//----------------------------------------------------------------------------------
// Some basic Defines
//----------------------------------------------------------------------------------
#ifndef RL_PI
    #define RL_PI 3.14159265358979323846f
#endif
#ifndef RL_DEG2RAD
    #define RL_DEG2RAD (RL_PI/180.0f)
#endif
#ifndef RL_RAD2DEG
    #define RL_RAD2DEG (180.0f/RL_PI)
#endif

// Allow custom memory allocators
// NOTE: Require recompiling raylib sources
#ifndef RL_MALLOC
    #define RL_MALLOC(sz)       malloc(sz)
#endif
#ifndef RL_CALLOC
    #define RL_CALLOC(n,sz)     calloc(n,sz)
#endif
#ifndef RL_REALLOC
    #define RL_REALLOC(ptr,sz)  realloc(ptr,sz)
#endif
#ifndef RL_FREE
    #define RL_FREE(ptr)        free(ptr)
#endif

// NOTE: MSVC C++ compiler does not support compound literals (C99 feature)
// Plain structures in C++ (without constructors) can be initialized with { }
// This is called aggregate initialization (C++11 feature)
#if defined(__cplusplus)
    #define RL_CLITERAL(type)      type
#else
    #define RL_CLITERAL(type)      (type)
#endif

// Used for initializer nonsense on cpp.
#if __cplusplus
#define CAST(type) type
#else
#define CAST(type) (type)
#endif


// Some compilers (mostly macos clang) default to C++98,
// where aggregate initialization can't be used
// So, give a more clear error stating how to fix this
#if !defined(_MSC_VER) && (defined(__cplusplus) && __cplusplus < 201103L)
    #error "C++11 or later is required. Add -std=c++11"
#endif

// NOTE: We set some defines with some data types declared by raylib
// Other modules (raymath, rlgl) also require some of those types, so,
// to be able to use those other modules as standalone (not depending on raylib)
// this defines are very useful for internal check and avoid type (re)definitions
#define RL_COLOR_TYPE
#define RL_RECTANGLE_TYPE
#define RL_VECTOR2_TYPE
#define RL_VECTOR3_TYPE
#define RL_VECTOR4_TYPE
#define RL_QUATERNION_TYPE
#define RL_MATRIX_TYPE

// Some Basic Colors
// NOTE: Custom raylib color palette for amazing visuals on RL_WHITE background
#define RL_LIGHTGRAY  RL_CLITERAL(rlColor){ 200, 200, 200, 255 }   // Light Gray
#define RL_GRAY       RL_CLITERAL(rlColor){ 130, 130, 130, 255 }   // Gray
#define RL_DARKGRAY   RL_CLITERAL(rlColor){ 80, 80, 80, 255 }      // Dark Gray
#define RL_YELLOW     RL_CLITERAL(rlColor){ 253, 249, 0, 255 }     // Yellow
#define RL_GOLD       RL_CLITERAL(rlColor){ 255, 203, 0, 255 }     // Gold
#define RL_ORANGE     RL_CLITERAL(rlColor){ 255, 161, 0, 255 }     // Orange
#define RL_PINK       RL_CLITERAL(rlColor){ 255, 109, 194, 255 }   // Pink
#define RL_RED        RL_CLITERAL(rlColor){ 230, 41, 55, 255 }     // Red
#define RL_MAROON     RL_CLITERAL(rlColor){ 190, 33, 55, 255 }     // Maroon
#define RL_GREEN      RL_CLITERAL(rlColor){ 0, 228, 48, 255 }      // Green
#define RL_LIME       RL_CLITERAL(rlColor){ 0, 158, 47, 255 }      // Lime
#define RL_DARKGREEN  RL_CLITERAL(rlColor){ 0, 117, 44, 255 }      // Dark Green
#define RL_SKYBLUE    RL_CLITERAL(rlColor){ 102, 191, 255, 255 }   // Sky Blue
#define RL_BLUE       RL_CLITERAL(rlColor){ 0, 121, 241, 255 }     // Blue
#define RL_DARKBLUE   RL_CLITERAL(rlColor){ 0, 82, 172, 255 }      // Dark Blue
#define RL_PURPLE     RL_CLITERAL(rlColor){ 200, 122, 255, 255 }   // Purple
#define RL_VIOLET     RL_CLITERAL(rlColor){ 135, 60, 190, 255 }    // Violet
#define RL_DARKPURPLE RL_CLITERAL(rlColor){ 112, 31, 126, 255 }    // Dark Purple
#define RL_BEIGE      RL_CLITERAL(rlColor){ 211, 176, 131, 255 }   // Beige
#define RL_BROWN      RL_CLITERAL(rlColor){ 127, 106, 79, 255 }    // Brown
#define RL_DARKBROWN  RL_CLITERAL(rlColor){ 76, 63, 47, 255 }      // Dark Brown

#define RL_WHITE      RL_CLITERAL(rlColor){ 255, 255, 255, 255 }   // White
#define RL_BLACK      RL_CLITERAL(rlColor){ 0, 0, 0, 255 }         // Black
#define RL_BLANK      RL_CLITERAL(rlColor){ 0, 0, 0, 0 }           // Blank (Transparent)
#define RL_MAGENTA    RL_CLITERAL(rlColor){ 255, 0, 255, 255 }     // Magenta
#define RL_RAYWHITE   RL_CLITERAL(rlColor){ 245, 245, 245, 255 }   // My own White (raylib logo)

//----------------------------------------------------------------------------------
// Structures Definition
//----------------------------------------------------------------------------------
// Boolean type
#if (defined(__STDC__) && __STDC_VERSION__ >= 199901L) || (defined(_MSC_VER) && _MSC_VER >= 1800)
    #include <stdbool.h>
#elif !defined(__cplusplus) && !defined(bool)
    typedef enum bool { false = 0, true = !false } bool;
    #define RL_BOOL_TYPE
#endif

#include "config.h"

RL_NS_BEGIN

// rlVector2, 2 components
typedef struct rlVector2 {
    float x;                // Vector x component
    float y;                // Vector y component
} rlVector2;

// rlVector3, 3 components
typedef struct rlVector3 {
    float x;                // Vector x component
    float y;                // Vector y component
    float z;                // Vector z component
} rlVector3;

// rlVector4, 4 components
typedef struct rlVector4 {
    float x;                // Vector x component
    float y;                // Vector y component
    float z;                // Vector z component
    float w;                // Vector w component
} rlVector4;

// rlQuaternion, 4 components (rlVector4 alias)
typedef rlVector4 rlQuaternion;

// rlMatrix, 4x4 components, column major, OpenGL style, right-handed
typedef struct rlMatrix {
    float m0, m4, m8, m12;  // rlMatrix first row (4 components)
    float m1, m5, m9, m13;  // rlMatrix second row (4 components)
    float m2, m6, m10, m14; // rlMatrix third row (4 components)
    float m3, m7, m11, m15; // rlMatrix fourth row (4 components)
} rlMatrix;

// rlColor, 4 components, R8G8B8A8 (32bit)
typedef struct rlColor {
    unsigned char r;        // rlColor red value
    unsigned char g;        // rlColor green value
    unsigned char b;        // rlColor blue value
    unsigned char a;        // rlColor alpha value
} rlColor;

// rlRectangle, 4 components
typedef struct rlRectangle {
    float x;                // rlRectangle top-left corner position x
    float y;                // rlRectangle top-left corner position y
    float width;            // rlRectangle width
    float height;           // rlRectangle height
} rlRectangle;

// rlImage, pixel data stored in CPU memory (RAM)
typedef struct rlImage {
    void *data;             // rlImage raw data
    int width;              // rlImage base width
    int height;             // rlImage base height
    int mipmaps;            // Mipmap levels, 1 by default
    int format;             // Data format (rlPixelFormat type)
} rlImage;

// rlTexture, tex data stored in GPU memory (VRAM)
typedef struct rlTexture {
    unsigned int id;        // OpenGL texture id
    int width;              // rlTexture base width
    int height;             // rlTexture base height
    int mipmaps;            // Mipmap levels, 1 by default
    int format;             // Data format (rlPixelFormat type)
} rlTexture;

// rlTexture2D, same as rlTexture
typedef rlTexture rlTexture2D;

// rlTextureCubemap, same as rlTexture
typedef rlTexture rlTextureCubemap;

// rlRenderTexture, fbo for texture rendering
typedef struct rlRenderTexture {
    unsigned int id;        // OpenGL framebuffer object id
    rlTexture texture;        // rlColor buffer attachment texture
    rlTexture depth;          // Depth buffer attachment texture
} rlRenderTexture;

// rlRenderTexture2D, same as rlRenderTexture
typedef rlRenderTexture rlRenderTexture2D;

// rlNPatchInfo, n-patch layout info
typedef struct rlNPatchInfo {
    rlRectangle source;       // rlTexture source rectangle
    int left;               // Left border offset
    int top;                // Top border offset
    int right;              // Right border offset
    int bottom;             // Bottom border offset
    int layout;             // Layout of the n-patch: 3x3, 1x3 or 3x1
} rlNPatchInfo;

// rlGlyphInfo, font characters glyphs info
typedef struct rlGlyphInfo {
    int value;              // Character value (Unicode)
    int offsetX;            // Character offset X when drawing
    int offsetY;            // Character offset Y when drawing
    int advanceX;           // Character advance position X
    rlImage image;            // Character image data
} rlGlyphInfo;

// rlFont, font texture and rlGlyphInfo array data
typedef struct rlFont {
    int baseSize;           // Base size (default chars height)
    int glyphCount;         // Number of glyph characters
    int glyphPadding;       // Padding around the glyph characters
    rlTexture2D texture;      // rlTexture atlas containing the glyphs
    rlRectangle *recs;        // Rectangles in texture for the glyphs
    rlGlyphInfo *glyphs;      // Glyphs info data
} rlFont;

// rlCamera, defines position/orientation in 3d space
typedef struct rlCamera3D {
    rlVector3 position;       // rlCamera position
    rlVector3 target;         // rlCamera target it looks-at
    rlVector3 up;             // rlCamera up vector (rotation over its axis)
    float fovy;             // rlCamera field-of-view aperture in Y (degrees) in perspective, used as near plane width in orthographic
    int projection;         // rlCamera projection: RL_CAMERA_PERSPECTIVE or RL_CAMERA_ORTHOGRAPHIC
} rlCamera3D;

typedef rlCamera3D rlCamera;    // rlCamera type fallback, defaults to rlCamera3D

// rlCamera2D, defines position/orientation in 2d space
typedef struct rlCamera2D {
    rlVector2 offset;         // rlCamera offset (displacement from target)
    rlVector2 target;         // rlCamera target (rotation and zoom origin)
    float rotation;         // rlCamera rotation in degrees
    float zoom;             // rlCamera zoom (scaling), should be 1.0f by default
} rlCamera2D;

// rlMesh, vertex data and vao/vbo
typedef struct rlMesh {
    int vertexCount;        // Number of vertices stored in arrays
    int triangleCount;      // Number of triangles stored (indexed or not)

    // Vertex attributes data
    float *vertices;        // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    float *texcoords;       // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
    float *texcoords2;      // Vertex texture second coordinates (UV - 2 components per vertex) (shader-location = 5)
    float *normals;         // Vertex normals (XYZ - 3 components per vertex) (shader-location = 2)
    float *tangents;        // Vertex tangents (XYZW - 4 components per vertex) (shader-location = 4)
    unsigned char *colors;      // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
    unsigned short *indices;    // Vertex indices (in case vertex data comes indexed)

    // Animation vertex data
    float *animVertices;    // Animated vertex positions (after bones transformations)
    float *animNormals;     // Animated normals (after bones transformations)
    unsigned char *boneIds; // Vertex bone ids, max 255 bone ids, up to 4 bones influence by vertex (skinning)
    float *boneWeights;     // Vertex bone weight, up to 4 bones influence by vertex (skinning)

    // OpenGL identifiers
    unsigned int vaoId;     // OpenGL Vertex Array Object id
    unsigned int *vboId;    // OpenGL Vertex Buffer Objects id (default vertex data)
} rlMesh;

// rlShader
typedef struct rlShader {
    unsigned int id;        // rlShader program id
    int *locs;              // rlShader locations array (RL_MAX_SHADER_LOCATIONS)
} rlShader;

// rlMaterialMap
typedef struct rlMaterialMap {
    rlTexture2D texture;      // rlMaterial map texture
    rlColor color;            // rlMaterial map color
    float value;            // rlMaterial map value
} rlMaterialMap;

// rlMaterial, includes shader and maps
typedef struct rlMaterial {
    rlShader shader;          // rlMaterial shader
    rlMaterialMap *maps;      // rlMaterial maps array (RL_MAX_MATERIAL_MAPS)
    float params[4];        // rlMaterial generic parameters (if required)
} rlMaterial;

// rlTransform, vertex transformation data
typedef struct rlTransform {
    rlVector3 translation;    // Translation
    rlQuaternion rotation;    // Rotation
    rlVector3 scale;          // Scale
} rlTransform;

// Bone, skeletal animation bone
typedef struct rlBoneInfo {
    char name[32];          // Bone name
    int parent;             // Bone parent
} rlBoneInfo;

// rlModel, meshes, materials and animation data
typedef struct rlModel {
    rlMatrix transform;       // Local transform matrix

    int meshCount;          // Number of meshes
    int materialCount;      // Number of materials
    rlMesh *meshes;           // Meshes array
    rlMaterial *materials;    // Materials array
    int *meshMaterial;      // rlMesh material number

    // Animation data
    int boneCount;          // Number of bones
    rlBoneInfo *bones;        // Bones information (skeleton)
    rlTransform *bindPose;    // Bones base transformation (pose)
} rlModel;

// rlModelAnimation
typedef struct rlModelAnimation {
    int boneCount;          // Number of bones
    int frameCount;         // Number of animation frames
    rlBoneInfo *bones;        // Bones information (skeleton)
    rlTransform **framePoses; // Poses array by frame
    char name[32];          // Animation name
} rlModelAnimation;

// rlRay, ray for raycasting
typedef struct rlRay {
    rlVector3 position;       // rlRay position (origin)
    rlVector3 direction;      // rlRay direction
} rlRay;

// rlRayCollision, ray hit information
typedef struct rlRayCollision {
    bool hit;               // Did the ray hit something?
    float distance;         // Distance to the nearest hit
    rlVector3 point;          // rlPoint of the nearest hit
    rlVector3 normal;         // Surface normal of hit
} rlRayCollision;

// rlBoundingBox
typedef struct rlBoundingBox {
    rlVector3 min;            // Minimum vertex box-corner
    rlVector3 max;            // Maximum vertex box-corner
} rlBoundingBox;

// rlWave, audio wave data
typedef struct rlWave {
    unsigned int frameCount;    // Total number of frames (considering channels)
    unsigned int sampleRate;    // Frequency (samples per second)
    unsigned int sampleSize;    // Bit depth (bits per sample): 8, 16, 32 (24 not supported)
    unsigned int channels;      // Number of channels (1-mono, 2-stereo, ...)
    void *data;                 // Buffer data pointer
} rlWave;

// Opaque structs declaration
// NOTE: Actual structs are defined internally in raudio module
typedef struct rlAudioBuffer rlAudioBuffer;
typedef struct rlAudioProcessor rlAudioProcessor;

// rlAudioStream, custom audio stream
typedef struct rlAudioStream {
    rlAudioBuffer *buffer;       // Pointer to internal data used by the audio system
    rlAudioProcessor *processor; // Pointer to internal data processor, useful for audio effects

    unsigned int sampleRate;    // Frequency (samples per second)
    unsigned int sampleSize;    // Bit depth (bits per sample): 8, 16, 32 (24 not supported)
    unsigned int channels;      // Number of channels (1-mono, 2-stereo, ...)
} rlAudioStream;

// rlSound
typedef struct rlSound {
    rlAudioStream stream;         // Audio stream
    unsigned int frameCount;    // Total number of frames (considering channels)
} rlSound;

// rlMusic, audio stream, anything longer than ~10 seconds should be streamed
typedef struct rlMusic {
    rlAudioStream stream;         // Audio stream
    unsigned int frameCount;    // Total number of frames (considering channels)
    bool looping;               // rlMusic looping enable

    int ctxType;                // Type of music context (audio filetype)
    void *ctxData;              // Audio context data, depends on type
} rlMusic;

// rlVrDeviceInfo, Head-Mounted-Display device parameters
typedef struct rlVrDeviceInfo {
    int hResolution;                // Horizontal resolution in pixels
    int vResolution;                // Vertical resolution in pixels
    float hScreenSize;              // Horizontal size in meters
    float vScreenSize;              // Vertical size in meters
    float vScreenCenter;            // Screen center in meters
    float eyeToScreenDistance;      // Distance between eye and display in meters
    float lensSeparationDistance;   // Lens separation distance in meters
    float interpupillaryDistance;   // IPD (distance between pupils) in meters
    float lensDistortionValues[4];  // Lens distortion constant parameters
    float chromaAbCorrection[4];    // Chromatic aberration correction parameters
} rlVrDeviceInfo;

// rlVrStereoConfig, VR stereo rendering configuration for simulator
typedef struct rlVrStereoConfig {
    rlMatrix projection[2];           // VR projection matrices (per eye)
    rlMatrix viewOffset[2];           // VR view offset matrices (per eye)
    float leftLensCenter[2];        // VR left lens center
    float rightLensCenter[2];       // VR right lens center
    float leftScreenCenter[2];      // VR left screen center
    float rightScreenCenter[2];     // VR right screen center
    float scale[2];                 // VR distortion scale
    float scaleIn[2];               // VR distortion scale in
} rlVrStereoConfig;

// File path list
typedef struct rlFilePathList {
    unsigned int capacity;          // Filepaths max entries
    unsigned int count;             // Filepaths entries count
    char **paths;                   // Filepaths entries
} rlFilePathList;

// Automation event
typedef struct rlAutomationEvent {
    unsigned int frame;             // Event frame
    unsigned int type;              // Event type (AutomationEventType)
    int params[4];                  // Event parameters (if required)
} rlAutomationEvent;

// Automation event list
typedef struct rlAutomationEventList {
    unsigned int capacity;          // Events max entries (RL_MAX_AUTOMATION_EVENTS)
    unsigned int count;             // Events entries count
    rlAutomationEvent *events;        // Events entries
} rlAutomationEventList;

//----------------------------------------------------------------------------------
// Enumerators Definition
//----------------------------------------------------------------------------------
// System/Window config flags
// NOTE: Every bit registers one state (use it with bit masks)
// By default all flags are set to 0
typedef enum {
    RL_FLAG_VSYNC_HINT         = 0x00000040,   // Set to try enabling V-Sync on GPU
    RL_FLAG_FULLSCREEN_MODE    = 0x00000002,   // Set to run program in fullscreen
    RL_FLAG_WINDOW_RESIZABLE   = 0x00000004,   // Set to allow resizable window
    RL_FLAG_WINDOW_UNDECORATED = 0x00000008,   // Set to disable window decoration (frame and buttons)
    RL_FLAG_WINDOW_HIDDEN      = 0x00000080,   // Set to hide window
    RL_FLAG_WINDOW_MINIMIZED   = 0x00000200,   // Set to minimize window (iconify)
    RL_FLAG_WINDOW_MAXIMIZED   = 0x00000400,   // Set to maximize window (expanded to monitor)
    RL_FLAG_WINDOW_UNFOCUSED   = 0x00000800,   // Set to window non focused
    RL_FLAG_WINDOW_TOPMOST     = 0x00001000,   // Set to window always on top
    RL_FLAG_WINDOW_ALWAYS_RUN  = 0x00000100,   // Set to allow windows running while minimized
    RL_FLAG_WINDOW_TRANSPARENT = 0x00000010,   // Set to allow transparent framebuffer
    RL_FLAG_WINDOW_HIGHDPI     = 0x00002000,   // Set to support HighDPI
    RL_FLAG_WINDOW_MOUSE_PASSTHROUGH = 0x00004000, // Set to support mouse passthrough, only supported when RL_FLAG_WINDOW_UNDECORATED
    RL_FLAG_BORDERLESS_WINDOWED_MODE = 0x00008000, // Set to run program in borderless windowed mode
    RL_FLAG_MSAA_4X_HINT       = 0x00000020,   // Set to try enabling MSAA 4X
    RL_FLAG_INTERLACED_HINT    = 0x00010000    // Set to try enabling interlaced video format (for V3D)
} rlConfigFlags;

// Trace log level
// NOTE: Organized by priority level
typedef enum {
    RL_LOG_ALL = 0,        // Display all logs
    RL_LOG_TRACE,          // Trace logging, intended for internal use only
    RL_LOG_DEBUG,          // Debug logging, used for internal debugging, it should be disabled on release builds
    RL_LOG_INFO,           // Info logging, used for program execution info
    RL_LOG_WARNING,        // Warning logging, used on recoverable failures
    RL_LOG_ERROR,          // Error logging, used on unrecoverable failures
    RL_LOG_FATAL,          // Fatal logging, used to abort program: exit(EXIT_FAILURE)
    RL_LOG_NONE            // Disable logging
} rlTraceLogLevel;

// Keyboard keys (US keyboard layout)
// NOTE: Use rlGetKeyPressed() to allow redefining
// required keys for alternative layouts
typedef enum {
    RL_KEY_NULL            = 0,        // Key: NULL, used for no key pressed
    // Alphanumeric keys
    RL_KEY_APOSTROPHE      = 39,       // Key: '
    RL_KEY_COMMA           = 44,       // Key: ,
    RL_KEY_MINUS           = 45,       // Key: -
    RL_KEY_PERIOD          = 46,       // Key: .
    RL_KEY_SLASH           = 47,       // Key: /
    RL_KEY_ZERO            = 48,       // Key: 0
    RL_KEY_ONE             = 49,       // Key: 1
    RL_KEY_TWO             = 50,       // Key: 2
    RL_KEY_THREE           = 51,       // Key: 3
    RL_KEY_FOUR            = 52,       // Key: 4
    RL_KEY_FIVE            = 53,       // Key: 5
    RL_KEY_SIX             = 54,       // Key: 6
    RL_KEY_SEVEN           = 55,       // Key: 7
    RL_KEY_EIGHT           = 56,       // Key: 8
    RL_KEY_NINE            = 57,       // Key: 9
    RL_KEY_SEMICOLON       = 59,       // Key: ;
    RL_KEY_EQUAL           = 61,       // Key: =
    RL_KEY_A               = 65,       // Key: A | a
    RL_KEY_B               = 66,       // Key: B | b
    RL_KEY_C               = 67,       // Key: C | c
    RL_KEY_D               = 68,       // Key: D | d
    RL_KEY_E               = 69,       // Key: E | e
    RL_KEY_F               = 70,       // Key: F | f
    RL_KEY_G               = 71,       // Key: G | g
    RL_KEY_H               = 72,       // Key: H | h
    RL_KEY_I               = 73,       // Key: I | i
    RL_KEY_J               = 74,       // Key: J | j
    RL_KEY_K               = 75,       // Key: K | k
    RL_KEY_L               = 76,       // Key: L | l
    RL_KEY_M               = 77,       // Key: M | m
    RL_KEY_N               = 78,       // Key: N | n
    RL_KEY_O               = 79,       // Key: O | o
    RL_KEY_P               = 80,       // Key: P | p
    RL_KEY_Q               = 81,       // Key: Q | q
    RL_KEY_R               = 82,       // Key: R | r
    RL_KEY_S               = 83,       // Key: S | s
    RL_KEY_T               = 84,       // Key: T | t
    RL_KEY_U               = 85,       // Key: U | u
    RL_KEY_V               = 86,       // Key: V | v
    RL_KEY_W               = 87,       // Key: W | w
    RL_KEY_X               = 88,       // Key: X | x
    RL_KEY_Y               = 89,       // Key: Y | y
    RL_KEY_Z               = 90,       // Key: Z | z
    RL_KEY_LEFT_BRACKET    = 91,       // Key: [
    RL_KEY_BACKSLASH       = 92,       // Key: '\'
    RL_KEY_RIGHT_BRACKET   = 93,       // Key: ]
    RL_KEY_GRAVE           = 96,       // Key: `
    // Function keys
    RL_KEY_SPACE           = 32,       // Key: Space
    RL_KEY_ESCAPE          = 256,      // Key: Esc
    RL_KEY_ENTER           = 257,      // Key: Enter
    RL_KEY_TAB             = 258,      // Key: Tab
    RL_KEY_BACKSPACE       = 259,      // Key: Backspace
    RL_KEY_INSERT          = 260,      // Key: Ins
    RL_KEY_DELETE          = 261,      // Key: Del
    RL_KEY_RIGHT           = 262,      // Key: Cursor right
    RL_KEY_LEFT            = 263,      // Key: Cursor left
    RL_KEY_DOWN            = 264,      // Key: Cursor down
    RL_KEY_UP              = 265,      // Key: Cursor up
    RL_KEY_PAGE_UP         = 266,      // Key: Page up
    RL_KEY_PAGE_DOWN       = 267,      // Key: Page down
    RL_KEY_HOME            = 268,      // Key: Home
    RL_KEY_END             = 269,      // Key: End
    RL_KEY_CAPS_LOCK       = 280,      // Key: Caps lock
    RL_KEY_SCROLL_LOCK     = 281,      // Key: Scroll down
    RL_KEY_NUM_LOCK        = 282,      // Key: Num lock
    RL_KEY_PRINT_SCREEN    = 283,      // Key: Print screen
    RL_KEY_PAUSE           = 284,      // Key: Pause
    RL_KEY_F1              = 290,      // Key: F1
    RL_KEY_F2              = 291,      // Key: F2
    RL_KEY_F3              = 292,      // Key: F3
    RL_KEY_F4              = 293,      // Key: F4
    RL_KEY_F5              = 294,      // Key: F5
    RL_KEY_F6              = 295,      // Key: F6
    RL_KEY_F7              = 296,      // Key: F7
    RL_KEY_F8              = 297,      // Key: F8
    RL_KEY_F9              = 298,      // Key: F9
    RL_KEY_F10             = 299,      // Key: F10
    RL_KEY_F11             = 300,      // Key: F11
    RL_KEY_F12             = 301,      // Key: F12
    RL_KEY_LEFT_SHIFT      = 340,      // Key: Shift left
    RL_KEY_LEFT_CONTROL    = 341,      // Key: Control left
    RL_KEY_LEFT_ALT        = 342,      // Key: Alt left
    RL_KEY_LEFT_SUPER      = 343,      // Key: Super left
    RL_KEY_RIGHT_SHIFT     = 344,      // Key: Shift right
    RL_KEY_RIGHT_CONTROL   = 345,      // Key: Control right
    RL_KEY_RIGHT_ALT       = 346,      // Key: Alt right
    RL_KEY_RIGHT_SUPER     = 347,      // Key: Super right
    RL_KEY_KB_MENU         = 348,      // Key: KB menu
    // Keypad keys
    RL_KEY_KP_0            = 320,      // Key: Keypad 0
    RL_KEY_KP_1            = 321,      // Key: Keypad 1
    RL_KEY_KP_2            = 322,      // Key: Keypad 2
    RL_KEY_KP_3            = 323,      // Key: Keypad 3
    RL_KEY_KP_4            = 324,      // Key: Keypad 4
    RL_KEY_KP_5            = 325,      // Key: Keypad 5
    RL_KEY_KP_6            = 326,      // Key: Keypad 6
    RL_KEY_KP_7            = 327,      // Key: Keypad 7
    RL_KEY_KP_8            = 328,      // Key: Keypad 8
    RL_KEY_KP_9            = 329,      // Key: Keypad 9
    RL_KEY_KP_DECIMAL      = 330,      // Key: Keypad .
    RL_KEY_KP_DIVIDE       = 331,      // Key: Keypad /
    RL_KEY_KP_MULTIPLY     = 332,      // Key: Keypad *
    RL_KEY_KP_SUBTRACT     = 333,      // Key: Keypad -
    RL_KEY_KP_ADD          = 334,      // Key: Keypad +
    RL_KEY_KP_ENTER        = 335,      // Key: Keypad Enter
    RL_KEY_KP_EQUAL        = 336,      // Key: Keypad =
    // Android key buttons
    RL_KEY_BACK            = 4,        // Key: Android back button
    RL_KEY_MENU            = 82,       // Key: Android menu button
    RL_KEY_VOLUME_UP       = 24,       // Key: Android volume up button
    RL_KEY_VOLUME_DOWN     = 25        // Key: Android volume down button
} rlKeyboardKey;

// Add backwards compatibility support for deprecated names
#define RL_MOUSE_LEFT_BUTTON   RL_MOUSE_BUTTON_LEFT
#define RL_MOUSE_RIGHT_BUTTON  RL_MOUSE_BUTTON_RIGHT
#define RL_MOUSE_MIDDLE_BUTTON RL_MOUSE_BUTTON_MIDDLE

// Mouse buttons
typedef enum {
    RL_MOUSE_BUTTON_LEFT    = 0,       // Mouse button left
    RL_MOUSE_BUTTON_RIGHT   = 1,       // Mouse button right
    RL_MOUSE_BUTTON_MIDDLE  = 2,       // Mouse button middle (pressed wheel)
    RL_MOUSE_BUTTON_SIDE    = 3,       // Mouse button side (advanced mouse device)
    RL_MOUSE_BUTTON_EXTRA   = 4,       // Mouse button extra (advanced mouse device)
    RL_MOUSE_BUTTON_FORWARD = 5,       // Mouse button forward (advanced mouse device)
    RL_MOUSE_BUTTON_BACK    = 6,       // Mouse button back (advanced mouse device)
} rlMouseButton;

// Mouse cursor
typedef enum {
    RL_MOUSE_CURSOR_DEFAULT       = 0,     // Default pointer shape
    RL_MOUSE_CURSOR_ARROW         = 1,     // Arrow shape
    RL_MOUSE_CURSOR_IBEAM         = 2,     // Text writing cursor shape
    RL_MOUSE_CURSOR_CROSSHAIR     = 3,     // Cross shape
    RL_MOUSE_CURSOR_POINTING_HAND = 4,     // Pointing hand cursor
    RL_MOUSE_CURSOR_RESIZE_EW     = 5,     // Horizontal resize/move arrow shape
    RL_MOUSE_CURSOR_RESIZE_NS     = 6,     // Vertical resize/move arrow shape
    RL_MOUSE_CURSOR_RESIZE_NWSE   = 7,     // Top-left to bottom-right diagonal resize/move arrow shape
    RL_MOUSE_CURSOR_RESIZE_NESW   = 8,     // The top-right to bottom-left diagonal resize/move arrow shape
    RL_MOUSE_CURSOR_RESIZE_ALL    = 9,     // The omnidirectional resize/move cursor shape
    RL_MOUSE_CURSOR_NOT_ALLOWED   = 10     // The operation-not-allowed shape
} rlMouseCursor;

// Gamepad buttons
typedef enum {
    RL_GAMEPAD_BUTTON_UNKNOWN = 0,         // Unknown button, just for error checking
    RL_GAMEPAD_BUTTON_LEFT_FACE_UP,        // Gamepad left DPAD up button
    RL_GAMEPAD_BUTTON_LEFT_FACE_RIGHT,     // Gamepad left DPAD right button
    RL_GAMEPAD_BUTTON_LEFT_FACE_DOWN,      // Gamepad left DPAD down button
    RL_GAMEPAD_BUTTON_LEFT_FACE_LEFT,      // Gamepad left DPAD left button
    RL_GAMEPAD_BUTTON_RIGHT_FACE_UP,       // Gamepad right button up (i.e. PS3: Triangle, Xbox: Y)
    RL_GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,    // Gamepad right button right (i.e. PS3: Square, Xbox: X)
    RL_GAMEPAD_BUTTON_RIGHT_FACE_DOWN,     // Gamepad right button down (i.e. PS3: Cross, Xbox: A)
    RL_GAMEPAD_BUTTON_RIGHT_FACE_LEFT,     // Gamepad right button left (i.e. PS3: Circle, Xbox: B)
    RL_GAMEPAD_BUTTON_LEFT_TRIGGER_1,      // Gamepad top/back trigger left (first), it could be a trailing button
    RL_GAMEPAD_BUTTON_LEFT_TRIGGER_2,      // Gamepad top/back trigger left (second), it could be a trailing button
    RL_GAMEPAD_BUTTON_RIGHT_TRIGGER_1,     // Gamepad top/back trigger right (one), it could be a trailing button
    RL_GAMEPAD_BUTTON_RIGHT_TRIGGER_2,     // Gamepad top/back trigger right (second), it could be a trailing button
    RL_GAMEPAD_BUTTON_MIDDLE_LEFT,         // Gamepad center buttons, left one (i.e. PS3: Select)
    RL_GAMEPAD_BUTTON_MIDDLE,              // Gamepad center buttons, middle one (i.e. PS3: PS, Xbox: XBOX)
    RL_GAMEPAD_BUTTON_MIDDLE_RIGHT,        // Gamepad center buttons, right one (i.e. PS3: Start)
    RL_GAMEPAD_BUTTON_LEFT_THUMB,          // Gamepad joystick pressed button left
    RL_GAMEPAD_BUTTON_RIGHT_THUMB          // Gamepad joystick pressed button right
} rlGamepadButton;

// Gamepad axis
typedef enum {
    RL_GAMEPAD_AXIS_LEFT_X        = 0,     // Gamepad left stick X axis
    RL_GAMEPAD_AXIS_LEFT_Y        = 1,     // Gamepad left stick Y axis
    RL_GAMEPAD_AXIS_RIGHT_X       = 2,     // Gamepad right stick X axis
    RL_GAMEPAD_AXIS_RIGHT_Y       = 3,     // Gamepad right stick Y axis
    RL_GAMEPAD_AXIS_LEFT_TRIGGER  = 4,     // Gamepad back trigger left, pressure level: [1..-1]
    RL_GAMEPAD_AXIS_RIGHT_TRIGGER = 5      // Gamepad back trigger right, pressure level: [1..-1]
} rlGamepadAxis;

// rlMaterial map index
typedef enum {
    RL_MATERIAL_MAP_ALBEDO = 0,        // Albedo material (same as: RL_MATERIAL_MAP_DIFFUSE)
    RL_MATERIAL_MAP_METALNESS,         // Metalness material (same as: RL_MATERIAL_MAP_SPECULAR)
    RL_MATERIAL_MAP_NORMAL,            // Normal material
    RL_MATERIAL_MAP_ROUGHNESS,         // Roughness material
    RL_MATERIAL_MAP_OCCLUSION,         // Ambient occlusion material
    RL_MATERIAL_MAP_EMISSION,          // Emission material
    RL_MATERIAL_MAP_HEIGHT,            // Heightmap material
    RL_MATERIAL_MAP_CUBEMAP,           // Cubemap material (NOTE: Uses GL_TEXTURE_CUBE_MAP)
    RL_MATERIAL_MAP_IRRADIANCE,        // Irradiance material (NOTE: Uses GL_TEXTURE_CUBE_MAP)
    RL_MATERIAL_MAP_PREFILTER,         // Prefilter material (NOTE: Uses GL_TEXTURE_CUBE_MAP)
    RL_MATERIAL_MAP_BRDF               // Brdf material
} rlMaterialMapIndex;

#define RL_MATERIAL_MAP_DIFFUSE      RL_MATERIAL_MAP_ALBEDO
#define RL_MATERIAL_MAP_SPECULAR     RL_MATERIAL_MAP_METALNESS

// rlShader location index
typedef enum {
    RL_SHADER_LOC_VERTEX_POSITION = 0, // rlShader location: vertex attribute: position
    RL_SHADER_LOC_VERTEX_TEXCOORD01,   // rlShader location: vertex attribute: texcoord01
    RL_SHADER_LOC_VERTEX_TEXCOORD02,   // rlShader location: vertex attribute: texcoord02
    RL_SHADER_LOC_VERTEX_NORMAL,       // rlShader location: vertex attribute: normal
    RL_SHADER_LOC_VERTEX_TANGENT,      // rlShader location: vertex attribute: tangent
    RL_SHADER_LOC_VERTEX_COLOR,        // rlShader location: vertex attribute: color
    RL_SHADER_LOC_MATRIX_MVP,          // rlShader location: matrix uniform: model-view-projection
    RL_SHADER_LOC_MATRIX_VIEW,         // rlShader location: matrix uniform: view (camera transform)
    RL_SHADER_LOC_MATRIX_PROJECTION,   // rlShader location: matrix uniform: projection
    RL_SHADER_LOC_MATRIX_MODEL,        // rlShader location: matrix uniform: model (transform)
    RL_SHADER_LOC_MATRIX_NORMAL,       // rlShader location: matrix uniform: normal
    RL_SHADER_LOC_VECTOR_VIEW,         // rlShader location: vector uniform: view
    RL_SHADER_LOC_COLOR_DIFFUSE,       // rlShader location: vector uniform: diffuse color
    RL_SHADER_LOC_COLOR_SPECULAR,      // rlShader location: vector uniform: specular color
    RL_SHADER_LOC_COLOR_AMBIENT,       // rlShader location: vector uniform: ambient color
    RL_SHADER_LOC_MAP_ALBEDO,          // rlShader location: sampler2d texture: albedo (same as: RL_SHADER_LOC_MAP_DIFFUSE)
    RL_SHADER_LOC_MAP_METALNESS,       // rlShader location: sampler2d texture: metalness (same as: RL_SHADER_LOC_MAP_SPECULAR)
    RL_SHADER_LOC_MAP_NORMAL,          // rlShader location: sampler2d texture: normal
    RL_SHADER_LOC_MAP_ROUGHNESS,       // rlShader location: sampler2d texture: roughness
    RL_SHADER_LOC_MAP_OCCLUSION,       // rlShader location: sampler2d texture: occlusion
    RL_SHADER_LOC_MAP_EMISSION,        // rlShader location: sampler2d texture: emission
    RL_SHADER_LOC_MAP_HEIGHT,          // rlShader location: sampler2d texture: height
    RL_SHADER_LOC_MAP_CUBEMAP,         // rlShader location: samplerCube texture: cubemap
    RL_SHADER_LOC_MAP_IRRADIANCE,      // rlShader location: samplerCube texture: irradiance
    RL_SHADER_LOC_MAP_PREFILTER,       // rlShader location: samplerCube texture: prefilter
    RL_SHADER_LOC_MAP_BRDF             // rlShader location: sampler2d texture: brdf
} rlShaderLocationIndex;

#define RL_SHADER_LOC_MAP_DIFFUSE      RL_SHADER_LOC_MAP_ALBEDO
#define RL_SHADER_LOC_MAP_SPECULAR     RL_SHADER_LOC_MAP_METALNESS

// rlShader uniform data type
typedef enum {
    RL_SHADER_UNIFORM_FLOAT = 0,       // rlShader uniform type: float
    RL_SHADER_UNIFORM_VEC2,            // rlShader uniform type: vec2 (2 float)
    RL_SHADER_UNIFORM_VEC3,            // rlShader uniform type: vec3 (3 float)
    RL_SHADER_UNIFORM_VEC4,            // rlShader uniform type: vec4 (4 float)
    RL_SHADER_UNIFORM_INT,             // rlShader uniform type: int
    RL_SHADER_UNIFORM_IVEC2,           // rlShader uniform type: ivec2 (2 int)
    RL_SHADER_UNIFORM_IVEC3,           // rlShader uniform type: ivec3 (3 int)
    RL_SHADER_UNIFORM_IVEC4,           // rlShader uniform type: ivec4 (4 int)
    RL_SHADER_UNIFORM_SAMPLER2D        // rlShader uniform type: sampler2d
} rlShaderUniformDataType;

// rlShader attribute data types
typedef enum {
    RL_SHADER_ATTRIB_FLOAT = 0,        // rlShader attribute type: float
    RL_SHADER_ATTRIB_VEC2,             // rlShader attribute type: vec2 (2 float)
    RL_SHADER_ATTRIB_VEC3,             // rlShader attribute type: vec3 (3 float)
    RL_SHADER_ATTRIB_VEC4              // rlShader attribute type: vec4 (4 float)
} rlShaderAttributeDataType;

// Pixel formats
// NOTE: Support depends on OpenGL version and platform
typedef enum {
    RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1, // 8 bit per pixel (no alpha)
    RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,    // 8*2 bpp (2 channels)
    RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5,        // 16 bpp
    RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8,        // 24 bpp
    RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,      // 16 bpp (1 bit alpha)
    RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,      // 16 bpp (4 bit alpha)
    RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,      // 32 bpp
    RL_PIXELFORMAT_UNCOMPRESSED_R32,           // 32 bpp (1 channel - float)
    RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32,     // 32*3 bpp (3 channels - float)
    RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,  // 32*4 bpp (4 channels - float)
    RL_PIXELFORMAT_UNCOMPRESSED_R16,           // 16 bpp (1 channel - half float)
    RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16,     // 16*3 bpp (3 channels - half float)
    RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,  // 16*4 bpp (4 channels - half float)
    RL_PIXELFORMAT_COMPRESSED_DXT1_RGB,        // 4 bpp (no alpha)
    RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA,       // 4 bpp (1 bit alpha)
    RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA,       // 8 bpp
    RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA,       // 8 bpp
    RL_PIXELFORMAT_COMPRESSED_ETC1_RGB,        // 4 bpp
    RL_PIXELFORMAT_COMPRESSED_ETC2_RGB,        // 4 bpp
    RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,   // 8 bpp
    RL_PIXELFORMAT_COMPRESSED_PVRT_RGB,        // 4 bpp
    RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA,       // 4 bpp
    RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,   // 8 bpp
    RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA    // 2 bpp
} rlPixelFormat;

// rlTexture parameters: filter mode
// NOTE 1: Filtering considers mipmaps if available in the texture
// NOTE 2: Filter is accordingly set for minification and magnification
typedef enum {
    RL_TEXTURE_FILTER_POINT = 0,               // No filter, just pixel approximation
    RL_TEXTURE_FILTER_BILINEAR,                // Linear filtering
    RL_TEXTURE_FILTER_TRILINEAR,               // Trilinear filtering (linear with mipmaps)
    RL_TEXTURE_FILTER_ANISOTROPIC_4X,          // Anisotropic filtering 4x
    RL_TEXTURE_FILTER_ANISOTROPIC_8X,          // Anisotropic filtering 8x
    RL_TEXTURE_FILTER_ANISOTROPIC_16X,         // Anisotropic filtering 16x
} rlTextureFilter;

// rlTexture parameters: wrap mode
typedef enum {
    RL_TEXTURE_WRAP_REPEAT = 0,                // Repeats texture in tiled mode
    RL_TEXTURE_WRAP_CLAMP,                     // Clamps texture to edge pixel in tiled mode
    RL_TEXTURE_WRAP_MIRROR_REPEAT,             // Mirrors and repeats the texture in tiled mode
    RL_TEXTURE_WRAP_MIRROR_CLAMP               // Mirrors and clamps to border the texture in tiled mode
} rlTextureWrap;

// Cubemap layouts
typedef enum {
    RL_CUBEMAP_LAYOUT_AUTO_DETECT = 0,         // Automatically detect layout type
    RL_CUBEMAP_LAYOUT_LINE_VERTICAL,           // Layout is defined by a vertical line with faces
    RL_CUBEMAP_LAYOUT_LINE_HORIZONTAL,         // Layout is defined by a horizontal line with faces
    RL_CUBEMAP_LAYOUT_CROSS_THREE_BY_FOUR,     // Layout is defined by a 3x4 cross with cubemap faces
    RL_CUBEMAP_LAYOUT_CROSS_FOUR_BY_THREE,     // Layout is defined by a 4x3 cross with cubemap faces
    RL_CUBEMAP_LAYOUT_PANORAMA                 // Layout is defined by a panorama image (equirrectangular map)
} rlCubemapLayout;

// rlFont type, defines generation method
typedef enum {
    RL_FONT_DEFAULT = 0,               // Default font generation, anti-aliased
    RL_FONT_BITMAP,                    // Bitmap font generation, no anti-aliasing
    RL_FONT_SDF                        // SDF font generation, requires external shader
} rlFontType;

// rlColor blending modes (pre-defined)
typedef enum {
    RL_BLEND_ALPHA = 0,                // Blend textures considering alpha (default)
    RL_BLEND_ADDITIVE,                 // Blend textures adding colors
    RL_BLEND_MULTIPLIED,               // Blend textures multiplying colors
    RL_BLEND_ADD_COLORS,               // Blend textures adding colors (alternative)
    RL_BLEND_SUBTRACT_COLORS,          // Blend textures subtracting colors (alternative)
    RL_BLEND_ALPHA_PREMULTIPLY,        // Blend premultiplied textures considering alpha
    RL_BLEND_CUSTOM,                   // Blend textures using custom src/dst factors (use rlglSetBlendFactors())
    RL_BLEND_CUSTOM_SEPARATE           // Blend textures using custom rgb/alpha separate src/dst factors (use rlglSetBlendFactorsSeparate())
} rlBlendMode;

// rlGesture
// NOTE: Provided as bit-wise flags to enable only desired gestures
typedef enum {
    RL_GESTURE_NONE        = 0,        // No gesture
    RL_GESTURE_TAP         = 1,        // Tap gesture
    RL_GESTURE_DOUBLETAP   = 2,        // Double tap gesture
    RL_GESTURE_HOLD        = 4,        // Hold gesture
    RL_GESTURE_DRAG        = 8,        // Drag gesture
    RL_GESTURE_SWIPE_RIGHT = 16,       // Swipe right gesture
    RL_GESTURE_SWIPE_LEFT  = 32,       // Swipe left gesture
    RL_GESTURE_SWIPE_UP    = 64,       // Swipe up gesture
    RL_GESTURE_SWIPE_DOWN  = 128,      // Swipe down gesture
    RL_GESTURE_PINCH_IN    = 256,      // Pinch in gesture
    RL_GESTURE_PINCH_OUT   = 512       // Pinch out gesture
} rlGesture;

// rlCamera system modes
typedef enum {
    RL_CAMERA_CUSTOM = 0,              // Custom camera
    RL_CAMERA_FREE,                    // Free camera
    RL_CAMERA_ORBITAL,                 // Orbital camera
    RL_CAMERA_FIRST_PERSON,            // First person camera
    RL_CAMERA_THIRD_PERSON             // Third person camera
} rlCameraMode;

// rlCamera projection
typedef enum {
    RL_CAMERA_PERSPECTIVE = 0,         // Perspective projection
    RL_CAMERA_ORTHOGRAPHIC             // Orthographic projection
} rlCameraProjection;

// N-patch layout
typedef enum {
    RL_NPATCH_NINE_PATCH = 0,          // Npatch layout: 3x3 tiles
    RL_NPATCH_THREE_PATCH_VERTICAL,    // Npatch layout: 1x3 tiles
    RL_NPATCH_THREE_PATCH_HORIZONTAL   // Npatch layout: 3x1 tiles
} rlNPatchLayout;

// Callbacks to hook some internal functions
// WARNING: These callbacks are intended for advance users
typedef void (*rlTraceLogCallback)(int logLevel, const char *text, va_list args);  // Logging: Redirect trace log messages
typedef unsigned char *(*rlLoadFileDataCallback)(const char *fileName, int *dataSize);    // FileIO: Load binary data
typedef bool (*rlSaveFileDataCallback)(const char *fileName, void *data, int dataSize);   // FileIO: Save binary data
typedef char *(*rlLoadFileTextCallback)(const char *fileName);            // FileIO: Load text data
typedef bool (*rlSaveFileTextCallback)(const char *fileName, char *text); // FileIO: Save text data

//------------------------------------------------------------------------------------
// Global Variables Definition
//------------------------------------------------------------------------------------
// It's lonely here...

//------------------------------------------------------------------------------------
// Window and Graphics Device Functions (Module: core)
//------------------------------------------------------------------------------------

RL_EXTERN_C_BEGIN

// Window-related functions
RLAPI void rlInitWindow(int width, int height, const char *title);  // Initialize window and OpenGL context
RLAPI void rlCloseWindow(void);                                     // Close window and unload OpenGL context
RLAPI bool rlWindowShouldClose(void);                               // Check if application should close (RL_KEY_ESCAPE pressed or windows close icon clicked)
RLAPI bool rlIsWindowReady(void);                                   // Check if window has been initialized successfully
RLAPI bool rlIsWindowFullscreen(void);                              // Check if window is currently fullscreen
RLAPI bool rlIsWindowHidden(void);                                  // Check if window is currently hidden (only PLATFORM_DESKTOP)
RLAPI bool rlIsWindowMinimized(void);                               // Check if window is currently minimized (only PLATFORM_DESKTOP)
RLAPI bool rlIsWindowMaximized(void);                               // Check if window is currently maximized (only PLATFORM_DESKTOP)
RLAPI bool rlIsWindowFocused(void);                                 // Check if window is currently focused (only PLATFORM_DESKTOP)
RLAPI bool rlIsWindowResized(void);                                 // Check if window has been resized last frame
RLAPI bool rlIsWindowState(unsigned int flag);                      // Check if one specific window flag is enabled
RLAPI void rlSetWindowState(unsigned int flags);                    // Set window configuration state using flags (only PLATFORM_DESKTOP)
RLAPI void rlClearWindowState(unsigned int flags);                  // Clear window configuration state flags
RLAPI void rlToggleFullscreen(void);                                // Toggle window state: fullscreen/windowed (only PLATFORM_DESKTOP)
RLAPI void rlToggleBorderlessWindowed(void);                        // Toggle window state: borderless windowed (only PLATFORM_DESKTOP)
RLAPI void rlMaximizeWindow(void);                                  // Set window state: maximized, if resizable (only PLATFORM_DESKTOP)
RLAPI void rlMinimizeWindow(void);                                  // Set window state: minimized, if resizable (only PLATFORM_DESKTOP)
RLAPI void rlRestoreWindow(void);                                   // Set window state: not minimized/maximized (only PLATFORM_DESKTOP)
RLAPI void rlSetWindowIcon(rlImage image);                            // Set icon for window (single image, RGBA 32bit, only PLATFORM_DESKTOP)
RLAPI void rlSetWindowIcons(rlImage *images, int count);              // Set icon for window (multiple images, RGBA 32bit, only PLATFORM_DESKTOP)
RLAPI void rlSetWindowTitle(const char *title);                     // Set title for window (only PLATFORM_DESKTOP and PLATFORM_WEB)
RLAPI void rlSetWindowPosition(int x, int y);                       // Set window position on screen (only PLATFORM_DESKTOP)
RLAPI void rlSetWindowMonitor(int monitor);                         // Set monitor for the current window
RLAPI void rlSetWindowMinSize(int width, int height);               // Set window minimum dimensions (for RL_FLAG_WINDOW_RESIZABLE)
RLAPI void rlSetWindowMaxSize(int width, int height);               // Set window maximum dimensions (for RL_FLAG_WINDOW_RESIZABLE)
RLAPI void rlSetWindowSize(int width, int height);                  // Set window dimensions
RLAPI void rlSetWindowOpacity(float opacity);                       // Set window opacity [0.0f..1.0f] (only PLATFORM_DESKTOP)
RLAPI void rlSetWindowFocused(void);                                // Set window focused (only PLATFORM_DESKTOP)
RLAPI void *rlGetWindowHandle(void);                                // Get native window handle
RLAPI int rlGetScreenWidth(void);                                   // Get current screen width
RLAPI int rlGetScreenHeight(void);                                  // Get current screen height
RLAPI int rlGetRenderWidth(void);                                   // Get current render width (it considers HiDPI)
RLAPI int rlGetRenderHeight(void);                                  // Get current render height (it considers HiDPI)
RLAPI int rlGetMonitorCount(void);                                  // Get number of connected monitors
RLAPI int rlGetCurrentMonitor(void);                                // Get current connected monitor
RLAPI rlVector2 rlGetMonitorPosition(int monitor);                    // Get specified monitor position
RLAPI int rlGetMonitorWidth(int monitor);                           // Get specified monitor width (current video mode used by monitor)
RLAPI int rlGetMonitorHeight(int monitor);                          // Get specified monitor height (current video mode used by monitor)
RLAPI int rlGetMonitorPhysicalWidth(int monitor);                   // Get specified monitor physical width in millimetres
RLAPI int rlGetMonitorPhysicalHeight(int monitor);                  // Get specified monitor physical height in millimetres
RLAPI int rlGetMonitorRefreshRate(int monitor);                     // Get specified monitor refresh rate
RLAPI rlVector2 rlGetWindowPosition(void);                            // Get window position XY on monitor
RLAPI rlVector2 rlGetWindowScaleDPI(void);                            // Get window scale DPI factor
RLAPI const char *rlGetMonitorName(int monitor);                    // Get the human-readable, UTF-8 encoded name of the specified monitor
RLAPI void rlSetClipboardText(const char *text);                    // Set clipboard text content
RLAPI const char *rlGetClipboardText(void);                         // Get clipboard text content
RLAPI void rlEnableEventWaiting(void);                              // Enable waiting for events on rlEndDrawing(), no automatic event polling
RLAPI void rlDisableEventWaiting(void);                             // Disable waiting for events on rlEndDrawing(), automatic events polling

// Cursor-related functions
RLAPI void rlShowCursor(void);                                      // Shows cursor
RLAPI void rlHideCursor(void);                                      // Hides cursor
RLAPI bool rlIsCursorHidden(void);                                  // Check if cursor is not visible
RLAPI void rlEnableCursor(void);                                    // Enables cursor (unlock cursor)
RLAPI void rlDisableCursor(void);                                   // Disables cursor (lock cursor)
RLAPI bool rlIsCursorOnScreen(void);                                // Check if cursor is on the screen

// Drawing-related functions
RLAPI void rlClearBackground(rlColor color);                          // Set background color (framebuffer clear color)
RLAPI void rlBeginDrawing(void);                                    // Setup canvas (framebuffer) to start drawing
RLAPI void rlEndDrawing(void);                                      // End canvas drawing and swap buffers (double buffering)
RLAPI void rlBeginMode2D(rlCamera2D camera);                          // Begin 2D mode with custom camera (2D)
RLAPI void rlEndMode2D(void);                                       // Ends 2D mode with custom camera
RLAPI void rlBeginMode3D(rlCamera3D camera);                          // Begin 3D mode with custom camera (3D)
RLAPI void rlEndMode3D(void);                                       // Ends 3D mode and returns to default 2D orthographic mode
RLAPI void rlBeginTextureMode(rlRenderTexture2D target);              // Begin drawing to render texture
RLAPI void rlEndTextureMode(void);                                  // Ends drawing to render texture
RLAPI void rlBeginShaderMode(rlShader shader);                        // Begin custom shader drawing
RLAPI void rlEndShaderMode(void);                                   // End custom shader drawing (use default shader)
RLAPI void rlBeginBlendMode(int mode);                              // Begin blending mode (alpha, additive, multiplied, subtract, custom)
RLAPI void rlEndBlendMode(void);                                    // End blending mode (reset to default: alpha blending)
RLAPI void rlBeginScissorMode(int x, int y, int width, int height); // Begin scissor mode (define screen area for following drawing)
RLAPI void rlEndScissorMode(void);                                  // End scissor mode
RLAPI void rlBeginVrStereoMode(rlVrStereoConfig config);              // Begin stereo rendering (requires VR simulator)
RLAPI void rlEndVrStereoMode(void);                                 // End stereo rendering (requires VR simulator)

// VR stereo config functions for VR simulator
RLAPI rlVrStereoConfig rlLoadVrStereoConfig(rlVrDeviceInfo device);     // Load VR stereo config for VR simulator device parameters
RLAPI void rlUnloadVrStereoConfig(rlVrStereoConfig config);           // Unload VR stereo config

// rlShader management functions
// NOTE: rlShader functionality is not available on OpenGL 1.1
RLAPI rlShader rlLoadShader(const char *vsFileName, const char *fsFileName);   // Load shader from files and bind default locations
RLAPI rlShader rlLoadShaderFromMemory(const char *vsCode, const char *fsCode); // Load shader from code strings and bind default locations
RLAPI bool rlIsShaderReady(rlShader shader);                                   // Check if a shader is ready
RLAPI int rlGetShaderLocation(rlShader shader, const char *uniformName);       // Get shader uniform location
RLAPI int rlGetShaderLocationAttrib(rlShader shader, const char *attribName);  // Get shader attribute location
RLAPI void rlSetShaderValue(rlShader shader, int locIndex, const void *value, int uniformType);               // Set shader uniform value
RLAPI void rlSetShaderValueV(rlShader shader, int locIndex, const void *value, int uniformType, int count);   // Set shader uniform value vector
RLAPI void rlSetShaderValueMatrix(rlShader shader, int locIndex, rlMatrix mat);         // Set shader uniform value (matrix 4x4)
RLAPI void rlSetShaderValueTexture(rlShader shader, int locIndex, rlTexture2D texture); // Set shader uniform value for texture (sampler2d)
RLAPI void rlUnloadShader(rlShader shader);                                    // Unload shader from GPU memory (VRAM)

// Screen-space-related functions
RLAPI rlRay rlGetMouseRay(rlVector2 mousePosition, rlCamera camera);      // Get a ray trace from mouse position
RLAPI rlMatrix rlGetCameraMatrix(rlCamera camera);                      // Get camera transform matrix (view matrix)
RLAPI rlMatrix rlGetCameraMatrix2D(rlCamera2D camera);                  // Get camera 2d transform matrix
RLAPI rlVector2 rlGetWorldToScreen(rlVector3 position, rlCamera camera);  // Get the screen space position for a 3d world space position
RLAPI rlVector2 rlGetScreenToWorld2D(rlVector2 position, rlCamera2D camera); // Get the world space position for a 2d camera screen space position
RLAPI rlVector2 rlGetWorldToScreenEx(rlVector3 position, rlCamera camera, int width, int height); // Get size position for a 3d world space position
RLAPI rlVector2 rlGetWorldToScreen2D(rlVector2 position, rlCamera2D camera); // Get the screen space position for a 2d camera world space position

// Timing-related functions
RLAPI void rlSetTargetFPS(int fps);                                 // Set target FPS (maximum)
RLAPI float rlGetFrameTime(void);                                   // Get time in seconds for last frame drawn (delta time)
RLAPI double rlGetTime(void);                                       // Get elapsed time in seconds since rlInitWindow()
RLAPI int rlGetFPS(void);                                           // Get current FPS

// Custom frame control functions
// NOTE: Those functions are intended for advance users that want full control over the frame processing
// By default rlEndDrawing() does this job: draws everything + rlSwapScreenBuffer() + manage frame timing + rlPollInputEvents()
// To avoid that behaviour and control frame processes manually, enable in config.h: RL_SUPPORT_CUSTOM_FRAME_CONTROL
RLAPI void rlSwapScreenBuffer(void);                                // Swap back buffer with front buffer (screen drawing)
RLAPI void rlPollInputEvents(void);                                 // Register all input events
RLAPI void rlWaitTime(double seconds);                              // Wait for some time (halt program execution)

// Random values generation functions
RLAPI void rlSetRandomSeed(unsigned int seed);                      // Set the seed for the random number generator
RLAPI int rlGetRandomValue(int min, int max);                       // Get a random value between min and max (both included)
RLAPI int *LoadRandomSequence(unsigned int count, int min, int max); // Load random values sequence, no values repeated
RLAPI void UnloadRandomSequence(int *sequence);                   // Unload random values sequence

// Misc. functions
RLAPI void rlTakeScreenshot(const char *fileName);                  // Takes a screenshot of current screen (filename extension defines format)
RLAPI void rlSetConfigFlags(unsigned int flags);                    // Setup init configuration flags (view FLAGS)
RLAPI void rlOpenURL(const char *url);                              // Open URL with default system browser (if available)

// NOTE: Following functions implemented in module [utils]
//------------------------------------------------------------------
RLAPI void rlTraceLog(int logLevel, const char *text, ...);         // Show trace log messages (RL_LOG_DEBUG, RL_LOG_INFO, RL_LOG_WARNING, RL_LOG_ERROR...)
RLAPI void rlSetTraceLogLevel(int logLevel);                        // Set the current threshold (minimum) log level
RLAPI void *rlMemAlloc(unsigned int size);                          // Internal memory allocator
RLAPI void *rlMemRealloc(void *ptr, unsigned int size);             // Internal memory reallocator
RLAPI void rlMemFree(void *ptr);                                    // Internal memory free

// Set custom callbacks
// WARNING: Callbacks setup is intended for advance users
RLAPI void rlSetTraceLogCallback(rlTraceLogCallback callback);         // Set custom trace log
RLAPI void rlSetLoadFileDataCallback(rlLoadFileDataCallback callback); // Set custom file binary data loader
RLAPI void rlSetSaveFileDataCallback(rlSaveFileDataCallback callback); // Set custom file binary data saver
RLAPI void rlSetLoadFileTextCallback(rlLoadFileTextCallback callback); // Set custom file text data loader
RLAPI void rlSetSaveFileTextCallback(rlSaveFileTextCallback callback); // Set custom file text data saver

// Files management functions
RLAPI unsigned char *rlLoadFileData(const char *fileName, int *dataSize); // Load file data as byte array (read)
RLAPI void rlUnloadFileData(unsigned char *data);                   // Unload file data allocated by rlLoadFileData()
RLAPI bool rlSaveFileData(const char *fileName, void *data, int dataSize); // Save data to file from byte array (write), returns true on success
RLAPI bool rlExportDataAsCode(const unsigned char *data, int dataSize, const char *fileName); // Export data to code (.h), returns true on success
RLAPI char *rlLoadFileText(const char *fileName);                   // Load text data from file (read), returns a '\0' terminated string
RLAPI void rlUnloadFileText(char *text);                            // Unload file text data allocated by rlLoadFileText()
RLAPI bool rlSaveFileText(const char *fileName, char *text);        // Save text data to file (write), string must be '\0' terminated, returns true on success
//------------------------------------------------------------------

// File system functions
RLAPI bool rlFileExists(const char *fileName);                      // Check if file exists
RLAPI bool rlDirectoryExists(const char *dirPath);                  // Check if a directory path exists
RLAPI bool rlIsFileExtension(const char *fileName, const char *ext); // Check file extension (including point: .png, .wav)
RLAPI int rlGetFileLength(const char *fileName);                    // Get file length in bytes (NOTE: GetFileSize() conflicts with windows.h)
RLAPI const char *rlGetFileExtension(const char *fileName);         // Get pointer to extension for a filename string (includes dot: '.png')
RLAPI const char *rlGetFileName(const char *filePath);              // Get pointer to filename for a path string
RLAPI const char *rlGetFileNameWithoutExt(const char *filePath);    // Get filename string without extension (uses static string)
RLAPI const char *rlGetDirectoryPath(const char *filePath);         // Get full path for a given fileName with path (uses static string)
RLAPI const char *rlGetPrevDirectoryPath(const char *dirPath);      // Get previous directory path for a given path (uses static string)
RLAPI const char *rlGetWorkingDirectory(void);                      // Get current working directory (uses static string)
RLAPI const char *rlGetApplicationDirectory(void);                  // Get the directory of the running application (uses static string)
RLAPI bool rlChangeDirectory(const char *dir);                      // Change working directory, return true on success
RLAPI bool rlIsPathFile(const char *path);                          // Check if a given path is a file or a directory
RLAPI rlFilePathList rlLoadDirectoryFiles(const char *dirPath);       // Load directory filepaths
RLAPI rlFilePathList rlLoadDirectoryFilesEx(const char *basePath, const char *filter, bool scanSubdirs); // Load directory filepaths with extension filtering and recursive directory scan
RLAPI void rlUnloadDirectoryFiles(rlFilePathList files);              // Unload filepaths
RLAPI bool rlIsFileDropped(void);                                   // Check if a file has been dropped into window
RLAPI rlFilePathList rlLoadDroppedFiles(void);                        // Load dropped filepaths
RLAPI void rlUnloadDroppedFiles(rlFilePathList files);                // Unload dropped filepaths
RLAPI long rlGetFileModTime(const char *fileName);                  // Get file modification time (last write time)

// Compression/Encoding functionality
RLAPI unsigned char *rlCompressData(const unsigned char *data, int dataSize, int *compDataSize);        // Compress data (DEFLATE algorithm), memory must be rlMemFree()
RLAPI unsigned char *rlDecompressData(const unsigned char *compData, int compDataSize, int *dataSize);  // Decompress data (DEFLATE algorithm), memory must be rlMemFree()
RLAPI char *rlEncodeDataBase64(const unsigned char *data, int dataSize, int *outputSize);               // Encode data to Base64 string, memory must be rlMemFree()
RLAPI unsigned char *rlDecodeDataBase64(const unsigned char *data, int *outputSize);                    // Decode Base64 string data, memory must be rlMemFree()

// Automation events functionality
RLAPI rlAutomationEventList rlLoadAutomationEventList(const char *fileName);                // Load automation events list from file, NULL for empty list, capacity = RL_MAX_AUTOMATION_EVENTS
RLAPI void rlUnloadAutomationEventList(rlAutomationEventList *list);                        // Unload automation events list from file
RLAPI bool rlExportAutomationEventList(rlAutomationEventList list, const char *fileName);   // Export automation events list as text file
RLAPI void rlSetAutomationEventList(rlAutomationEventList *list);                           // Set automation event list to record to
RLAPI void rlSetAutomationEventBaseFrame(int frame);                                      // Set automation event internal base frame to start recording
RLAPI void rlStartAutomationEventRecording(void);                                         // Start recording automation events (rlAutomationEventList must be set)
RLAPI void rlStopAutomationEventRecording(void);                                          // Stop recording automation events
RLAPI void rlPlayAutomationEvent(rlAutomationEvent event);                                  // Play a recorded automation event

//------------------------------------------------------------------------------------
// Input Handling Functions (Module: core)
//------------------------------------------------------------------------------------

// Input-related functions: keyboard
RLAPI bool rlIsKeyPressed(int key);                             // Check if a key has been pressed once
RLAPI bool rlIsKeyPressedRepeat(int key);                       // Check if a key has been pressed again (Only PLATFORM_DESKTOP)
RLAPI bool rlIsKeyDown(int key);                                // Check if a key is being pressed
RLAPI bool rlIsKeyReleased(int key);                            // Check if a key has been released once
RLAPI bool rlIsKeyUp(int key);                                  // Check if a key is NOT being pressed
RLAPI int rlGetKeyPressed(void);                                // Get key pressed (keycode), call it multiple times for keys queued, returns 0 when the queue is empty
RLAPI int rlGetCharPressed(void);                               // Get char pressed (unicode), call it multiple times for chars queued, returns 0 when the queue is empty
RLAPI void rlSetExitKey(int key);                               // Set a custom key to exit program (default is ESC)

// Input-related functions: gamepads
RLAPI bool rlIsGamepadAvailable(int gamepad);                   // Check if a gamepad is available
RLAPI const char *rlGetGamepadName(int gamepad);                // Get gamepad internal name id
RLAPI bool rlIsGamepadButtonPressed(int gamepad, int button);   // Check if a gamepad button has been pressed once
RLAPI bool rlIsGamepadButtonDown(int gamepad, int button);      // Check if a gamepad button is being pressed
RLAPI bool rlIsGamepadButtonReleased(int gamepad, int button);  // Check if a gamepad button has been released once
RLAPI bool rlIsGamepadButtonUp(int gamepad, int button);        // Check if a gamepad button is NOT being pressed
RLAPI int rlGetGamepadButtonPressed(void);                      // Get the last gamepad button pressed
RLAPI int rlGetGamepadAxisCount(int gamepad);                   // Get gamepad axis count for a gamepad
RLAPI float rlGetGamepadAxisMovement(int gamepad, int axis);    // Get axis movement value for a gamepad axis
RLAPI int rlSetGamepadMappings(const char *mappings);           // Set internal gamepad mappings (SDL_GameControllerDB)

// Input-related functions: mouse
RLAPI bool rlIsMouseButtonPressed(int button);                  // Check if a mouse button has been pressed once
RLAPI bool rlIsMouseButtonDown(int button);                     // Check if a mouse button is being pressed
RLAPI bool rlIsMouseButtonReleased(int button);                 // Check if a mouse button has been released once
RLAPI bool rlIsMouseButtonUp(int button);                       // Check if a mouse button is NOT being pressed
RLAPI int rlGetMouseX(void);                                    // Get mouse position X
RLAPI int rlGetMouseY(void);                                    // Get mouse position Y
RLAPI rlVector2 rlGetMousePosition(void);                         // Get mouse position XY
RLAPI rlVector2 rlGetMouseDelta(void);                            // Get mouse delta between frames
RLAPI void rlSetMousePosition(int x, int y);                    // Set mouse position XY
RLAPI void rlSetMouseOffset(int offsetX, int offsetY);          // Set mouse offset
RLAPI void rlSetMouseScale(float scaleX, float scaleY);         // Set mouse scaling
RLAPI float rlGetMouseWheelMove(void);                          // Get mouse wheel movement for X or Y, whichever is larger
RLAPI rlVector2 rlGetMouseWheelMoveV(void);                       // Get mouse wheel movement for both X and Y
RLAPI void rlSetMouseCursor(int cursor);                        // Set mouse cursor

// Input-related functions: touch
RLAPI int rlGetTouchX(void);                                    // Get touch position X for touch point 0 (relative to screen size)
RLAPI int rlGetTouchY(void);                                    // Get touch position Y for touch point 0 (relative to screen size)
RLAPI rlVector2 rlGetTouchPosition(int index);                    // Get touch position XY for a touch point index (relative to screen size)
RLAPI int rlGetTouchPointId(int index);                         // Get touch point identifier for given index
RLAPI int rlGetTouchPointCount(void);                           // Get number of touch points

//------------------------------------------------------------------------------------
// Gestures and Touch Handling Functions (Module: rgestures)
//------------------------------------------------------------------------------------
RLAPI void rlSetGesturesEnabled(unsigned int flags);      // Enable a set of gestures using flags
RLAPI bool rlIsGestureDetected(unsigned int gesture);     // Check if a gesture have been detected
RLAPI int rlGetGestureDetected(void);                     // Get latest detected gesture
RLAPI float rlGetGestureHoldDuration(void);               // Get gesture hold time in milliseconds
RLAPI rlVector2 rlGetGestureDragVector(void);               // Get gesture drag vector
RLAPI float rlGetGestureDragAngle(void);                  // Get gesture drag angle
RLAPI rlVector2 rlGetGesturePinchVector(void);              // Get gesture pinch delta
RLAPI float rlGetGesturePinchAngle(void);                 // Get gesture pinch angle

//------------------------------------------------------------------------------------
// rlCamera System Functions (Module: rcamera)
//------------------------------------------------------------------------------------
RLAPI void rlUpdateCamera(rlCamera *camera, int mode);      // Update camera position for selected mode
RLAPI void rlUpdateCameraPro(rlCamera *camera, rlVector3 movement, rlVector3 rotation, float zoom); // Update camera movement/rotation

//------------------------------------------------------------------------------------
// Basic Shapes Drawing Functions (Module: shapes)
//------------------------------------------------------------------------------------
// Set texture and rectangle to be used on shapes drawing
// NOTE: It can be useful when using basic shapes and one single font,
// defining a font char white rectangle would allow drawing everything in a single draw call
RLAPI void rlSetShapesTexture(rlTexture2D texture, rlRectangle source);       // Set texture and rectangle to be used on shapes drawing

// Basic shapes drawing functions
RLAPI void rlDrawPixel(int posX, int posY, rlColor color);                                                   // Draw a pixel
RLAPI void rlDrawPixelV(rlVector2 position, rlColor color);                                                    // Draw a pixel (Vector version)
RLAPI void rlDrawLine(int startPosX, int startPosY, int endPosX, int endPosY, rlColor color);                // Draw a line
RLAPI void rlDrawLineV(rlVector2 startPos, rlVector2 endPos, rlColor color);                                     // Draw a line (using gl lines)
RLAPI void rlDrawLineEx(rlVector2 startPos, rlVector2 endPos, float thick, rlColor color);                       // Draw a line (using triangles/quads)
RLAPI void rlDrawLineStrip(rlVector2 *points, int pointCount, rlColor color);                                  // Draw lines sequence (using gl lines)
RLAPI void rlDrawLineBezier(rlVector2 startPos, rlVector2 endPos, float thick, rlColor color);                   // Draw line segment cubic-bezier in-out interpolation
RLAPI void rlDrawCircle(int centerX, int centerY, float radius, rlColor color);                              // Draw a color-filled circle
RLAPI void rlDrawCircleSector(rlVector2 center, float radius, float startAngle, float endAngle, int segments, rlColor color);      // Draw a piece of a circle
RLAPI void rlDrawCircleSectorLines(rlVector2 center, float radius, float startAngle, float endAngle, int segments, rlColor color); // Draw circle sector outline
RLAPI void rlDrawCircleGradient(int centerX, int centerY, float radius, rlColor color1, rlColor color2);       // Draw a gradient-filled circle
RLAPI void rlDrawCircleV(rlVector2 center, float radius, rlColor color);                                       // Draw a color-filled circle (Vector version)
RLAPI void rlDrawCircleLines(int centerX, int centerY, float radius, rlColor color);                         // Draw circle outline
RLAPI void rlDrawCircleLinesV(rlVector2 center, float radius, rlColor color);                                  // Draw circle outline (Vector version)
RLAPI void rlDrawEllipse(int centerX, int centerY, float radiusH, float radiusV, rlColor color);             // Draw ellipse
RLAPI void rlDrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV, rlColor color);        // Draw ellipse outline
RLAPI void rlDrawRing(rlVector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, rlColor color); // Draw ring
RLAPI void rlDrawRingLines(rlVector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, rlColor color);    // Draw ring outline
RLAPI void rlDrawRectangle(int posX, int posY, int width, int height, rlColor color);                        // Draw a color-filled rectangle
RLAPI void rlDrawRectangleV(rlVector2 position, rlVector2 size, rlColor color);                                  // Draw a color-filled rectangle (Vector version)
RLAPI void rlDrawRectangleRec(rlRectangle rec, rlColor color);                                                 // Draw a color-filled rectangle
RLAPI void rlDrawRectanglePro(rlRectangle rec, rlVector2 origin, float rotation, rlColor color);                 // Draw a color-filled rectangle with pro parameters
RLAPI void rlDrawRectangleGradientV(int posX, int posY, int width, int height, rlColor color1, rlColor color2);// Draw a vertical-gradient-filled rectangle
RLAPI void rlDrawRectangleGradientH(int posX, int posY, int width, int height, rlColor color1, rlColor color2);// Draw a horizontal-gradient-filled rectangle
RLAPI void rlDrawRectangleGradientEx(rlRectangle rec, rlColor col1, rlColor col2, rlColor col3, rlColor col4);       // Draw a gradient-filled rectangle with custom vertex colors
RLAPI void rlDrawRectangleLines(int posX, int posY, int width, int height, rlColor color);                   // Draw rectangle outline
RLAPI void rlDrawRectangleLinesEx(rlRectangle rec, float lineThick, rlColor color);                            // Draw rectangle outline with extended parameters
RLAPI void rlDrawRectangleRounded(rlRectangle rec, float roundness, int segments, rlColor color);              // Draw rectangle with rounded edges
RLAPI void rlDrawRectangleRoundedLines(rlRectangle rec, float roundness, int segments, float lineThick, rlColor color); // Draw rectangle with rounded edges outline
RLAPI void rlDrawTriangle(rlVector2 v1, rlVector2 v2, rlVector2 v3, rlColor color);                                // Draw a color-filled triangle (vertex in counter-clockwise order!)
RLAPI void rlDrawTriangleLines(rlVector2 v1, rlVector2 v2, rlVector2 v3, rlColor color);                           // Draw triangle outline (vertex in counter-clockwise order!)
RLAPI void rlDrawTriangleFan(rlVector2 *points, int pointCount, rlColor color);                                // Draw a triangle fan defined by points (first vertex is the center)
RLAPI void rlDrawTriangleStrip(rlVector2 *points, int pointCount, rlColor color);                              // Draw a triangle strip defined by points
RLAPI void rlDrawPoly(rlVector2 center, int sides, float radius, float rotation, rlColor color);               // Draw a regular polygon (Vector version)
RLAPI void rlDrawPolyLines(rlVector2 center, int sides, float radius, float rotation, rlColor color);          // Draw a polygon outline of n sides
RLAPI void rlDrawPolyLinesEx(rlVector2 center, int sides, float radius, float rotation, float lineThick, rlColor color); // Draw a polygon outline of n sides with extended parameters

// Splines drawing functions
RLAPI void DrawSplineLinear(rlVector2 *points, int pointCount, float thick, rlColor color);                  // Draw spline: Linear, minimum 2 points
RLAPI void DrawSplineBasis(rlVector2 *points, int pointCount, float thick, rlColor color);                   // Draw spline: B-Spline, minimum 4 points
RLAPI void DrawSplineCatmullRom(rlVector2 *points, int pointCount, float thick, rlColor color);              // Draw spline: Catmull-Rom, minimum 4 points
RLAPI void DrawSplineBezierQuadratic(rlVector2 *points, int pointCount, float thick, rlColor color);         // Draw spline: Quadratic Bezier, minimum 3 points (1 control point): [p1, c2, p3, c4...]
RLAPI void DrawSplineBezierCubic(rlVector2 *points, int pointCount, float thick, rlColor color);             // Draw spline: Cubic Bezier, minimum 4 points (2 control points): [p1, c2, c3, p4, c5, c6...]
RLAPI void DrawSplineSegmentLinear(rlVector2 p1, rlVector2 p2, float thick, rlColor color);                    // Draw spline segment: Linear, 2 points
RLAPI void DrawSplineSegmentBasis(rlVector2 p1, rlVector2 p2, rlVector2 p3, rlVector2 p4, float thick, rlColor color); // Draw spline segment: B-Spline, 4 points
RLAPI void DrawSplineSegmentCatmullRom(rlVector2 p1, rlVector2 p2, rlVector2 p3, rlVector2 p4, float thick, rlColor color); // Draw spline segment: Catmull-Rom, 4 points
RLAPI void DrawSplineSegmentBezierQuadratic(rlVector2 p1, rlVector2 c2, rlVector2 p3, float thick, rlColor color); // Draw spline segment: Quadratic Bezier, 2 points, 1 control point
RLAPI void DrawSplineSegmentBezierCubic(rlVector2 p1, rlVector2 c2, rlVector2 c3, rlVector2 p4, float thick, rlColor color); // Draw spline segment: Cubic Bezier, 2 points, 2 control points

// Spline segment point evaluation functions, for a given t [0.0f .. 1.0f]
RLAPI rlVector2 GetSplinePointLinear(rlVector2 startPos, rlVector2 endPos, float t);                           // Get (evaluate) spline point: Linear
RLAPI rlVector2 GetSplinePointBasis(rlVector2 p1, rlVector2 p2, rlVector2 p3, rlVector2 p4, float t);              // Get (evaluate) spline point: B-Spline
RLAPI rlVector2 GetSplinePointCatmullRom(rlVector2 p1, rlVector2 p2, rlVector2 p3, rlVector2 p4, float t);         // Get (evaluate) spline point: Catmull-Rom
RLAPI rlVector2 GetSplinePointBezierQuad(rlVector2 p1, rlVector2 c2, rlVector2 p3, float t);                     // Get (evaluate) spline point: Quadratic Bezier
RLAPI rlVector2 GetSplinePointBezierCubic(rlVector2 p1, rlVector2 c2, rlVector2 c3, rlVector2 p4, float t);        // Get (evaluate) spline point: Cubic Bezier

// Basic shapes collision detection functions
RLAPI bool rlCheckCollisionRecs(rlRectangle rec1, rlRectangle rec2);                                           // Check collision between two rectangles
RLAPI bool rlCheckCollisionCircles(rlVector2 center1, float radius1, rlVector2 center2, float radius2);        // Check collision between two circles
RLAPI bool rlCheckCollisionCircleRec(rlVector2 center, float radius, rlRectangle rec);                         // Check collision between circle and rectangle
RLAPI bool rlCheckCollisionPointRec(rlVector2 point, rlRectangle rec);                                         // Check if point is inside rectangle
RLAPI bool rlCheckCollisionPointCircle(rlVector2 point, rlVector2 center, float radius);                       // Check if point is inside circle
RLAPI bool rlCheckCollisionPointTriangle(rlVector2 point, rlVector2 p1, rlVector2 p2, rlVector2 p3);               // Check if point is inside a triangle
RLAPI bool rlCheckCollisionPointPoly(rlVector2 point, rlVector2 *points, int pointCount);                      // Check if point is within a polygon described by array of vertices
RLAPI bool rlCheckCollisionLines(rlVector2 startPos1, rlVector2 endPos1, rlVector2 startPos2, rlVector2 endPos2, rlVector2 *collisionPoint); // Check the collision between two lines defined by two points each, returns collision point by reference
RLAPI bool rlCheckCollisionPointLine(rlVector2 point, rlVector2 p1, rlVector2 p2, int threshold);                // Check if point belongs to line created between two points [p1] and [p2] with defined margin in pixels [threshold]
RLAPI rlRectangle rlGetCollisionRec(rlRectangle rec1, rlRectangle rec2);                                         // Get collision rectangle for two rectangles collision

//------------------------------------------------------------------------------------
// rlTexture Loading and Drawing Functions (Module: textures)
//------------------------------------------------------------------------------------

// rlImage loading functions
// NOTE: These functions do not require GPU access
RLAPI rlImage rlLoadImage(const char *fileName);                                                             // Load image from file into CPU memory (RAM)
RLAPI rlImage rlLoadImageRaw(const char *fileName, int width, int height, int format, int headerSize);       // Load image from RAW file data
RLAPI rlImage rlLoadImageSvg(const char *fileNameOrString, int width, int height);                           // Load image from SVG file data or string with specified size
RLAPI rlImage rlLoadImageAnim(const char *fileName, int *frames);                                            // Load image sequence from file (frames appended to image.data)
RLAPI rlImage rlLoadImageFromMemory(const char *fileType, const unsigned char *fileData, int dataSize);      // Load image from memory buffer, fileType refers to extension: i.e. '.png'
RLAPI rlImage rlLoadImageFromTexture(rlTexture2D texture);                                                     // Load image from GPU texture data
RLAPI rlImage rlLoadImageFromScreen(void);                                                                   // Load image from screen buffer and (screenshot)
RLAPI bool rlIsImageReady(rlImage image);                                                                    // Check if an image is ready
RLAPI void rlUnloadImage(rlImage image);                                                                     // Unload image from CPU memory (RAM)
RLAPI bool rlExportImage(rlImage image, const char *fileName);                                               // Export image data to file, returns true on success
RLAPI unsigned char *rlExportImageToMemory(rlImage image, const char *fileType, int *fileSize);              // Export image to memory buffer
RLAPI bool rlExportImageAsCode(rlImage image, const char *fileName);                                         // Export image as code file defining an array of bytes, returns true on success

// rlImage generation functions
RLAPI rlImage rlGenImageColor(int width, int height, rlColor color);                                           // Generate image: plain color
RLAPI rlImage rlGenImageGradientLinear(int width, int height, int direction, rlColor start, rlColor end);        // Generate image: linear gradient, direction in degrees [0..360], 0=Vertical gradient
RLAPI rlImage rlGenImageGradientRadial(int width, int height, float density, rlColor inner, rlColor outer);      // Generate image: radial gradient
RLAPI rlImage rlGenImageGradientSquare(int width, int height, float density, rlColor inner, rlColor outer);      // Generate image: square gradient
RLAPI rlImage rlGenImageChecked(int width, int height, int checksX, int checksY, rlColor col1, rlColor col2);    // Generate image: checked
RLAPI rlImage rlGenImageWhiteNoise(int width, int height, float factor);                                     // Generate image: white noise
RLAPI rlImage rlGenImagePerlinNoise(int width, int height, int offsetX, int offsetY, float scale);           // Generate image: perlin noise
RLAPI rlImage rlGenImageCellular(int width, int height, int tileSize);                                       // Generate image: cellular algorithm, bigger tileSize means bigger cells
RLAPI rlImage rlGenImageText(int width, int height, const char *text);                                       // Generate image: grayscale image from text data

// rlImage manipulation functions
RLAPI rlImage rlImageCopy(rlImage image);                                                                      // Create an image duplicate (useful for transformations)
RLAPI rlImage rlImageFromImage(rlImage image, rlRectangle rec);                                                  // Create an image from another image piece
RLAPI rlImage rlImageText(const char *text, int fontSize, rlColor color);                                      // Create an image from text (default font)
RLAPI rlImage rlImageTextEx(rlFont font, const char *text, float fontSize, float spacing, rlColor tint);         // Create an image from text (custom sprite font)
RLAPI void rlImageFormat(rlImage *image, int newFormat);                                                     // Convert image data to desired format
RLAPI void rlImageToPOT(rlImage *image, rlColor fill);                                                         // Convert image to POT (power-of-two)
RLAPI void rlImageCrop(rlImage *image, rlRectangle crop);                                                      // Crop an image to a defined rectangle
RLAPI void rlImageAlphaCrop(rlImage *image, float threshold);                                                // Crop image depending on alpha value
RLAPI void rlImageAlphaClear(rlImage *image, rlColor color, float threshold);                                  // Clear alpha channel to desired color
RLAPI void rlImageAlphaMask(rlImage *image, rlImage alphaMask);                                                // Apply alpha mask to image
RLAPI void rlImageAlphaPremultiply(rlImage *image);                                                          // Premultiply alpha channel
RLAPI void rlImageBlurGaussian(rlImage *image, int blurSize);                                                // Apply Gaussian blur using a box blur approximation
RLAPI void rlImageResize(rlImage *image, int newWidth, int newHeight);                                       // Resize image (Bicubic scaling algorithm)
RLAPI void rlImageResizeNN(rlImage *image, int newWidth,int newHeight);                                      // Resize image (Nearest-Neighbor scaling algorithm)
RLAPI void rlImageResizeCanvas(rlImage *image, int newWidth, int newHeight, int offsetX, int offsetY, rlColor fill);  // Resize canvas and fill with color
RLAPI void rlImageMipmaps(rlImage *image);                                                                   // Compute all mipmap levels for a provided image
RLAPI void rlImageDither(rlImage *image, int rBpp, int gBpp, int bBpp, int aBpp);                            // Dither image data to 16bpp or lower (Floyd-Steinberg dithering)
RLAPI void rlImageFlipVertical(rlImage *image);                                                              // Flip image vertically
RLAPI void rlImageFlipHorizontal(rlImage *image);                                                            // Flip image horizontally
RLAPI void rlImageRotate(rlImage *image, int degrees);                                                       // Rotate image by input angle in degrees (-359 to 359)
RLAPI void rlImageRotateCW(rlImage *image);                                                                  // Rotate image clockwise 90deg
RLAPI void rlImageRotateCCW(rlImage *image);                                                                 // Rotate image counter-clockwise 90deg
RLAPI void rlImageColorTint(rlImage *image, rlColor color);                                                    // Modify image color: tint
RLAPI void rlImageColorInvert(rlImage *image);                                                               // Modify image color: invert
RLAPI void rlImageColorGrayscale(rlImage *image);                                                            // Modify image color: grayscale
RLAPI void rlImageColorContrast(rlImage *image, float contrast);                                             // Modify image color: contrast (-100 to 100)
RLAPI void rlImageColorBrightness(rlImage *image, int brightness);                                           // Modify image color: brightness (-255 to 255)
RLAPI void rlImageColorReplace(rlImage *image, rlColor color, rlColor replace);                                  // Modify image color: replace color
RLAPI rlColor *rlLoadImageColors(rlImage image);                                                               // Load color data from image as a rlColor array (RGBA - 32bit)
RLAPI rlColor *rlLoadImagePalette(rlImage image, int maxPaletteSize, int *colorCount);                         // Load colors palette from image as a rlColor array (RGBA - 32bit)
RLAPI void rlUnloadImageColors(rlColor *colors);                                                             // Unload color data loaded with rlLoadImageColors()
RLAPI void rlUnloadImagePalette(rlColor *colors);                                                            // Unload colors palette loaded with rlLoadImagePalette()
RLAPI rlRectangle rlGetImageAlphaBorder(rlImage image, float threshold);                                       // Get image alpha border rectangle
RLAPI rlColor rlGetImageColor(rlImage image, int x, int y);                                                    // Get image pixel color at (x, y) position

// rlImage drawing functions
// NOTE: rlImage software-rendering functions (CPU)
RLAPI void rlImageClearBackground(rlImage *dst, rlColor color);                                                // Clear image background with given color
RLAPI void rlImageDrawPixel(rlImage *dst, int posX, int posY, rlColor color);                                  // Draw pixel within an image
RLAPI void rlImageDrawPixelV(rlImage *dst, rlVector2 position, rlColor color);                                   // Draw pixel within an image (Vector version)
RLAPI void rlImageDrawLine(rlImage *dst, int startPosX, int startPosY, int endPosX, int endPosY, rlColor color); // Draw line within an image
RLAPI void rlImageDrawLineV(rlImage *dst, rlVector2 start, rlVector2 end, rlColor color);                          // Draw line within an image (Vector version)
RLAPI void rlImageDrawCircle(rlImage *dst, int centerX, int centerY, int radius, rlColor color);               // Draw a filled circle within an image
RLAPI void rlImageDrawCircleV(rlImage *dst, rlVector2 center, int radius, rlColor color);                        // Draw a filled circle within an image (Vector version)
RLAPI void rlImageDrawCircleLines(rlImage *dst, int centerX, int centerY, int radius, rlColor color);          // Draw circle outline within an image
RLAPI void rlImageDrawCircleLinesV(rlImage *dst, rlVector2 center, int radius, rlColor color);                   // Draw circle outline within an image (Vector version)
RLAPI void rlImageDrawRectangle(rlImage *dst, int posX, int posY, int width, int height, rlColor color);       // Draw rectangle within an image
RLAPI void rlImageDrawRectangleV(rlImage *dst, rlVector2 position, rlVector2 size, rlColor color);                 // Draw rectangle within an image (Vector version)
RLAPI void rlImageDrawRectangleRec(rlImage *dst, rlRectangle rec, rlColor color);                                // Draw rectangle within an image
RLAPI void rlImageDrawRectangleLines(rlImage *dst, rlRectangle rec, int thick, rlColor color);                   // Draw rectangle lines within an image
RLAPI void rlImageDraw(rlImage *dst, rlImage src, rlRectangle srcRec, rlRectangle dstRec, rlColor tint);             // Draw a source image within a destination image (tint applied to source)
RLAPI void rlImageDrawText(rlImage *dst, const char *text, int posX, int posY, int fontSize, rlColor color);   // Draw text (using default font) within an image (destination)
RLAPI void rlImageDrawTextEx(rlImage *dst, rlFont font, const char *text, rlVector2 position, float fontSize, float spacing, rlColor tint); // Draw text (custom sprite font) within an image (destination)

// rlTexture loading functions
// NOTE: These functions require GPU access
RLAPI rlTexture2D rlLoadTexture(const char *fileName);                                                       // Load texture from file into GPU memory (VRAM)
RLAPI rlTexture2D rlLoadTextureFromImage(rlImage image);                                                       // Load texture from image data
RLAPI rlTextureCubemap rlLoadTextureCubemap(rlImage image, int layout);                                        // Load cubemap from image, multiple image cubemap layouts supported
RLAPI rlRenderTexture2D rlLoadRenderTexture(int width, int height);                                          // Load texture for rendering (framebuffer)
RLAPI bool rlIsTextureReady(rlTexture2D texture);                                                            // Check if a texture is ready
RLAPI void rlUnloadTexture(rlTexture2D texture);                                                             // Unload texture from GPU memory (VRAM)
RLAPI bool rlIsRenderTextureReady(rlRenderTexture2D target);                                                 // Check if a render texture is ready
RLAPI void rlUnloadRenderTexture(rlRenderTexture2D target);                                                  // Unload render texture from GPU memory (VRAM)
RLAPI void rlUpdateTexture(rlTexture2D texture, const void *pixels);                                         // Update GPU texture with new data
RLAPI void rlUpdateTextureRec(rlTexture2D texture, rlRectangle rec, const void *pixels);                       // Update GPU texture rectangle with new data

// rlTexture configuration functions
RLAPI void rlGenTextureMipmaps(rlTexture2D *texture);                                                        // Generate GPU mipmaps for a texture
RLAPI void rlSetTextureFilter(rlTexture2D texture, int filter);                                              // Set texture scaling filter mode
RLAPI void rlSetTextureWrap(rlTexture2D texture, int wrap);                                                  // Set texture wrapping mode

// rlTexture drawing functions
RLAPI void rlDrawTexture(rlTexture2D texture, int posX, int posY, rlColor tint);                               // Draw a rlTexture2D
RLAPI void rlDrawTextureV(rlTexture2D texture, rlVector2 position, rlColor tint);                                // Draw a rlTexture2D with position defined as rlVector2
RLAPI void rlDrawTextureEx(rlTexture2D texture, rlVector2 position, float rotation, float scale, rlColor tint);  // Draw a rlTexture2D with extended parameters
RLAPI void rlDrawTextureRec(rlTexture2D texture, rlRectangle source, rlVector2 position, rlColor tint);            // Draw a part of a texture defined by a rectangle
RLAPI void rlDrawTexturePro(rlTexture2D texture, rlRectangle source, rlRectangle dest, rlVector2 origin, float rotation, rlColor tint); // Draw a part of a texture defined by a rectangle with 'pro' parameters
RLAPI void rlDrawTextureNPatch(rlTexture2D texture, rlNPatchInfo nPatchInfo, rlRectangle dest, rlVector2 origin, float rotation, rlColor tint); // Draws a texture (or part of it) that stretches or shrinks nicely

// rlColor/pixel related functions
RLAPI rlColor rlFade(rlColor color, float alpha);                                 // Get color with alpha applied, alpha goes from 0.0f to 1.0f
RLAPI int rlColorToInt(rlColor color);                                          // Get hexadecimal value for a rlColor
RLAPI rlVector4 rlColorNormalize(rlColor color);                                  // Get rlColor normalized as float [0..1]
RLAPI rlColor rlColorFromNormalized(rlVector4 normalized);                        // Get rlColor from normalized values [0..1]
RLAPI rlVector3 rlColorToHSV(rlColor color);                                      // Get HSV values for a rlColor, hue [0..360], saturation/value [0..1]
RLAPI rlColor rlColorFromHSV(float hue, float saturation, float value);         // Get a rlColor from HSV values, hue [0..360], saturation/value [0..1]
RLAPI rlColor rlColorTint(rlColor color, rlColor tint);                             // Get color multiplied with another color
RLAPI rlColor rlColorBrightness(rlColor color, float factor);                     // Get color with brightness correction, brightness factor goes from -1.0f to 1.0f
RLAPI rlColor rlColorContrast(rlColor color, float contrast);                     // Get color with contrast correction, contrast values between -1.0f and 1.0f
RLAPI rlColor rlColorAlpha(rlColor color, float alpha);                           // Get color with alpha applied, alpha goes from 0.0f to 1.0f
RLAPI rlColor rlColorAlphaBlend(rlColor dst, rlColor src, rlColor tint);              // Get src alpha-blended into dst color with tint
RLAPI rlColor rlGetColor(unsigned int hexValue);                                // Get rlColor structure from hexadecimal value
RLAPI rlColor rlGetPixelColor(void *srcPtr, int format);                        // Get rlColor from a source pixel pointer of certain format
RLAPI void rlSetPixelColor(void *dstPtr, rlColor color, int format);            // Set color formatted into destination pixel pointer
RLAPI int rlGetPixelDataSize(int width, int height, int format);              // Get pixel data size in bytes for certain format

//------------------------------------------------------------------------------------
// rlFont Loading and Text Drawing Functions (Module: text)
//------------------------------------------------------------------------------------

// rlFont loading/unloading functions
RLAPI rlFont rlGetFontDefault(void);                                                            // Get the default rlFont
RLAPI rlFont rlLoadFont(const char *fileName);                                                  // Load font from file into GPU memory (VRAM)
RLAPI rlFont rlLoadFontEx(const char *fileName, int fontSize, int *codepoints, int codepointCount);  // Load font from file with extended parameters, use NULL for codepoints and 0 for codepointCount to load the default character set
RLAPI rlFont rlLoadFontFromImage(rlImage image, rlColor key, int firstChar);                        // Load font from rlImage (XNA style)
RLAPI rlFont rlLoadFontFromMemory(const char *fileType, const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount); // Load font from memory buffer, fileType refers to extension: i.e. '.ttf'
RLAPI bool rlIsFontReady(rlFont font);                                                          // Check if a font is ready
RLAPI rlGlyphInfo *rlLoadFontData(const unsigned char *fileData, int dataSize, int fontSize, int *codepoints, int codepointCount, int type); // Load font data for further use
RLAPI rlImage rlGenImageFontAtlas(const rlGlyphInfo *glyphs, rlRectangle **glyphRecs, int glyphCount, int fontSize, int padding, int packMethod); // Generate image font atlas using chars info
RLAPI void rlUnloadFontData(rlGlyphInfo *glyphs, int glyphCount);                               // Unload font chars info data (RAM)
RLAPI void rlUnloadFont(rlFont font);                                                           // Unload font from GPU memory (VRAM)
RLAPI bool rlExportFontAsCode(rlFont font, const char *fileName);                               // Export font as code file, returns true on success

// Text drawing functions
RLAPI void rlDrawFPS(int posX, int posY);                                                     // Draw current FPS
RLAPI void rlDrawText(const char *text, int posX, int posY, int fontSize, rlColor color);       // Draw text (using default font)
RLAPI void rlDrawTextEx(rlFont font, const char *text, rlVector2 position, float fontSize, float spacing, rlColor tint); // Draw text using font and additional parameters
RLAPI void rlDrawTextPro(rlFont font, const char *text, rlVector2 position, rlVector2 origin, float rotation, float fontSize, float spacing, rlColor tint); // Draw text using rlFont and pro parameters (rotation)
RLAPI void rlDrawTextCodepoint(rlFont font, int codepoint, rlVector2 position, float fontSize, rlColor tint); // Draw one character (codepoint)
RLAPI void rlDrawTextCodepoints(rlFont font, const int *codepoints, int codepointCount, rlVector2 position, float fontSize, float spacing, rlColor tint); // Draw multiple character (codepoint)

// Text font info functions
RLAPI void rlSetTextLineSpacing(int spacing);                                                 // Set vertical line spacing when drawing with line-breaks
RLAPI int rlMeasureText(const char *text, int fontSize);                                      // Measure string width for default font
RLAPI rlVector2 rlMeasureTextEx(rlFont font, const char *text, float fontSize, float spacing);    // Measure string size for rlFont
RLAPI int rlGetGlyphIndex(rlFont font, int codepoint);                                          // Get glyph index position in font for a codepoint (unicode character), fallback to '?' if not found
RLAPI rlGlyphInfo rlGetGlyphInfo(rlFont font, int codepoint);                                     // Get glyph font info data for a codepoint (unicode character), fallback to '?' if not found
RLAPI rlRectangle rlGetGlyphAtlasRec(rlFont font, int codepoint);                                 // Get glyph rectangle in font atlas for a codepoint (unicode character), fallback to '?' if not found

// Text codepoints management functions (unicode characters)
RLAPI char *rlLoadUTF8(const int *codepoints, int length);                // Load UTF-8 text encoded from codepoints array
RLAPI void rlUnloadUTF8(char *text);                                      // Unload UTF-8 text encoded from codepoints array
RLAPI int *rlLoadCodepoints(const char *text, int *count);                // Load all codepoints from a UTF-8 text string, codepoints count returned by parameter
RLAPI void rlUnloadCodepoints(int *codepoints);                           // Unload codepoints data from memory
RLAPI int rlGetCodepointCount(const char *text);                          // Get total number of codepoints in a UTF-8 encoded string
RLAPI int rlGetCodepoint(const char *text, int *codepointSize);           // Get next codepoint in a UTF-8 encoded string, 0x3f('?') is returned on failure
RLAPI int rlGetCodepointNext(const char *text, int *codepointSize);       // Get next codepoint in a UTF-8 encoded string, 0x3f('?') is returned on failure
RLAPI int rlGetCodepointPrevious(const char *text, int *codepointSize);   // Get previous codepoint in a UTF-8 encoded string, 0x3f('?') is returned on failure
RLAPI const char *rlCodepointToUTF8(int codepoint, int *utf8Size);        // Encode one codepoint into UTF-8 byte array (array length returned as parameter)

// Text strings management functions (no UTF-8 strings, only byte chars)
// NOTE: Some strings allocate memory internally for returned strings, just be careful!
RLAPI int rlTextCopy(char *dst, const char *src);                                             // Copy one string to another, returns bytes copied
RLAPI bool rlTextIsEqual(const char *text1, const char *text2);                               // Check if two text string are equal
RLAPI unsigned int rlTextLength(const char *text);                                            // Get text length, checks for '\0' ending
RLAPI const char *rlTextFormat(const char *text, ...);                                        // Text formatting with variables (sprintf() style)
RLAPI const char *rlTextSubtext(const char *text, int position, int length);                  // Get a piece of a text string
RLAPI char *rlTextReplace(char *text, const char *replace, const char *by);                   // Replace text string (WARNING: memory must be freed!)
RLAPI char *rlTextInsert(const char *text, const char *insert, int position);                 // Insert text in a position (WARNING: memory must be freed!)
RLAPI const char *rlTextJoin(const char **textList, int count, const char *delimiter);        // Join text strings with delimiter
RLAPI const char **TextSplit(const char *text, char delimiter, int *count);                 // Split text into multiple strings
RLAPI void rlTextAppend(char *text, const char *append, int *position);                       // Append text at specific position and move cursor!
RLAPI int rlTextFindIndex(const char *text, const char *find);                                // Find first text occurrence within a string
RLAPI const char *rlTextToUpper(const char *text);                      // Get upper case version of provided string
RLAPI const char *rlTextToLower(const char *text);                      // Get lower case version of provided string
RLAPI const char *rlTextToPascal(const char *text);                     // Get Pascal case notation version of provided string
RLAPI int rlTextToInteger(const char *text);                            // Get integer value from text (negative values not supported)

//------------------------------------------------------------------------------------
// Basic 3d Shapes Drawing Functions (Module: models)
//------------------------------------------------------------------------------------

// Basic geometric 3D shapes drawing functions
RLAPI void rlDrawLine3D(rlVector3 startPos, rlVector3 endPos, rlColor color);                                    // Draw a line in 3D world space
RLAPI void rlDrawPoint3D(rlVector3 position, rlColor color);                                                   // Draw a point in 3D space, actually a small line
RLAPI void rlDrawCircle3D(rlVector3 center, float radius, rlVector3 rotationAxis, float rotationAngle, rlColor color); // Draw a circle in 3D world space
RLAPI void rlDrawTriangle3D(rlVector3 v1, rlVector3 v2, rlVector3 v3, rlColor color);                              // Draw a color-filled triangle (vertex in counter-clockwise order!)
RLAPI void rlDrawTriangleStrip3D(rlVector3 *points, int pointCount, rlColor color);                            // Draw a triangle strip defined by points
RLAPI void rlDrawCube(rlVector3 position, float width, float height, float length, rlColor color);             // Draw cube
RLAPI void rlDrawCubeV(rlVector3 position, rlVector3 size, rlColor color);                                       // Draw cube (Vector version)
RLAPI void rlDrawCubeWires(rlVector3 position, float width, float height, float length, rlColor color);        // Draw cube wires
RLAPI void rlDrawCubeWiresV(rlVector3 position, rlVector3 size, rlColor color);                                  // Draw cube wires (Vector version)
RLAPI void rlDrawSphere(rlVector3 centerPos, float radius, rlColor color);                                     // Draw sphere
RLAPI void rlDrawSphereEx(rlVector3 centerPos, float radius, int rings, int slices, rlColor color);            // Draw sphere with extended parameters
RLAPI void rlDrawSphereWires(rlVector3 centerPos, float radius, int rings, int slices, rlColor color);         // Draw sphere wires
RLAPI void rlDrawCylinder(rlVector3 position, float radiusTop, float radiusBottom, float height, int slices, rlColor color); // Draw a cylinder/cone
RLAPI void rlDrawCylinderEx(rlVector3 startPos, rlVector3 endPos, float startRadius, float endRadius, int sides, rlColor color); // Draw a cylinder with base at startPos and top at endPos
RLAPI void rlDrawCylinderWires(rlVector3 position, float radiusTop, float radiusBottom, float height, int slices, rlColor color); // Draw a cylinder/cone wires
RLAPI void rlDrawCylinderWiresEx(rlVector3 startPos, rlVector3 endPos, float startRadius, float endRadius, int sides, rlColor color); // Draw a cylinder wires with base at startPos and top at endPos
RLAPI void rlDrawCapsule(rlVector3 startPos, rlVector3 endPos, float radius, int slices, int rings, rlColor color); // Draw a capsule with the center of its sphere caps at startPos and endPos
RLAPI void rlDrawCapsuleWires(rlVector3 startPos, rlVector3 endPos, float radius, int slices, int rings, rlColor color); // Draw capsule wireframe with the center of its sphere caps at startPos and endPos
RLAPI void rlDrawPlane(rlVector3 centerPos, rlVector2 size, rlColor color);                                      // Draw a plane XZ
RLAPI void rlDrawRay(rlRay ray, rlColor color);                                                                // Draw a ray line
RLAPI void rlDrawGrid(int slices, float spacing);                                                          // Draw a grid (centered at (0, 0, 0))

//------------------------------------------------------------------------------------
// rlModel 3d Loading and Drawing Functions (Module: models)
//------------------------------------------------------------------------------------

// rlModel management functions
RLAPI rlModel rlLoadModel(const char *fileName);                                                // Load model from files (meshes and materials)
RLAPI rlModel rlLoadModelFromMesh(rlMesh mesh);                                                   // Load model from generated mesh (default material)
RLAPI bool rlIsModelReady(rlModel model);                                                       // Check if a model is ready
RLAPI void rlUnloadModel(rlModel model);                                                        // Unload model (including meshes) from memory (RAM and/or VRAM)
RLAPI rlBoundingBox rlGetModelBoundingBox(rlModel model);                                         // Compute model bounding box limits (considers all meshes)

// rlModel drawing functions
RLAPI void rlDrawModel(rlModel model, rlVector3 position, float scale, rlColor tint);               // Draw a model (with texture if set)
RLAPI void rlDrawModelEx(rlModel model, rlVector3 position, rlVector3 rotationAxis, float rotationAngle, rlVector3 scale, rlColor tint); // Draw a model with extended parameters
RLAPI void rlDrawModelWires(rlModel model, rlVector3 position, float scale, rlColor tint);          // Draw a model wires (with texture if set)
RLAPI void rlDrawModelWiresEx(rlModel model, rlVector3 position, rlVector3 rotationAxis, float rotationAngle, rlVector3 scale, rlColor tint); // Draw a model wires (with texture if set) with extended parameters
RLAPI void rlDrawBoundingBox(rlBoundingBox box, rlColor color);                                   // Draw bounding box (wires)
RLAPI void rlDrawBillboard(rlCamera camera, rlTexture2D texture, rlVector3 position, float size, rlColor tint);   // Draw a billboard texture
RLAPI void rlDrawBillboardRec(rlCamera camera, rlTexture2D texture, rlRectangle source, rlVector3 position, rlVector2 size, rlColor tint); // Draw a billboard texture defined by source
RLAPI void rlDrawBillboardPro(rlCamera camera, rlTexture2D texture, rlRectangle source, rlVector3 position, rlVector3 up, rlVector2 size, rlVector2 origin, float rotation, rlColor tint); // Draw a billboard texture defined by source and rotation

// rlMesh management functions
RLAPI void rlUploadMesh(rlMesh *mesh, bool dynamic);                                            // Upload mesh vertex data in GPU and provide VAO/VBO ids
RLAPI void rlUpdateMeshBuffer(rlMesh mesh, int index, const void *data, int dataSize, int offset); // Update mesh vertex data in GPU for a specific buffer index
RLAPI void rlUnloadMesh(rlMesh mesh);                                                           // Unload mesh data from CPU and GPU
RLAPI void rlDrawMesh(rlMesh mesh, rlMaterial material, rlMatrix transform);                        // Draw a 3d mesh with material and transform
RLAPI void rlDrawMeshInstanced(rlMesh mesh, rlMaterial material, const rlMatrix *transforms, int instances); // Draw multiple mesh instances with material and different transforms
RLAPI bool rlExportMesh(rlMesh mesh, const char *fileName);                                     // Export mesh data to file, returns true on success
RLAPI rlBoundingBox rlGetMeshBoundingBox(rlMesh mesh);                                            // Compute mesh bounding box limits
RLAPI void rlGenMeshTangents(rlMesh *mesh);                                                     // Compute mesh tangents

// rlMesh generation functions
RLAPI rlMesh rlGenMeshPoly(int sides, float radius);                                            // Generate polygonal mesh
RLAPI rlMesh rlGenMeshPlane(float width, float length, int resX, int resZ);                     // Generate plane mesh (with subdivisions)
RLAPI rlMesh rlGenMeshCube(float width, float height, float length);                            // Generate cuboid mesh
RLAPI rlMesh rlGenMeshSphere(float radius, int rings, int slices);                              // Generate sphere mesh (standard sphere)
RLAPI rlMesh rlGenMeshHemiSphere(float radius, int rings, int slices);                          // Generate half-sphere mesh (no bottom cap)
RLAPI rlMesh rlGenMeshCylinder(float radius, float height, int slices);                         // Generate cylinder mesh
RLAPI rlMesh rlGenMeshCone(float radius, float height, int slices);                             // Generate cone/pyramid mesh
RLAPI rlMesh rlGenMeshTorus(float radius, float size, int radSeg, int sides);                   // Generate torus mesh
RLAPI rlMesh rlGenMeshKnot(float radius, float size, int radSeg, int sides);                    // Generate trefoil knot mesh
RLAPI rlMesh rlGenMeshHeightmap(rlImage heightmap, rlVector3 size);                                 // Generate heightmap mesh from image data
RLAPI rlMesh rlGenMeshCubicmap(rlImage cubicmap, rlVector3 cubeSize);                               // Generate cubes-based map mesh from image data

// rlMaterial loading/unloading functions
RLAPI rlMaterial *rlLoadMaterials(const char *fileName, int *materialCount);                    // Load materials from model file
RLAPI rlMaterial rlLoadMaterialDefault(void);                                                   // Load default material (Supports: DIFFUSE, SPECULAR, NORMAL maps)
RLAPI bool rlIsMaterialReady(rlMaterial material);                                              // Check if a material is ready
RLAPI void rlUnloadMaterial(rlMaterial material);                                               // Unload material from GPU memory (VRAM)
RLAPI void rlSetMaterialTexture(rlMaterial *material, int mapType, rlTexture2D texture);          // Set texture for a material map type (RL_MATERIAL_MAP_DIFFUSE, RL_MATERIAL_MAP_SPECULAR...)
RLAPI void rlSetModelMeshMaterial(rlModel *model, int meshId, int materialId);                  // Set material for a mesh

// rlModel animations loading/unloading functions
RLAPI rlModelAnimation *rlLoadModelAnimations(const char *fileName, int *animCount);            // Load model animations from file
RLAPI void rlUpdateModelAnimation(rlModel model, rlModelAnimation anim, int frame);               // Update model animation pose
RLAPI void rlUnloadModelAnimation(rlModelAnimation anim);                                       // Unload animation data
RLAPI void rlUnloadModelAnimations(rlModelAnimation *animations, int animCount);                // Unload animation array data
RLAPI bool rlIsModelAnimationValid(rlModel model, rlModelAnimation anim);                         // Check model animation skeleton match

// Collision detection functions
RLAPI bool rlCheckCollisionSpheres(rlVector3 center1, float radius1, rlVector3 center2, float radius2);   // Check collision between two spheres
RLAPI bool rlCheckCollisionBoxes(rlBoundingBox box1, rlBoundingBox box2);                                 // Check collision between two bounding boxes
RLAPI bool rlCheckCollisionBoxSphere(rlBoundingBox box, rlVector3 center, float radius);                  // Check collision between box and sphere
RLAPI rlRayCollision rlGetRayCollisionSphere(rlRay ray, rlVector3 center, float radius);                    // Get collision info between ray and sphere
RLAPI rlRayCollision rlGetRayCollisionBox(rlRay ray, rlBoundingBox box);                                    // Get collision info between ray and box
RLAPI rlRayCollision rlGetRayCollisionMesh(rlRay ray, rlMesh mesh, rlMatrix transform);                       // Get collision info between ray and mesh
RLAPI rlRayCollision rlGetRayCollisionTriangle(rlRay ray, rlVector3 p1, rlVector3 p2, rlVector3 p3);            // Get collision info between ray and triangle
RLAPI rlRayCollision rlGetRayCollisionQuad(rlRay ray, rlVector3 p1, rlVector3 p2, rlVector3 p3, rlVector3 p4);    // Get collision info between ray and quad

//------------------------------------------------------------------------------------
// Audio Loading and Playing Functions (Module: audio)
//------------------------------------------------------------------------------------
typedef void (*AudioCallback)(void *bufferData, unsigned int frames);

// Audio device management functions
RLAPI void rlInitAudioDevice(void);                                     // Initialize audio device and context
RLAPI void rlCloseAudioDevice(void);                                    // Close the audio device and context
RLAPI bool rlIsAudioDeviceReady(void);                                  // Check if audio device has been initialized successfully
RLAPI void rlSetMasterVolume(float volume);                             // Set master volume (listener)
RLAPI float rlGetMasterVolume(void);                                    // Get master volume (listener)

// rlWave/rlSound loading/unloading functions
RLAPI rlWave rlLoadWave(const char *fileName);                            // Load wave data from file
RLAPI rlWave rlLoadWaveFromMemory(const char *fileType, const unsigned char *fileData, int dataSize); // Load wave from memory buffer, fileType refers to extension: i.e. '.wav'
RLAPI bool rlIsWaveReady(rlWave wave);                                    // Checks if wave data is ready
RLAPI rlSound rlLoadSound(const char *fileName);                          // Load sound from file
RLAPI rlSound rlLoadSoundFromWave(rlWave wave);                             // Load sound from wave data
RLAPI rlSound rlLoadSoundAlias(rlSound source);                             // Create a new sound that shares the same sample data as the source sound, does not own the sound data
RLAPI bool rlIsSoundReady(rlSound sound);                                 // Checks if a sound is ready
RLAPI void rlUpdateSound(rlSound sound, const void *data, int sampleCount); // Update sound buffer with new data
RLAPI void rlUnloadWave(rlWave wave);                                     // Unload wave data
RLAPI void rlUnloadSound(rlSound sound);                                  // Unload sound
RLAPI void rlUnloadSoundAlias(rlSound alias);                             // Unload a sound alias (does not deallocate sample data)
RLAPI bool rlExportWave(rlWave wave, const char *fileName);               // Export wave data to file, returns true on success
RLAPI bool rlExportWaveAsCode(rlWave wave, const char *fileName);         // Export wave sample data to code (.h), returns true on success

// rlWave/rlSound management functions
RLAPI void rlPlaySound(rlSound sound);                                    // Play a sound
RLAPI void rlStopSound(rlSound sound);                                    // Stop playing a sound
RLAPI void rlPauseSound(rlSound sound);                                   // Pause a sound
RLAPI void rlResumeSound(rlSound sound);                                  // Resume a paused sound
RLAPI bool rlIsSoundPlaying(rlSound sound);                               // Check if a sound is currently playing
RLAPI void rlSetSoundVolume(rlSound sound, float volume);                 // Set volume for a sound (1.0 is max level)
RLAPI void rlSetSoundPitch(rlSound sound, float pitch);                   // Set pitch for a sound (1.0 is base level)
RLAPI void rlSetSoundPan(rlSound sound, float pan);                       // Set pan for a sound (0.5 is center)
RLAPI rlWave rlWaveCopy(rlWave wave);                                       // Copy a wave to a new wave
RLAPI void rlWaveCrop(rlWave *wave, int initSample, int finalSample);     // Crop a wave to defined samples range
RLAPI void rlWaveFormat(rlWave *wave, int sampleRate, int sampleSize, int channels); // Convert wave data to desired format
RLAPI float *rlLoadWaveSamples(rlWave wave);                              // Load samples data from wave as a 32bit float data array
RLAPI void rlUnloadWaveSamples(float *samples);                         // Unload samples data loaded with rlLoadWaveSamples()

// rlMusic management functions
RLAPI rlMusic rlLoadMusicStream(const char *fileName);                    // Load music stream from file
RLAPI rlMusic rlLoadMusicStreamFromMemory(const char *fileType, const unsigned char *data, int dataSize); // Load music stream from data
RLAPI bool rlIsMusicReady(rlMusic music);                                 // Checks if a music stream is ready
RLAPI void rlUnloadMusicStream(rlMusic music);                            // Unload music stream
RLAPI void rlPlayMusicStream(rlMusic music);                              // Start music playing
RLAPI bool rlIsMusicStreamPlaying(rlMusic music);                         // Check if music is playing
RLAPI void rlUpdateMusicStream(rlMusic music);                            // Updates buffers for music streaming
RLAPI void rlStopMusicStream(rlMusic music);                              // Stop music playing
RLAPI void rlPauseMusicStream(rlMusic music);                             // Pause music playing
RLAPI void rlResumeMusicStream(rlMusic music);                            // Resume playing paused music
RLAPI void rlSeekMusicStream(rlMusic music, float position);              // Seek music to a position (in seconds)
RLAPI void rlSetMusicVolume(rlMusic music, float volume);                 // Set volume for music (1.0 is max level)
RLAPI void rlSetMusicPitch(rlMusic music, float pitch);                   // Set pitch for a music (1.0 is base level)
RLAPI void rlSetMusicPan(rlMusic music, float pan);                       // Set pan for a music (0.5 is center)
RLAPI float rlGetMusicTimeLength(rlMusic music);                          // Get music time length (in seconds)
RLAPI float rlGetMusicTimePlayed(rlMusic music);                          // Get current music time played (in seconds)

// rlAudioStream management functions
RLAPI rlAudioStream rlLoadAudioStream(unsigned int sampleRate, unsigned int sampleSize, unsigned int channels); // Load audio stream (to stream raw audio pcm data)
RLAPI bool rlIsAudioStreamReady(rlAudioStream stream);                    // Checks if an audio stream is ready
RLAPI void rlUnloadAudioStream(rlAudioStream stream);                     // Unload audio stream and free memory
RLAPI void rlUpdateAudioStream(rlAudioStream stream, const void *data, int frameCount); // Update audio stream buffers with data
RLAPI bool rlIsAudioStreamProcessed(rlAudioStream stream);                // Check if any audio stream buffers requires refill
RLAPI void rlPlayAudioStream(rlAudioStream stream);                       // Play audio stream
RLAPI void rlPauseAudioStream(rlAudioStream stream);                      // Pause audio stream
RLAPI void rlResumeAudioStream(rlAudioStream stream);                     // Resume audio stream
RLAPI bool rlIsAudioStreamPlaying(rlAudioStream stream);                  // Check if audio stream is playing
RLAPI void rlStopAudioStream(rlAudioStream stream);                       // Stop audio stream
RLAPI void rlSetAudioStreamVolume(rlAudioStream stream, float volume);    // Set volume for audio stream (1.0 is max level)
RLAPI void rlSetAudioStreamPitch(rlAudioStream stream, float pitch);      // Set pitch for audio stream (1.0 is base level)
RLAPI void rlSetAudioStreamPan(rlAudioStream stream, float pan);          // Set pan for audio stream (0.5 is centered)
RLAPI void rlSetAudioStreamBufferSizeDefault(int size);                 // Default size for new audio streams
RLAPI void rlSetAudioStreamCallback(rlAudioStream stream, AudioCallback callback); // Audio thread callback to request new data

RLAPI void rlAttachAudioStreamProcessor(rlAudioStream stream, AudioCallback processor); // Attach audio stream processor to stream, receives the samples as <float>s
RLAPI void rlDetachAudioStreamProcessor(rlAudioStream stream, AudioCallback processor); // Detach audio stream processor from stream

RLAPI void rlAttachAudioMixedProcessor(AudioCallback processor); // Attach audio stream processor to the entire audio pipeline, receives the samples as <float>s
RLAPI void rlDetachAudioMixedProcessor(AudioCallback processor); // Detach audio stream processor from the entire audio pipeline

RL_EXTERN_C_END
RL_NS_END

#endif // RAYLIB_H
