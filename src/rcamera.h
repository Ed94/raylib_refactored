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

    // rlMatrix, 4x4 components, column major, OpenGL style, right-handed
    typedef struct rlMatrix {
        float m0, m4, m8, m12;  // rlMatrix first row (4 components)
        float m1, m5, m9, m13;  // rlMatrix second row (4 components)
        float m2, m6, m10, m14; // rlMatrix third row (4 components)
        float m3, m7, m11, m15; // rlMatrix fourth row (4 components)
    } rlMatrix;

    // rlCamera type, defines a camera position/orientation in 3d space
    typedef struct rlCamera3D {
        rlVector3 position;       // rlCamera position
        rlVector3 target;         // rlCamera target it looks-at
        rlVector3 up;             // rlCamera up vector (rotation over its axis)
        float fovy;             // rlCamera field-of-view apperture in Y (degrees) in perspective, used as near plane width in orthographic
        int projection;         // rlCamera projection type: RL_CAMERA_PERSPECTIVE or RL_CAMERA_ORTHOGRAPHIC
    } rlCamera3D;

    typedef rlCamera3D rlCamera;    // rlCamera type fallback, defaults to rlCamera3D

    // rlCamera projection
    typedef enum {
        RL_CAMERA_PERSPECTIVE = 0, // Perspective projection
        RL_CAMERA_ORTHOGRAPHIC     // Orthographic projection
    } rlCameraProjection;

    // rlCamera system modes
    typedef enum {
        RL_CAMERA_CUSTOM = 0,      // rlCamera custom, controlled by user (rlUpdateCamera() does nothing)
        RL_CAMERA_FREE,            // rlCamera free mode
        RL_CAMERA_ORBITAL,         // rlCamera orbital, around target, zoom supported
        RL_CAMERA_FIRST_PERSON,    // rlCamera first person
        RL_CAMERA_THIRD_PERSON     // rlCamera third person
    } rlCameraMode;
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
//...

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------

RL_EXTERN_C_BEGIN

RLAPI rlVector3 rlGetCameraForward(rlCamera *camera);
RLAPI rlVector3 rlGetCameraUp(rlCamera *camera);
RLAPI rlVector3 rlGetCameraRight(rlCamera *camera);

// rlCamera movement
RLAPI void rlCameraMoveForward(rlCamera *camera, float distance, bool moveInWorldPlane);
RLAPI void rlCameraMoveUp(rlCamera *camera, float distance);
RLAPI void rlCameraMoveRight(rlCamera *camera, float distance, bool moveInWorldPlane);
RLAPI void rlCameraMoveToTarget(rlCamera *camera, float delta);

// rlCamera rotation
RLAPI void rlCameraYaw(rlCamera *camera, float angle, bool rotateAroundTarget);
RLAPI void rlCameraPitch(rlCamera *camera, float angle, bool lockView, bool rotateAroundTarget, bool rotateUp);
RLAPI void rlCameraRoll(rlCamera *camera, float angle);

RLAPI rlMatrix rlGetCameraViewMatrix(rlCamera *camera);
RLAPI rlMatrix rlGetCameraProjectionMatrix(rlCamera* camera, float aspect);

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
                            // rlVector3Add()
                            // rlVector3Subtract()
                            // rlVector3Scale()
                            // rlVector3Normalize()
                            // rlVector3Distance()
                            // rlVector3CrossProduct()
                            // rlVector3RotateByAxisAngle()
                            // rlVector3Angle()
                            // rlVector3Negate()
                            // rlMatrixLookAt()
                            // rlMatrixPerspective()
                            // rlMatrixOrtho()
                            // rlMatrixIdentity()

// raylib required functionality:
                            // rlGetMouseDelta()
                            // rlGetMouseWheelMove()
                            // rlIsKeyDown()
                            // rlIsKeyPressed()
                            // rlGetFrameTime()

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define RL_CAMERA_MOVE_SPEED                               0.09f
#define RL_CAMERA_ROTATION_SPEED                           0.03f
#define RL_CAMERA_PAN_SPEED                                0.2f

// rlCamera mouse movement sensitivity
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
rlVector3 rlGetCameraForward(rlCamera *camera)
{
    return rlVector3Normalize(rlVector3Subtract(camera->target, camera->position));
}

// Returns the cameras up vector (normalized)
// Note: The up vector might not be perpendicular to the forward vector
rlVector3 rlGetCameraUp(rlCamera *camera)
{
    return rlVector3Normalize(camera->up);
}

