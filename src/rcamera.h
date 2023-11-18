/*******************************************************************************************
*
*   rcamera - Basic camera system with support for multiple camera modes
*
*   CONFIGURATION:
*       #define RCAMERA_IMPLEMENTATION
*           Generates the implementation of the library into the included file.
*           If not defined, the library is in header only mode and can be included in other headers
*           or source files without problems. But only ONE file should hold the implementation.
*
*       #define RCAMERA_STANDALONE
*           If defined, the library can be used as standalone as a camera system but some
*           functions must be redefined to manage inputs accordingly.
*
*   CONTRIBUTORS:
*       Ramon Santamaria:   Supervision, review, update and maintenance
*       Christoph Wagner:   Complete redesign, using raymath (2022)
*       Marc Palau:         Initial implementation (2014)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2022-2023 Christoph Wagner (@Crydsch) & Ramon Santamaria (@raysan5)
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

#ifndef RCAMERA_H
#define RCAMERA_H

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Function specifiers definition

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

#if defined(RCAMERA_STANDALONE)
    #define RL_CAMERA_CULL_DISTANCE_NEAR      0.01
    #define RL_CAMERA_CULL_DISTANCE_FAR    1000.0
#else
    #define RL_CAMERA_CULL_DISTANCE_NEAR   RL_CULL_DISTANCE_NEAR
    #define RL_CAMERA_CULL_DISTANCE_FAR    RL_CULL_DISTANCE_FAR
#endif

RL_NS_BEGIN

//----------------------------------------------------------------------------------
// Types and Structures Definition
// NOTE: Below types are required for standalone usage
//----------------------------------------------------------------------------------
#if defined(RCAMERA_STANDALONE)
    // Vector2, 2 components
    typedef struct Vector2 {
        float x;                // Vector x component
        float y;                // Vector y component
    } Vector2;

    // Vector3, 3 components
    typedef struct Vector3 {
        float x;                // Vector x component
        float y;                // Vector y component
        float z;                // Vector z component
    } Vector3;

    // Matrix, 4x4 components, column major, OpenGL style, right-handed
    typedef struct Matrix {
        float m0, m4, m8, m12;  // Matrix first row (4 components)
        float m1, m5, m9, m13;  // Matrix second row (4 components)
        float m2, m6, m10, m14; // Matrix third row (4 components)
        float m3, m7, m11, m15; // Matrix fourth row (4 components)
    } Matrix;

    // Camera type, defines a camera position/orientation in 3d space
    typedef struct Camera3D {
        Vector3 position;       // Camera position
        Vector3 target;         // Camera target it looks-at
        Vector3 up;             // Camera up vector (rotation over its axis)
        float fovy;             // Camera field-of-view apperture in Y (degrees) in perspective, used as near plane width in orthographic
        int projection;         // Camera projection type: CAMERA_PERSPECTIVE or CAMERA_ORTHOGRAPHIC
    } Camera3D;

    typedef Camera3D Camera;    // Camera type fallback, defaults to Camera3D

    // Camera projection
    typedef enum {
        CAMERA_PERSPECTIVE = 0, // Perspective projection
        CAMERA_ORTHOGRAPHIC     // Orthographic projection
    } CameraProjection;

    // Camera system modes
    typedef enum {
        CAMERA_CUSTOM = 0,      // Camera custom, controlled by user (UpdateCamera() does nothing)
        CAMERA_FREE,            // Camera free mode
        CAMERA_ORBITAL,         // Camera orbital, around target, zoom supported
        CAMERA_FIRST_PERSON,    // Camera first person
        CAMERA_THIRD_PERSON     // Camera third person
    } CameraMode;
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

RL_EXTERN_C_BEGIN

RLAPI Vector3 rlGetCameraForward(Camera *camera);
RLAPI Vector3 rlGetCameraUp(Camera *camera);
RLAPI Vector3 rlGetCameraRight(Camera *camera);

// Camera movement
RLAPI void rlCameraMoveForward(Camera *camera, float distance, bool moveInWorldPlane);
RLAPI void rlCameraMoveUp(Camera *camera, float distance);
RLAPI void rlCameraMoveRight(Camera *camera, float distance, bool moveInWorldPlane);
RLAPI void rlCameraMoveToTarget(Camera *camera, float delta);

// Camera rotation
RLAPI void rlCameraYaw(Camera *camera, float angle, bool rotateAroundTarget);
RLAPI void rlCameraPitch(Camera *camera, float angle, bool lockView, bool rotateAroundTarget, bool rotateUp);
RLAPI void rlCameraRoll(Camera *camera, float angle);

RLAPI Matrix rlGetCameraViewMatrix(Camera *camera);
RLAPI Matrix rlGetCameraProjectionMatrix(Camera* camera, float aspect);

RL_EXTERN_C_END

RL_NS_END

#endif // RCAMERA_H


/***********************************************************************************
*
*   CAMERA IMPLEMENTATION
*
************************************************************************************/