// Returns the cameras right vector (normalized)
rlVector3 rlGetCameraRight(rlCamera *camera)
{
    rlVector3 forward = rlGetCameraForward(camera);
    rlVector3 up = rlGetCameraUp(camera);

    return rlVector3CrossProduct(forward, up);
}

// Moves the camera in its forward direction
void rlCameraMoveForward(rlCamera *camera, float distance, bool moveInWorldPlane)
{
    rlVector3 forward = rlGetCameraForward(camera);

    if (moveInWorldPlane)
    {
        // Project vector onto world plane
        forward.y = 0;
        forward = rlVector3Normalize(forward);
    }

    // Scale by distance
    forward = rlVector3Scale(forward, distance);

    // Move position and target
    camera->position = rlVector3Add(camera->position, forward);
    camera->target = rlVector3Add(camera->target, forward);
}

// Moves the camera in its up direction
void rlCameraMoveUp(rlCamera *camera, float distance)
{
    rlVector3 up = rlGetCameraUp(camera);

    // Scale by distance
    up = rlVector3Scale(up, distance);

    // Move position and target
    camera->position = rlVector3Add(camera->position, up);
    camera->target = rlVector3Add(camera->target, up);
}

// Moves the camera target in its current right direction
void rlCameraMoveRight(rlCamera *camera, float distance, bool moveInWorldPlane)
{
    rlVector3 right = rlGetCameraRight(camera);

    if (moveInWorldPlane)
    {
        // Project vector onto world plane
        right.y = 0;
        right = rlVector3Normalize(right);
    }

    // Scale by distance
    right = rlVector3Scale(right, distance);

    // Move position and target
    camera->position = rlVector3Add(camera->position, right);
    camera->target = rlVector3Add(camera->target, right);
}

// Moves the camera position closer/farther to/from the camera target
void rlCameraMoveToTarget(rlCamera *camera, float delta)
{
    float distance = rlVector3Distance(camera->position, camera->target);

    // Apply delta
    distance += delta;

    // Distance must be greater than 0
    if (distance <= 0) distance = 0.001f;

    // Set new distance by moving the position along the forward vector
    rlVector3 forward = rlGetCameraForward(camera);
    camera->position = rlVector3Add(camera->target, rlVector3Scale(forward, -distance));
}

// Rotates the camera around its up vector
// Yaw is "looking left and right"
// If rotateAroundTarget is false, the camera rotates around its position
// Note: angle must be provided in radians
void rlCameraYaw(rlCamera *camera, float angle, bool rotateAroundTarget)
{
    // Rotation axis
    rlVector3 up = rlGetCameraUp(camera);

    // View vector
    rlVector3 targetPosition = rlVector3Subtract(camera->target, camera->position);

    // Rotate view vector around up axis
    targetPosition = rlVector3RotateByAxisAngle(targetPosition, up, angle);

    if (rotateAroundTarget)
    {
        // Move position relative to target
        camera->position = rlVector3Subtract(camera->target, targetPosition);
    }
    else // rotate around camera.position
    {
        // Move target relative to position
        camera->target = rlVector3Add(camera->position, targetPosition);
    }
}

// Rotates the camera around its right vector, pitch is "looking up and down"
//  - lockView prevents camera overrotation (aka "somersaults")
//  - rotateAroundTarget defines if rotation is around target or around its position
//  - rotateUp rotates the up direction as well (typically only usefull in RL_CAMERA_FREE)
// NOTE: angle must be provided in radians
void rlCameraPitch(rlCamera *camera, float angle, bool lockView, bool rotateAroundTarget, bool rotateUp)
{
    // Up direction
    rlVector3 up = rlGetCameraUp(camera);

    // View vector
    rlVector3 targetPosition = rlVector3Subtract(camera->target, camera->position);

    if (lockView)
    {
        // In these camera modes we clamp the Pitch angle
        // to allow only viewing straight up or down.

        // rlClamp view up
        float maxAngleUp = rlVector3Angle(up, targetPosition);
        maxAngleUp -= 0.001f; // avoid numerical errors
        if (angle > maxAngleUp) angle = maxAngleUp;

        // rlClamp view down
        float maxAngleDown = rlVector3Angle(rlVector3Negate(up), targetPosition);
        maxAngleDown *= -1.0f; // downwards angle is negative
        maxAngleDown += 0.001f; // avoid numerical errors
        if (angle < maxAngleDown) angle = maxAngleDown;
    }

    // Rotation axis
    rlVector3 right = rlGetCameraRight(camera);

    // Rotate view vector around right axis
    targetPosition = rlVector3RotateByAxisAngle(targetPosition, right, angle);

    if (rotateAroundTarget)
    {
        // Move position relative to target
        camera->position = rlVector3Subtract(camera->target, targetPosition);
    }
    else // rotate around camera.position
    {
        // Move target relative to position
        camera->target = rlVector3Add(camera->position, targetPosition);
    }

    if (rotateUp)
    {
        // Rotate up direction around right axis
        camera->up = rlVector3RotateByAxisAngle(camera->up, right, angle);
    }
}