#if defined(RCAMERA_IMPLEMENTATION)

#include "raymath.h"        // Required for vector maths:
                            // Vector3Add()
                            // Vector3Subtract()
                            // Vector3Scale()
                            // Vector3Normalize()
                            // Vector3Distance()
                            // Vector3CrossProduct()
                            // Vector3RotateByAxisAngle()
                            // Vector3Angle()
                            // Vector3Negate()
                            // MatrixLookAt()
                            // MatrixPerspective()
                            // MatrixOrtho()
                            // MatrixIdentity()

// raylib required functionality:
                            // GetMouseDelta()
                            // GetMouseWheelMove()
                            // IsKeyDown()
                            // IsKeyPressed()
                            // GetFrameTime()

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define RL_CAMERA_MOVE_SPEED                               0.09f
#define RL_CAMERA_ROTATION_SPEED                           0.03f
#define RL_CAMERA_PAN_SPEED                                0.2f

// Camera mouse movement sensitivity
#define RL_CAMERA_MOUSE_MOVE_SENSITIVITY                   0.003f     // TODO: it should be independant of framerate
#define RL_CAMERA_MOUSE_SCROLL_SENSITIVITY                 1.5f

#define RL_CAMERA_ORBITAL_SPEED                            0.5f       // Radians per second


#define RL_CAMERA_FIRST_PERSON_STEP_TRIGONOMETRIC_DIVIDER  8.0f
#define RL_CAMERA_FIRST_PERSON_STEP_DIVIDER                30.0f
#define RL_CAMERA_FIRST_PERSON_WAVING_DIVIDER              200.0f

// PLAYER (used by camera)
#define RL_PLAYER_MOVEMENT_SENSITIVITY                     20.0f

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
//...

RL_NS_BEGIN

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
// Returns the cameras forward vector (normalized)
Vector3 rlGetCameraForward(Camera *camera)
{
    return Vector3Normalize(Vector3Subtract(camera->target, camera->position));
}

// Returns the cameras up vector (normalized)
// Note: The up vector might not be perpendicular to the forward vector
Vector3 rlGetCameraUp(Camera *camera)
{
    return Vector3Normalize(camera->up);
}

// Returns the cameras right vector (normalized)
Vector3 rlGetCameraRight(Camera *camera)
{
    Vector3 forward = rlGetCameraForward(camera);
    Vector3 up = rlGetCameraUp(camera);

    return Vector3CrossProduct(forward, up);
}

// Moves the camera in its forward direction
void rlCameraMoveForward(Camera *camera, float distance, bool moveInWorldPlane)
{
    Vector3 forward = rlGetCameraForward(camera);

    if (moveInWorldPlane)
    {
        // Project vector onto world plane
        forward.y = 0;
        forward = Vector3Normalize(forward);
    }

    // Scale by distance
    forward = Vector3Scale(forward, distance);

    // Move position and target
    camera->position = Vector3Add(camera->position, forward);
    camera->target = Vector3Add(camera->target, forward);
}

// Moves the camera in its up direction
void rlCameraMoveUp(Camera *camera, float distance)
{
    Vector3 up = rlGetCameraUp(camera);

    // Scale by distance
    up = Vector3Scale(up, distance);

    // Move position and target
    camera->position = Vector3Add(camera->position, up);
    camera->target = Vector3Add(camera->target, up);
}