// Rotates the camera around its forward vector
// Roll is "turning your head sideways to the left or right"
// Note: angle must be provided in radians
void rlCameraRoll(rlCamera *camera, float angle)
{
    // Rotation axis
    rlVector3 forward = rlGetCameraForward(camera);

    // Rotate up direction around forward axis
    camera->up = rlVector3RotateByAxisAngle(camera->up, forward, angle);
}

// Returns the camera view matrix
rlMatrix rlGetCameraViewMatrix(rlCamera *camera)
{
    return rlMatrixLookAt(camera->position, camera->target, camera->up);
}

// Returns the camera projection matrix
rlMatrix rlGetCameraProjectionMatrix(rlCamera *camera, float aspect)
{
    if (camera->projection == RL_CAMERA_PERSPECTIVE)
    {
        return rlMatrixPerspective(camera->fovy*RL_DEG2RAD, aspect, RL_CAMERA_CULL_DISTANCE_NEAR, RL_CAMERA_CULL_DISTANCE_FAR);
    }
    else if (camera->projection == RL_CAMERA_ORTHOGRAPHIC)
    {
        double top = camera->fovy/2.0;
        double right = top*aspect;

        return rlMatrixOrtho(-right, right, -top, top, RL_CAMERA_CULL_DISTANCE_NEAR, RL_CAMERA_CULL_DISTANCE_FAR);
    }

    return rlMatrixIdentity();
}