// Moves the camera target in its current right direction
void rlCameraMoveRight(Camera *camera, float distance, bool moveInWorldPlane)
{
    Vector3 right = rlGetCameraRight(camera);

    if (moveInWorldPlane)
    {
        // Project vector onto world plane
        right.y = 0;
        right = Vector3Normalize(right);
    }

    // Scale by distance
    right = Vector3Scale(right, distance);

    // Move position and target
    camera->position = Vector3Add(camera->position, right);
    camera->target = Vector3Add(camera->target, right);
}

// Moves the camera position closer/farther to/from the camera target
void rlCameraMoveToTarget(Camera *camera, float delta)
{
    float distance = Vector3Distance(camera->position, camera->target);

    // Apply delta
    distance += delta;

    // Distance must be greater than 0
    if (distance <= 0) distance = 0.001f;

    // Set new distance by moving the position along the forward vector
    Vector3 forward = rlGetCameraForward(camera);
    camera->position = Vector3Add(camera->target, Vector3Scale(forward, -distance));
}

// Rotates the camera around its up vector
// Yaw is "looking left and right"
// If rotateAroundTarget is false, the camera rotates around its position
// Note: angle must be provided in radians
void rlCameraYaw(Camera *camera, float angle, bool rotateAroundTarget)
{
    // Rotation axis
    Vector3 up = rlGetCameraUp(camera);

    // View vector
    Vector3 targetPosition = Vector3Subtract(camera->target, camera->position);

    // Rotate view vector around up axis
    targetPosition = Vector3RotateByAxisAngle(targetPosition, up, angle);

    if (rotateAroundTarget)
    {
        // Move position relative to target
        camera->position = Vector3Subtract(camera->target, targetPosition);
    }
    else // rotate around camera.position
    {
        // Move target relative to position
        camera->target = Vector3Add(camera->position, targetPosition);
    }
}

// Rotates the camera around its right vector, pitch is "looking up and down"
//  - lockView prevents camera overrotation (aka "somersaults")
//  - rotateAroundTarget defines if rotation is around target or around its position
//  - rotateUp rotates the up direction as well (typically only usefull in CAMERA_FREE)
// NOTE: angle must be provided in radians
void rlCameraPitch(Camera *camera, float angle, bool lockView, bool rotateAroundTarget, bool rotateUp)
{
    // Up direction
    Vector3 up = rlGetCameraUp(camera);

    // View vector
    Vector3 targetPosition = Vector3Subtract(camera->target, camera->position);

    if (lockView)
    {
        // In these camera modes we clamp the Pitch angle
        // to allow only viewing straight up or down.

        // Clamp view up
        float maxAngleUp = Vector3Angle(up, targetPosition);
        maxAngleUp -= 0.001f; // avoid numerical errors
        if (angle > maxAngleUp) angle = maxAngleUp;

        // Clamp view down
        float maxAngleDown = Vector3Angle(Vector3Negate(up), targetPosition);
        maxAngleDown *= -1.0f; // downwards angle is negative
        maxAngleDown += 0.001f; // avoid numerical errors
        if (angle < maxAngleDown) angle = maxAngleDown;
    }

    // Rotation axis
    Vector3 right = rlGetCameraRight(camera);

    // Rotate view vector around right axis
    targetPosition = Vector3RotateByAxisAngle(targetPosition, right, angle);

    if (rotateAroundTarget)
    {
        // Move position relative to target
        camera->position = Vector3Subtract(camera->target, targetPosition);
    }
    else // rotate around camera.position
    {
        // Move target relative to position
        camera->target = Vector3Add(camera->position, targetPosition);
    }

    if (rotateUp)
    {
        // Rotate up direction around right axis
        camera->up = Vector3RotateByAxisAngle(camera->up, right, angle);
    }
}

// Rotates the camera around its forward vector
// Roll is "turning your head sideways to the left or right"
// Note: angle must be provided in radians
void rlCameraRoll(Camera *camera, float angle)
{
    // Rotation axis
    Vector3 forward = rlGetCameraForward(camera);

    // Rotate up direction around forward axis
    camera->up = Vector3RotateByAxisAngle(camera->up, forward, angle);
}