#if !defined(RCAMERA_STANDALONE)
// Update camera position for selected mode
// rlCamera mode: RL_CAMERA_FREE, RL_CAMERA_FIRST_PERSON, RL_CAMERA_THIRD_PERSON, RL_CAMERA_ORBITAL or CUSTOM
void rlUpdateCamera(rlCamera *camera, int mode)
{
    rlVector2 mousePositionDelta = rlGetMouseDelta();

    bool moveInWorldPlane = ((mode == RL_CAMERA_FIRST_PERSON) || (mode == RL_CAMERA_THIRD_PERSON));
    bool rotateAroundTarget = ((mode == RL_CAMERA_THIRD_PERSON) || (mode == RL_CAMERA_ORBITAL));
    bool lockView = ((mode == RL_CAMERA_FIRST_PERSON) || (mode == RL_CAMERA_THIRD_PERSON) || (mode == RL_CAMERA_ORBITAL));
    bool rotateUp = false;

    if (mode == RL_CAMERA_ORBITAL)
    {
        // Orbital can just orbit
        rlMatrix rotation = rlMatrixRotate(rlGetCameraUp(camera), RL_CAMERA_ORBITAL_SPEED*rlGetFrameTime());
        rlVector3 view = rlVector3Subtract(camera->position, camera->target);
        view = rlVector3Transform(view, rotation);
        camera->position = rlVector3Add(camera->target, view);
    }
    else
    {
        // rlCamera rotation
        if (rlIsKeyDown(RL_KEY_DOWN)) rlCameraPitch(camera, -RL_CAMERA_ROTATION_SPEED, lockView, rotateAroundTarget, rotateUp);
        if (rlIsKeyDown(RL_KEY_UP)) rlCameraPitch(camera, RL_CAMERA_ROTATION_SPEED, lockView, rotateAroundTarget, rotateUp);
        if (rlIsKeyDown(RL_KEY_RIGHT)) rlCameraYaw(camera, -RL_CAMERA_ROTATION_SPEED, rotateAroundTarget);
        if (rlIsKeyDown(RL_KEY_LEFT)) rlCameraYaw(camera, RL_CAMERA_ROTATION_SPEED, rotateAroundTarget);
        if (rlIsKeyDown(RL_KEY_Q)) rlCameraRoll(camera, -RL_CAMERA_ROTATION_SPEED);
        if (rlIsKeyDown(RL_KEY_E)) rlCameraRoll(camera, RL_CAMERA_ROTATION_SPEED);

        // rlCamera movement
        if (!rlIsGamepadAvailable(0))
        {
            // rlCamera pan (for RL_CAMERA_FREE)
            if ((mode == RL_CAMERA_FREE) && (rlIsMouseButtonDown(RL_MOUSE_BUTTON_MIDDLE)))
            {
                const rlVector2 mouseDelta = rlGetMouseDelta();
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
            if (rlIsKeyDown(RL_KEY_W)) rlCameraMoveForward(camera, RL_CAMERA_MOVE_SPEED, moveInWorldPlane);
            if (rlIsKeyDown(RL_KEY_A)) rlCameraMoveRight(camera, -RL_CAMERA_MOVE_SPEED, moveInWorldPlane);
            if (rlIsKeyDown(RL_KEY_S)) rlCameraMoveForward(camera, -RL_CAMERA_MOVE_SPEED, moveInWorldPlane);
            if (rlIsKeyDown(RL_KEY_D)) rlCameraMoveRight(camera, RL_CAMERA_MOVE_SPEED, moveInWorldPlane);
        }
        else
        {
            // Gamepad controller support
            rlCameraYaw(camera, -(rlGetGamepadAxisMovement(0, RL_GAMEPAD_AXIS_RIGHT_X) * 2)*RL_CAMERA_MOUSE_MOVE_SENSITIVITY, rotateAroundTarget);
            rlCameraPitch(camera, -(rlGetGamepadAxisMovement(0, RL_GAMEPAD_AXIS_RIGHT_Y) * 2)*RL_CAMERA_MOUSE_MOVE_SENSITIVITY, lockView, rotateAroundTarget, rotateUp);

            if (rlGetGamepadAxisMovement(0, RL_GAMEPAD_AXIS_LEFT_Y) <= -0.25f) rlCameraMoveForward(camera, RL_CAMERA_MOVE_SPEED, moveInWorldPlane);
            if (rlGetGamepadAxisMovement(0, RL_GAMEPAD_AXIS_LEFT_X) <= -0.25f) rlCameraMoveRight(camera, -RL_CAMERA_MOVE_SPEED, moveInWorldPlane);
            if (rlGetGamepadAxisMovement(0, RL_GAMEPAD_AXIS_LEFT_Y) >= 0.25f) rlCameraMoveForward(camera, -RL_CAMERA_MOVE_SPEED, moveInWorldPlane);
            if (rlGetGamepadAxisMovement(0, RL_GAMEPAD_AXIS_LEFT_X) >= 0.25f) rlCameraMoveRight(camera, RL_CAMERA_MOVE_SPEED, moveInWorldPlane);
        }

        if (mode == RL_CAMERA_FREE)
        {
            if (rlIsKeyDown(RL_KEY_SPACE)) rlCameraMoveUp(camera, RL_CAMERA_MOVE_SPEED);
            if (rlIsKeyDown(RL_KEY_LEFT_CONTROL)) rlCameraMoveUp(camera, -RL_CAMERA_MOVE_SPEED);
        }
    }

    if ((mode == RL_CAMERA_THIRD_PERSON) || (mode == RL_CAMERA_ORBITAL) || (mode == RL_CAMERA_FREE))
    {
        // Zoom target distance
        rlCameraMoveToTarget(camera, -rlGetMouseWheelMove());
        if (rlIsKeyPressed(RL_KEY_KP_SUBTRACT)) rlCameraMoveToTarget(camera, 2.0f);
        if (rlIsKeyPressed(RL_KEY_KP_ADD)) rlCameraMoveToTarget(camera, -2.0f);
    }
}
#endif // !RCAMERA_STANDALONE

// Update camera movement, movement/rotation values should be provided by user
void rlUpdateCameraPro(rlCamera *camera, rlVector3 movement, rlVector3 rotation, float zoom)
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

    // rlCamera rotation
    rlCameraPitch(camera, -rotation.y*RL_DEG2RAD, lockView, rotateAroundTarget, rotateUp);
    rlCameraYaw(camera, -rotation.x*RL_DEG2RAD, rotateAroundTarget);
    rlCameraRoll(camera, rotation.z*RL_DEG2RAD);

    // rlCamera movement
    rlCameraMoveForward(camera, movement.x, moveInWorldPlane);
    rlCameraMoveRight(camera, movement.y, moveInWorldPlane);
    rlCameraMoveUp(camera, movement.z);

    // Zoom target distance
    rlCameraMoveToTarget(camera, zoom);
}

RL_NS_END

#endif // RCAMERA_IMPLEMENTATION