// Returns the camera view matrix
Matrix rlGetCameraViewMatrix(Camera *camera)
{
    return MatrixLookAt(camera->position, camera->target, camera->up);
}

// Returns the camera projection matrix
Matrix rlGetCameraProjectionMatrix(Camera *camera, float aspect)
{
    if (camera->projection == CAMERA_PERSPECTIVE)
    {
        return MatrixPerspective(camera->fovy*RL_DEG2RAD, aspect, RL_CAMERA_CULL_DISTANCE_NEAR, RL_CAMERA_CULL_DISTANCE_FAR);
    }
    else if (camera->projection == CAMERA_ORTHOGRAPHIC)
    {
        double top = camera->fovy/2.0;
        double right = top*aspect;

        return MatrixOrtho(-right, right, -top, top, RL_CAMERA_CULL_DISTANCE_NEAR, RL_CAMERA_CULL_DISTANCE_FAR);
    }

    return MatrixIdentity();
}

#if !defined(RCAMERA_STANDALONE)
// Update camera position for selected mode
// Camera mode: CAMERA_FREE, CAMERA_FIRST_PERSON, CAMERA_THIRD_PERSON, CAMERA_ORBITAL or CUSTOM
void UpdateCamera(Camera *camera, int mode)
{
    Vector2 mousePositionDelta = GetMouseDelta();

    bool moveInWorldPlane = ((mode == CAMERA_FIRST_PERSON) || (mode == CAMERA_THIRD_PERSON));
    bool rotateAroundTarget = ((mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL));
    bool lockView = ((mode == CAMERA_FIRST_PERSON) || (mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL));
    bool rotateUp = false;

    if (mode == CAMERA_ORBITAL)
    {
        // Orbital can just orbit
        Matrix rotation = MatrixRotate(rlGetCameraUp(camera), RL_CAMERA_ORBITAL_SPEED*GetFrameTime());
        Vector3 view = Vector3Subtract(camera->position, camera->target);
        view = Vector3Transform(view, rotation);
        camera->position = Vector3Add(camera->target, view);
    }
    else
    {
        // Camera rotation
        if (IsKeyDown(KEY_DOWN)) rlCameraPitch(camera, -RL_CAMERA_ROTATION_SPEED, lockView, rotateAroundTarget, rotateUp);
        if (IsKeyDown(KEY_UP)) rlCameraPitch(camera, RL_CAMERA_ROTATION_SPEED, lockView, rotateAroundTarget, rotateUp);
        if (IsKeyDown(KEY_RIGHT)) rlCameraYaw(camera, -RL_CAMERA_ROTATION_SPEED, rotateAroundTarget);
        if (IsKeyDown(KEY_LEFT)) rlCameraYaw(camera, RL_CAMERA_ROTATION_SPEED, rotateAroundTarget);
        if (IsKeyDown(KEY_Q)) rlCameraRoll(camera, -RL_CAMERA_ROTATION_SPEED);
        if (IsKeyDown(KEY_E)) rlCameraRoll(camera, RL_CAMERA_ROTATION_SPEED);

        // Camera movement
        if (!IsGamepadAvailable(0))
        {
            // Camera pan (for CAMERA_FREE)
            if ((mode == CAMERA_FREE) && (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)))
            {
                const Vector2 mouseDelta = GetMouseDelta();
                if (mouseDelta.x > 0.0f) rlCameraMoveRight(camera, RL_CAMERA_PAN_SPEED, moveInWorldPlane);
                if (mouseDelta.x < 0.0f) rlCameraMoveRight(camera, -RL_CAMERA_PAN_SPEED, moveInWorldPlane);
                if (mouseDelta.y > 0.0f) rlCameraMoveUp(camera, -RL_CAMERA_PAN_SPEED);
                if (mouseDelta.y < 0.0f) rlCameraMoveUp(camera, RL_CAMERA_PAN_SPEED);
            }
            else
            {
                // Mouse support
                rlCameraYaw(camera, -mousePositionDelta.x*RL_CAMERA_MOUSE_MOVE_SENSITIVITY, rotateAroundTarget);
                rlCameraPitch(camera, -mousePositionDelta.y*RL_CAMERA_MOUSE_MOVE_SENSITIVITY, lockView, rotateAroundTarget, rotateUp);
            }

            // Keyboard support
            if (IsKeyDown(KEY_W)) rlCameraMoveForward(camera, RL_CAMERA_MOVE_SPEED, moveInWorldPlane);
            if (IsKeyDown(KEY_A)) rlCameraMoveRight(camera, -RL_CAMERA_MOVE_SPEED, moveInWorldPlane);
            if (IsKeyDown(KEY_S)) rlCameraMoveForward(camera, -RL_CAMERA_MOVE_SPEED, moveInWorldPlane);
            if (IsKeyDown(KEY_D)) rlCameraMoveRight(camera, RL_CAMERA_MOVE_SPEED, moveInWorldPlane);
        }
        else
        {
            // Gamepad controller support
            rlCameraYaw(camera, -(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X) * 2)*RL_CAMERA_MOUSE_MOVE_SENSITIVITY, rotateAroundTarget);
            rlCameraPitch(camera, -(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y) * 2)*RL_CAMERA_MOUSE_MOVE_SENSITIVITY, lockView, rotateAroundTarget, rotateUp);

            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) <= -0.25f) rlCameraMoveForward(camera, RL_CAMERA_MOVE_SPEED, moveInWorldPlane);
            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) <= -0.25f) rlCameraMoveRight(camera, -RL_CAMERA_MOVE_SPEED, moveInWorldPlane);
            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) >= 0.25f) rlCameraMoveForward(camera, -RL_CAMERA_MOVE_SPEED, moveInWorldPlane);
            if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) >= 0.25f) rlCameraMoveRight(camera, RL_CAMERA_MOVE_SPEED, moveInWorldPlane);
        }

        if (mode == CAMERA_FREE)
        {
            if (IsKeyDown(KEY_SPACE)) rlCameraMoveUp(camera, RL_CAMERA_MOVE_SPEED);
            if (IsKeyDown(KEY_LEFT_CONTROL)) rlCameraMoveUp(camera, -RL_CAMERA_MOVE_SPEED);
        }
    }

    if ((mode == CAMERA_THIRD_PERSON) || (mode == CAMERA_ORBITAL) || (mode == CAMERA_FREE))
    {
        // Zoom target distance
        rlCameraMoveToTarget(camera, -GetMouseWheelMove());
        if (IsKeyPressed(KEY_KP_SUBTRACT)) rlCameraMoveToTarget(camera, 2.0f);
        if (IsKeyPressed(KEY_KP_ADD)) rlCameraMoveToTarget(camera, -2.0f);
    }
}
#endif // !RCAMERA_STANDALONE

// Update camera movement, movement/rotation values should be provided by user
void UpdateCameraPro(Camera *camera, Vector3 movement, Vector3 rotation, float zoom)
{
    // Required values
    // movement.x - Move forward/backward
    // movement.y - Move right/left
    // movement.z - Move up/down
    // rotation.x - yaw
    // rotation.y - pitch
    // rotation.z - roll
    // zoom - Move towards target

    bool lockView = true;
    bool rotateAroundTarget = false;
    bool rotateUp = false;
    bool moveInWorldPlane = true;

    // Camera rotation
    rlCameraPitch(camera, -rotation.y*RL_DEG2RAD, lockView, rotateAroundTarget, rotateUp);
    rlCameraYaw(camera, -rotation.x*RL_DEG2RAD, rotateAroundTarget);
    rlCameraRoll(camera, rotation.z*RL_DEG2RAD);

    // Camera movement
    rlCameraMoveForward(camera, movement.x, moveInWorldPlane);
    rlCameraMoveRight(camera, movement.y, moveInWorldPlane);
    rlCameraMoveUp(camera, movement.z);

    // Zoom target distance
    rlCameraMoveToTarget(camera, zoom);
}

RL_NS_END

#endif // RCAMERA_IMPLEMENTATION
