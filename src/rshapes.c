/**********************************************************************************************
*
*   rshapes - Basic functions to draw 2d shapes and check collisions
*
*   ADDITIONAL NOTES:
*       Shapes can be draw using 3 types of primitives: LINES, TRIANGLES and QUADS.
*       Some functions implement two drawing options: TRIANGLES and QUADS, by default TRIANGLES
*       are used but QUADS implementation can be selected with RL_SUPPORT_QUADS_DRAW_MODE define
*
*       Some functions define texture coordinates (rlglTexCoord2f()) for the shapes and use a
*       user-provided texture with rlSetShapesTexture(), the pourpouse of this implementation
*       is allowing to reduce draw calls when combined with a texture-atlas.
*
*       By default, raylib sets the default texture and rectangle at rlInitWindow()[rcore] to one
*       white character of default font [rtext], this way, raylib text and shapes can be draw with
*       a single draw call and it also allows users to configure it the same way with their own fonts.
*
*   CONFIGURATION:
*       #define RL_SUPPORT_MODULE_RSHAPES
*           rshapes module is included in the build
*
*       #define RL_SUPPORT_QUADS_DRAW_MODE
*           Use QUADS instead of TRIANGLES for drawing when possible. Lines-based shapes still use LINES
*
*
*   LICENSE: zlib/libpng
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

#include "raylib.h"     // Declares module functions

// Check if config flags have been externally provided on compilation line
#if !defined(EXTERNAL_CONFIG_FLAGS)
    #include "config.h"         // Defines module configuration flags
#endif

#if defined(RL_SUPPORT_MODULE_RSHAPES)

#include "rlgl.h"       // OpenGL abstraction layer to OpenGL 1.1, 2.1, 3.3+ or ES2

#include <math.h>       // Required for: sinf(), asinf(), cosf(), acosf(), sqrtf(), fabsf()
#include <float.h>      // Required for: FLT_EPSILON
#include <stdlib.h>     // Required for: RL_FREE

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Error rate to calculate how many segments we need to draw a smooth circle,
// taken from https://stackoverflow.com/a/2244088
#ifndef SMOOTH_CIRCLE_ERROR_RATE
    #define SMOOTH_CIRCLE_ERROR_RATE    0.5f      // Circle error rate
#endif
#ifndef SPLINE_SEGMENT_DIVISIONS
    #define SPLINE_SEGMENT_DIVISIONS      24      // Spline segment divisions
#endif

RL_NS_BEGIN

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Not here...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
rlTexture2D texShapes = { 1, 1, 1, 1, 7 };                // rlTexture used on shapes drawing (white pixel loaded by rlgl)
rlRectangle texShapesRec = { 0.0f, 0.0f, 1.0f, 1.0f };    // rlTexture source rectangle used on shapes drawing

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static float EaseCubicInOut(float t, float b, float c, float d);    // Cubic easing

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Set texture and rectangle to be used on shapes drawing
// NOTE: It can be useful when using basic shapes and one single font,
// defining a font char white rectangle would allow drawing everything in a single draw call
void rlSetShapesTexture(rlTexture2D texture, rlRectangle source)
{
    // Reset texture to default pixel if required
    // WARNING: Shapes texture should be probably better validated,
    // it can break the rendering of all shapes if misused
    if ((texture.id == 0) || (source.width == 0) || (source.height == 0))
    {
        texShapes = CAST(rlTexture2D){ 1, 1, 1, 1, 7 };
        texShapesRec = CAST(rlRectangle){ 0.0f, 0.0f, 1.0f, 1.0f };
    }
    else
    {
        texShapes = texture;
        texShapesRec = source;
    }
}

// Draw a pixel
void rlDrawPixel(int posX, int posY, rlColor color)
{
  rlDrawPixelV(CAST(rlVector2){ (float)posX, (float)posY }, color);
}

// Draw a pixel (Vector version)
void rlDrawPixelV(rlVector2 position, rlColor color)
{
#if defined(RL_SUPPORT_QUADS_DRAW_MODE)
    rlglSetTexture(texShapes.id);

    rlglBegin(RL_QUADS);

        rlglNormal3f(0.0f, 0.0f, 1.0f);
        rlglColor4ub(color.r, color.g, color.b, color.a);

        rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(position.x, position.y);

        rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(position.x, position.y + 1);

        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(position.x + 1, position.y + 1);

        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(position.x + 1, position.y);

    rlglEnd();

    rlglSetTexture(0);
#else
    rlglBegin(RL_TRIANGLES);

        rlglColor4ub(color.r, color.g, color.b, color.a);

        rlglVertex2f(position.x, position.y);
        rlglVertex2f(position.x, position.y + 1);
        rlglVertex2f(position.x + 1, position.y);

        rlglVertex2f(position.x + 1, position.y);
        rlglVertex2f(position.x, position.y + 1);
        rlglVertex2f(position.x + 1, position.y + 1);

    rlglEnd();
#endif
}

// Draw a line (using gl lines)
void rlDrawLine(int startPosX, int startPosY, int endPosX, int endPosY, rlColor color)
{
    rlglBegin(RL_LINES);
        rlglColor4ub(color.r, color.g, color.b, color.a);
        rlglVertex2f((float)startPosX, (float)startPosY);
        rlglVertex2f((float)endPosX, (float)endPosY);
    rlglEnd();
}

// Draw a line (using gl lines)
void rlDrawLineV(rlVector2 startPos, rlVector2 endPos, rlColor color)
{
    rlglBegin(RL_LINES);
        rlglColor4ub(color.r, color.g, color.b, color.a);
        rlglVertex2f(startPos.x, startPos.y);
        rlglVertex2f(endPos.x, endPos.y);
    rlglEnd();
}

// Draw lines sequuence (using gl lines)
void rlDrawLineStrip(rlVector2 *points, int pointCount, rlColor color)
{
    if (pointCount >= 2)
    {
        rlglBegin(RL_LINES);
            rlglColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 0; i < pointCount - 1; i++)
            {
                rlglVertex2f(points[i].x, points[i].y);
                rlglVertex2f(points[i + 1].x, points[i + 1].y);
            }
        rlglEnd();
    }
}

// Draw line using cubic-bezier spline, in-out interpolation, no control points
void rlDrawLineBezier(rlVector2 startPos, rlVector2 endPos, float thick, rlColor color)
{
    rlVector2 previous = startPos;
    rlVector2 current = { 0 };

    rlVector2 points[2*SPLINE_SEGMENT_DIVISIONS + 2] = { 0 };

    for (int i = 1; i <= SPLINE_SEGMENT_DIVISIONS; i++)
    {
        // Cubic easing in-out
        // NOTE: Easing is calculated only for y position value
        current.y = EaseCubicInOut((float)i, startPos.y, endPos.y - startPos.y, (float)SPLINE_SEGMENT_DIVISIONS);
        current.x = previous.x + (endPos.x - startPos.x)/(float)SPLINE_SEGMENT_DIVISIONS;

        float dy = current.y - previous.y;
        float dx = current.x - previous.x;
        float size = 0.5f*thick/sqrtf(dx*dx+dy*dy);

        if (i == 1)
        {
            points[0].x = previous.x + dy*size;
            points[0].y = previous.y - dx*size;
            points[1].x = previous.x - dy*size;
            points[1].y = previous.y + dx*size;
        }

        points[2*i + 1].x = current.x - dy*size;
        points[2*i + 1].y = current.y + dx*size;
        points[2*i].x = current.x + dy*size;
        points[2*i].y = current.y - dx*size;

        previous = current;
    }

    rlDrawTriangleStrip(points, 2*SPLINE_SEGMENT_DIVISIONS + 2, color);
}

// Draw a line defining thickness
void rlDrawLineEx(rlVector2 startPos, rlVector2 endPos, float thick, rlColor color)
{
    rlVector2 delta = { endPos.x - startPos.x, endPos.y - startPos.y };
    float length = sqrtf(delta.x*delta.x + delta.y*delta.y);

    if ((length > 0) && (thick > 0))
    {
        float scale = thick/(2*length);

        rlVector2 radius = { -scale*delta.y, scale*delta.x };
        rlVector2 strip[4] = {
            { startPos.x - radius.x, startPos.y - radius.y },
            { startPos.x + radius.x, startPos.y + radius.y },
            { endPos.x - radius.x, endPos.y - radius.y },
            { endPos.x + radius.x, endPos.y + radius.y }
        };

        rlDrawTriangleStrip(strip, 4, color);
    }
}

// Draw a color-filled circle
void rlDrawCircle(int centerX, int centerY, float radius, rlColor color)
{
    rlDrawCircleV(CAST(rlVector2){ (float)centerX, (float)centerY }, radius, color);
}

// Draw a color-filled circle (Vector version)
// NOTE: On OpenGL 3.3 and ES2 we use QUADS to avoid drawing order issues
void rlDrawCircleV(rlVector2 center, float radius, rlColor color)
{
    rlDrawCircleSector(center, radius, 0, 360, 36, color);
}

// Draw a piece of a circle
void rlDrawCircleSector(rlVector2 center, float radius, float startAngle, float endAngle, int segments, rlColor color)
{
    if (radius <= 0.0f) radius = 0.1f;  // Avoid div by zero

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        float tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    int minSegments = (int)ceilf((endAngle - startAngle)/90);

    if (segments < minSegments)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (int)((endAngle - startAngle)*ceilf(2*RL_PI/th)/360);

        if (segments <= 0) segments = minSegments;
    }

    float stepLength = (endAngle - startAngle)/(float)segments;
    float angle = startAngle;

#if defined(RL_SUPPORT_QUADS_DRAW_MODE)
    rlglSetTexture(texShapes.id);

    rlglBegin(RL_QUADS);

        // NOTE: Every QUAD actually represents two segments
        for (int i = 0; i < segments/2; i++)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);

            rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(center.x, center.y);

            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength*2.0f))*radius, center.y + sinf(RL_DEG2RAD*(angle + stepLength*2.0f))*radius);

            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*radius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*radius);

            rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*radius, center.y + sinf(RL_DEG2RAD*angle)*radius);

            angle += (stepLength*2.0f);
        }

        // NOTE: In case number of segments is odd, we add one last piece to the cake
        if ((segments%2) == 1)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);

            rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(center.x, center.y);

            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*radius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*radius);

            rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*radius, center.y + sinf(RL_DEG2RAD*angle)*radius);

            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(center.x, center.y);
        }

    rlglEnd();

    rlglSetTexture(0);
#else
    rlglBegin(RL_TRIANGLES);
        for (int i = 0; i < segments; i++)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);

            rlglVertex2f(center.x, center.y);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*radius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*radius);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*radius, center.y + sinf(RL_DEG2RAD*angle)*radius);

            angle += stepLength;
        }
    rlglEnd();
#endif
}

// Draw a piece of a circle outlines
void rlDrawCircleSectorLines(rlVector2 center, float radius, float startAngle, float endAngle, int segments, rlColor color)
{
    if (radius <= 0.0f) radius = 0.1f;  // Avoid div by zero issue

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        float tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    int minSegments = (int)ceilf((endAngle - startAngle)/90);

    if (segments < minSegments)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (int)((endAngle - startAngle)*ceilf(2*RL_PI/th)/360);

        if (segments <= 0) segments = minSegments;
    }

    float stepLength = (endAngle - startAngle)/(float)segments;
    float angle = startAngle;
    bool showCapLines = true;

    rlglBegin(RL_LINES);
        if (showCapLines)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);
            rlglVertex2f(center.x, center.y);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*radius, center.y + sinf(RL_DEG2RAD*angle)*radius);
        }

        for (int i = 0; i < segments; i++)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);

            rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*radius, center.y + sinf(RL_DEG2RAD*angle)*radius);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*radius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*radius);

            angle += stepLength;
        }

        if (showCapLines)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);
            rlglVertex2f(center.x, center.y);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*radius, center.y + sinf(RL_DEG2RAD*angle)*radius);
        }
    rlglEnd();
}

// Draw a gradient-filled circle
// NOTE: Gradient goes from center (color1) to border (color2)
void rlDrawCircleGradient(int centerX, int centerY, float radius, rlColor color1, rlColor color2)
{
    rlglBegin(RL_TRIANGLES);
        for (int i = 0; i < 360; i += 10)
        {
            rlglColor4ub(color1.r, color1.g, color1.b, color1.a);
            rlglVertex2f((float)centerX, (float)centerY);
            rlglColor4ub(color2.r, color2.g, color2.b, color2.a);
            rlglVertex2f((float)centerX + cosf(RL_DEG2RAD*(i + 10))*radius, (float)centerY + sinf(RL_DEG2RAD*(i + 10))*radius);
            rlglColor4ub(color2.r, color2.g, color2.b, color2.a);
            rlglVertex2f((float)centerX + cosf(RL_DEG2RAD*i)*radius, (float)centerY + sinf(RL_DEG2RAD*i)*radius);
        }
    rlglEnd();
}

// Draw circle outline
void rlDrawCircleLines(int centerX, int centerY, float radius, rlColor color)
{
    rlDrawCircleLinesV(CAST(rlVector2){ (float)centerX, (float)centerY }, radius, color);
}

// Draw circle outline (Vector version)
void rlDrawCircleLinesV(rlVector2 center, float radius, rlColor color)
{
    rlglBegin(RL_LINES);
        rlglColor4ub(color.r, color.g, color.b, color.a);

        // NOTE: Circle outline is drawn pixel by pixel every degree (0 to 360)
        for (int i = 0; i < 360; i += 10)
        {
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*i)*radius, center.y + sinf(RL_DEG2RAD*i)*radius);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*(i + 10))*radius, center.y + sinf(RL_DEG2RAD*(i + 10))*radius);
        }
    rlglEnd();
}

// Draw ellipse
void rlDrawEllipse(int centerX, int centerY, float radiusH, float radiusV, rlColor color)
{
    rlglBegin(RL_TRIANGLES);
        for (int i = 0; i < 360; i += 10)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);
            rlglVertex2f((float)centerX, (float)centerY);
            rlglVertex2f((float)centerX + cosf(RL_DEG2RAD*(i + 10))*radiusH, (float)centerY + sinf(RL_DEG2RAD*(i + 10))*radiusV);
            rlglVertex2f((float)centerX + cosf(RL_DEG2RAD*i)*radiusH, (float)centerY + sinf(RL_DEG2RAD*i)*radiusV);
        }
    rlglEnd();
}

// Draw ellipse outline
void rlDrawEllipseLines(int centerX, int centerY, float radiusH, float radiusV, rlColor color)
{
    rlglBegin(RL_LINES);
        for (int i = 0; i < 360; i += 10)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);
            rlglVertex2f(centerX + cosf(RL_DEG2RAD*(i + 10))*radiusH, centerY + sinf(RL_DEG2RAD*(i + 10))*radiusV);
            rlglVertex2f(centerX + cosf(RL_DEG2RAD*i)*radiusH, centerY + sinf(RL_DEG2RAD*i)*radiusV);
        }
    rlglEnd();
}

// Draw ring
void rlDrawRing(rlVector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, rlColor color)
{
    if (startAngle == endAngle) return;

    // Function expects (outerRadius > innerRadius)
    if (outerRadius < innerRadius)
    {
        float tmp = outerRadius;
        outerRadius = innerRadius;
        innerRadius = tmp;

        if (outerRadius <= 0.0f) outerRadius = 0.1f;
    }

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        float tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    int minSegments = (int)ceilf((endAngle - startAngle)/90);

    if (segments < minSegments)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/outerRadius, 2) - 1);
        segments = (int)((endAngle - startAngle)*ceilf(2*RL_PI/th)/360);

        if (segments <= 0) segments = minSegments;
    }

    // Not a ring
    if (innerRadius <= 0.0f)
    {
        rlDrawCircleSector(center, outerRadius, startAngle, endAngle, segments, color);
        return;
    }

    float stepLength = (endAngle - startAngle)/(float)segments;
    float angle = startAngle;

#if defined(RL_SUPPORT_QUADS_DRAW_MODE)
    rlglSetTexture(texShapes.id);

    rlglBegin(RL_QUADS);
        for (int i = 0; i < segments; i++)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);

            rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*outerRadius, center.y + sinf(RL_DEG2RAD*angle)*outerRadius);

            rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*innerRadius, center.y + sinf(RL_DEG2RAD*angle)*innerRadius);

            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*innerRadius);

            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*outerRadius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*outerRadius);

            angle += stepLength;
        }
    rlglEnd();

    rlglSetTexture(0);
#else
    rlglBegin(RL_TRIANGLES);
        for (int i = 0; i < segments; i++)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);

            rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*innerRadius, center.y + sinf(RL_DEG2RAD*angle)*innerRadius);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*innerRadius);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*outerRadius, center.y + sinf(RL_DEG2RAD*angle)*outerRadius);

            rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*innerRadius);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*outerRadius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*outerRadius);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*outerRadius, center.y + sinf(RL_DEG2RAD*angle)*outerRadius);

            angle += stepLength;
        }
    rlglEnd();
#endif
}

// Draw ring outline
void rlDrawRingLines(rlVector2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, int segments, rlColor color)
{
    if (startAngle == endAngle) return;

    // Function expects (outerRadius > innerRadius)
    if (outerRadius < innerRadius)
    {
        float tmp = outerRadius;
        outerRadius = innerRadius;
        innerRadius = tmp;

        if (outerRadius <= 0.0f) outerRadius = 0.1f;
    }

    // Function expects (endAngle > startAngle)
    if (endAngle < startAngle)
    {
        // Swap values
        float tmp = startAngle;
        startAngle = endAngle;
        endAngle = tmp;
    }

    int minSegments = (int)ceilf((endAngle - startAngle)/90);

    if (segments < minSegments)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/outerRadius, 2) - 1);
        segments = (int)((endAngle - startAngle)*ceilf(2*RL_PI/th)/360);

        if (segments <= 0) segments = minSegments;
    }

    if (innerRadius <= 0.0f)
    {
        rlDrawCircleSectorLines(center, outerRadius, startAngle, endAngle, segments, color);
        return;
    }

    float stepLength = (endAngle - startAngle)/(float)segments;
    float angle = startAngle;
    bool showCapLines = true;

    rlglBegin(RL_LINES);
        if (showCapLines)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*outerRadius, center.y + sinf(RL_DEG2RAD*angle)*outerRadius);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*innerRadius, center.y + sinf(RL_DEG2RAD*angle)*innerRadius);
        }

        for (int i = 0; i < segments; i++)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);

            rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*outerRadius, center.y + sinf(RL_DEG2RAD*angle)*outerRadius);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*outerRadius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*outerRadius);

            rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*innerRadius, center.y + sinf(RL_DEG2RAD*angle)*innerRadius);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*innerRadius);

            angle += stepLength;
        }

        if (showCapLines)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*outerRadius, center.y + sinf(RL_DEG2RAD*angle)*outerRadius);
            rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*innerRadius, center.y + sinf(RL_DEG2RAD*angle)*innerRadius);
        }
    rlglEnd();
}

// Draw a color-filled rectangle
void rlDrawRectangle(int posX, int posY, int width, int height, rlColor color)
{
    rlDrawRectangleV(CAST(rlVector2){ (float)posX, (float)posY }, CAST(rlVector2){ (float)width, (float)height }, color);
}

// Draw a color-filled rectangle (Vector version)
// NOTE: On OpenGL 3.3 and ES2 we use QUADS to avoid drawing order issues
void rlDrawRectangleV(rlVector2 position, rlVector2 size, rlColor color)
{
    rlDrawRectanglePro(CAST(rlRectangle){ position.x, position.y, size.x, size.y }, CAST(rlVector2){ 0.0f, 0.0f }, 0.0f, color);
}

// Draw a color-filled rectangle
void rlDrawRectangleRec(rlRectangle rec, rlColor color)
{
    rlDrawRectanglePro(rec, CAST(rlVector2){ 0.0f, 0.0f }, 0.0f, color);
}

// Draw a color-filled rectangle with pro parameters
void rlDrawRectanglePro(rlRectangle rec, rlVector2 origin, float rotation, rlColor color)
{
    rlVector2 topLeft = { 0 };
    rlVector2 topRight = { 0 };
    rlVector2 bottomLeft = { 0 };
    rlVector2 bottomRight = { 0 };

    // Only calculate rotation if needed
    if (rotation == 0.0f)
    {
        float x = rec.x - origin.x;
        float y = rec.y - origin.y;
        topLeft = CAST(rlVector2){ x, y };
        topRight = CAST(rlVector2){ x + rec.width, y };
        bottomLeft = CAST(rlVector2){ x, y + rec.height };
        bottomRight = CAST(rlVector2){ x + rec.width, y + rec.height };
    }
    else
    {
        float sinRotation = sinf(rotation*RL_DEG2RAD);
        float cosRotation = cosf(rotation*RL_DEG2RAD);
        float x = rec.x;
        float y = rec.y;
        float dx = -origin.x;
        float dy = -origin.y;

        topLeft.x = x + dx*cosRotation - dy*sinRotation;
        topLeft.y = y + dx*sinRotation + dy*cosRotation;

        topRight.x = x + (dx + rec.width)*cosRotation - dy*sinRotation;
        topRight.y = y + (dx + rec.width)*sinRotation + dy*cosRotation;

        bottomLeft.x = x + dx*cosRotation - (dy + rec.height)*sinRotation;
        bottomLeft.y = y + dx*sinRotation + (dy + rec.height)*cosRotation;

        bottomRight.x = x + (dx + rec.width)*cosRotation - (dy + rec.height)*sinRotation;
        bottomRight.y = y + (dx + rec.width)*sinRotation + (dy + rec.height)*cosRotation;
    }

#if defined(RL_SUPPORT_QUADS_DRAW_MODE)
    rlglSetTexture(texShapes.id);

    rlglBegin(RL_QUADS);

        rlglNormal3f(0.0f, 0.0f, 1.0f);
        rlglColor4ub(color.r, color.g, color.b, color.a);

        rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(topLeft.x, topLeft.y);

        rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(bottomLeft.x, bottomLeft.y);

        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(bottomRight.x, bottomRight.y);

        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(topRight.x, topRight.y);

    rlglEnd();

    rlglSetTexture(0);
#else
    rlglBegin(RL_TRIANGLES);

        rlglColor4ub(color.r, color.g, color.b, color.a);

        rlglVertex2f(topLeft.x, topLeft.y);
        rlglVertex2f(bottomLeft.x, bottomLeft.y);
        rlglVertex2f(topRight.x, topRight.y);

        rlglVertex2f(topRight.x, topRight.y);
        rlglVertex2f(bottomLeft.x, bottomLeft.y);
        rlglVertex2f(bottomRight.x, bottomRight.y);

    rlglEnd();
#endif
}

// Draw a vertical-gradient-filled rectangle
// NOTE: Gradient goes from bottom (color1) to top (color2)
void rlDrawRectangleGradientV(int posX, int posY, int width, int height, rlColor color1, rlColor color2)
{
    rlDrawRectangleGradientEx(CAST(rlRectangle){ (float)posX, (float)posY, (float)width, (float)height }, color1, color2, color2, color1);
}

// Draw a horizontal-gradient-filled rectangle
// NOTE: Gradient goes from bottom (color1) to top (color2)
void rlDrawRectangleGradientH(int posX, int posY, int width, int height, rlColor color1, rlColor color2)
{
    rlDrawRectangleGradientEx(CAST(rlRectangle){ (float)posX, (float)posY, (float)width, (float)height }, color1, color1, color2, color2);
}

// Draw a gradient-filled rectangle
// NOTE: Colors refer to corners, starting at top-lef corner and counter-clockwise
void rlDrawRectangleGradientEx(rlRectangle rec, rlColor col1, rlColor col2, rlColor col3, rlColor col4)
{
    rlglSetTexture(texShapes.id);

    rlglBegin(RL_QUADS);
        rlglNormal3f(0.0f, 0.0f, 1.0f);

        // NOTE: Default raylib font character 95 is a white square
        rlglColor4ub(col1.r, col1.g, col1.b, col1.a);
        rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(rec.x, rec.y);

        rlglColor4ub(col2.r, col2.g, col2.b, col2.a);
        rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(rec.x, rec.y + rec.height);

        rlglColor4ub(col3.r, col3.g, col3.b, col3.a);
        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(rec.x + rec.width, rec.y + rec.height);

        rlglColor4ub(col4.r, col4.g, col4.b, col4.a);
        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(rec.x + rec.width, rec.y);
    rlglEnd();

    rlglSetTexture(0);
}

// Draw rectangle outline
// NOTE: On OpenGL 3.3 and ES2 we use QUADS to avoid drawing order issues
void rlDrawRectangleLines(int posX, int posY, int width, int height, rlColor color)
{
#if defined(RL_SUPPORT_QUADS_DRAW_MODE)
    rlDrawRectangle(posX, posY, width, 1, color);
    rlDrawRectangle(posX + width - 1, posY + 1, 1, height - 2, color);
    rlDrawRectangle(posX, posY + height - 1, width, 1, color);
    rlDrawRectangle(posX, posY + 1, 1, height - 2, color);
#else
    rlglBegin(RL_LINES);
        rlglColor4ub(color.r, color.g, color.b, color.a);
        rlglVertex2f(posX + 1, posY + 1);
        rlglVertex2f(posX + width, posY + 1);

        rlglVertex2f(posX + width, posY + 1);
        rlglVertex2f(posX + width, posY + height);

        rlglVertex2f(posX + width, posY + height);
        rlglVertex2f(posX + 1, posY + height);

        rlglVertex2f(posX + 1, posY + height);
        rlglVertex2f(posX + 1, posY + 1);
    rlglEnd();
#endif
}

// Draw rectangle outline with extended parameters
void rlDrawRectangleLinesEx(rlRectangle rec, float lineThick, rlColor color)
{
    if ((lineThick > rec.width) || (lineThick > rec.height))
    {
        if (rec.width > rec.height) lineThick = rec.height/2;
        else if (rec.width < rec.height) lineThick = rec.width/2;
    }

    // When rec = { x, y, 8.0f, 6.0f } and lineThick = 2, the following
    // four rectangles are drawn ([T]op, [B]ottom, [L]eft, [R]ight):
    //
    //   TTTTTTTT
    //   TTTTTTTT
    //   LL    RR
    //   LL    RR
    //   BBBBBBBB
    //   BBBBBBBB
    //

    rlRectangle top = { rec.x, rec.y, rec.width, lineThick };
    rlRectangle bottom = { rec.x, rec.y - lineThick + rec.height, rec.width, lineThick };
    rlRectangle left = { rec.x, rec.y + lineThick, lineThick, rec.height - lineThick*2.0f };
    rlRectangle right = { rec.x - lineThick + rec.width, rec.y + lineThick, lineThick, rec.height - lineThick*2.0f };

    rlDrawRectangleRec(top, color);
    rlDrawRectangleRec(bottom, color);
    rlDrawRectangleRec(left, color);
    rlDrawRectangleRec(right, color);
}

// Draw rectangle with rounded edges
void rlDrawRectangleRounded(rlRectangle rec, float roundness, int segments, rlColor color)
{
    // Not a rounded rectangle
    if ((roundness <= 0.0f) || (rec.width < 1) || (rec.height < 1 ))
    {
        rlDrawRectangleRec(rec, color);
        return;
    }

    if (roundness >= 1.0f) roundness = 1.0f;

    // Calculate corner radius
    float radius = (rec.width > rec.height)? (rec.height*roundness)/2 : (rec.width*roundness)/2;
    if (radius <= 0.0f) return;

    // Calculate number of segments to use for the corners
    if (segments < 4)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (int)(ceilf(2*RL_PI/th)/4.0f);
        if (segments <= 0) segments = 4;
    }

    float stepLength = 90.0f/(float)segments;

    /*
    Quick sketch to make sense of all of this,
    there are 9 parts to draw, also mark the 12 points we'll use

          P0____________________P1
          /|                    |\
         /1|          2         |3\
     P7 /__|____________________|__\ P2
       |   |P8                P9|   |
       | 8 |          9         | 4 |
       | __|____________________|__ |
     P6 \  |P11              P10|  / P3
         \7|          6         |5/
          \|____________________|/
          P5                    P4
    */
    // Coordinates of the 12 points that define the rounded rect
    const rlVector2 point[12] = {
        {(float)rec.x + radius, rec.y}, {(float)(rec.x + rec.width) - radius, rec.y}, { rec.x + rec.width, (float)rec.y + radius },     // PO, P1, P2
        {rec.x + rec.width, (float)(rec.y + rec.height) - radius}, {(float)(rec.x + rec.width) - radius, rec.y + rec.height},           // P3, P4
        {(float)rec.x + radius, rec.y + rec.height}, { rec.x, (float)(rec.y + rec.height) - radius}, {rec.x, (float)rec.y + radius},    // P5, P6, P7
        {(float)rec.x + radius, (float)rec.y + radius}, {(float)(rec.x + rec.width) - radius, (float)rec.y + radius},                   // P8, P9
        {(float)(rec.x + rec.width) - radius, (float)(rec.y + rec.height) - radius}, {(float)rec.x + radius, (float)(rec.y + rec.height) - radius} // P10, P11
    };

    const rlVector2 centers[4] = { point[8], point[9], point[10], point[11] };
    const float angles[4] = { 180.0f, 270.0f, 0.0f, 90.0f };

#if defined(RL_SUPPORT_QUADS_DRAW_MODE)
    rlglSetTexture(texShapes.id);

    rlglBegin(RL_QUADS);
        // Draw all the 4 corners: [1] Upper Left Corner, [3] Upper Right Corner, [5] Lower Right Corner, [7] Lower Left Corner
        for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
        {
            float angle = angles[k];
            const rlVector2 center = centers[k];

            // NOTE: Every QUAD actually represents two segments
            for (int i = 0; i < segments/2; i++)
            {
                rlglColor4ub(color.r, color.g, color.b, color.a);
                rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
                rlglVertex2f(center.x, center.y);

                rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
                rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength*2))*radius, center.y + sinf(RL_DEG2RAD*(angle + stepLength*2))*radius);

                rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*radius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*radius);

                rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*radius, center.y + sinf(RL_DEG2RAD*angle)*radius);

                angle += (stepLength*2);
            }

            // NOTE: In case number of segments is odd, we add one last piece to the cake
            if (segments%2)
            {
                rlglColor4ub(color.r, color.g, color.b, color.a);
                rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
                rlglVertex2f(center.x, center.y);

                rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*radius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*radius);

                rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*radius, center.y + sinf(RL_DEG2RAD*angle)*radius);

                rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
                rlglVertex2f(center.x, center.y);
            }
        }

        // [2] Upper rlRectangle
        rlglColor4ub(color.r, color.g, color.b, color.a);
        rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(point[0].x, point[0].y);
        rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(point[8].x, point[8].y);
        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(point[9].x, point[9].y);
        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(point[1].x, point[1].y);

        // [4] Right rlRectangle
        rlglColor4ub(color.r, color.g, color.b, color.a);
        rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(point[2].x, point[2].y);
        rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(point[9].x, point[9].y);
        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(point[10].x, point[10].y);
        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(point[3].x, point[3].y);

        // [6] Bottom rlRectangle
        rlglColor4ub(color.r, color.g, color.b, color.a);
        rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(point[11].x, point[11].y);
        rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(point[5].x, point[5].y);
        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(point[4].x, point[4].y);
        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(point[10].x, point[10].y);

        // [8] Left rlRectangle
        rlglColor4ub(color.r, color.g, color.b, color.a);
        rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(point[7].x, point[7].y);
        rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(point[6].x, point[6].y);
        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(point[11].x, point[11].y);
        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(point[8].x, point[8].y);

        // [9] Middle rlRectangle
        rlglColor4ub(color.r, color.g, color.b, color.a);
        rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(point[8].x, point[8].y);
        rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(point[11].x, point[11].y);
        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(point[10].x, point[10].y);
        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(point[9].x, point[9].y);

    rlglEnd();
    rlglSetTexture(0);
#else
    rlglBegin(RL_TRIANGLES);

        // Draw all of the 4 corners: [1] Upper Left Corner, [3] Upper Right Corner, [5] Lower Right Corner, [7] Lower Left Corner
        for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
        {
            float angle = angles[k];
            const rlVector2 center = centers[k];
            for (int i = 0; i < segments; i++)
            {
                rlglColor4ub(color.r, color.g, color.b, color.a);
                rlglVertex2f(center.x, center.y);
                rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*radius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*radius);
                rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*radius, center.y + sinf(RL_DEG2RAD*angle)*radius);
                angle += stepLength;
            }
        }

        // [2] Upper rlRectangle
        rlglColor4ub(color.r, color.g, color.b, color.a);
        rlglVertex2f(point[0].x, point[0].y);
        rlglVertex2f(point[8].x, point[8].y);
        rlglVertex2f(point[9].x, point[9].y);
        rlglVertex2f(point[1].x, point[1].y);
        rlglVertex2f(point[0].x, point[0].y);
        rlglVertex2f(point[9].x, point[9].y);

        // [4] Right rlRectangle
        rlglColor4ub(color.r, color.g, color.b, color.a);
        rlglVertex2f(point[9].x, point[9].y);
        rlglVertex2f(point[10].x, point[10].y);
        rlglVertex2f(point[3].x, point[3].y);
        rlglVertex2f(point[2].x, point[2].y);
        rlglVertex2f(point[9].x, point[9].y);
        rlglVertex2f(point[3].x, point[3].y);

        // [6] Bottom rlRectangle
        rlglColor4ub(color.r, color.g, color.b, color.a);
        rlglVertex2f(point[11].x, point[11].y);
        rlglVertex2f(point[5].x, point[5].y);
        rlglVertex2f(point[4].x, point[4].y);
        rlglVertex2f(point[10].x, point[10].y);
        rlglVertex2f(point[11].x, point[11].y);
        rlglVertex2f(point[4].x, point[4].y);

        // [8] Left rlRectangle
        rlglColor4ub(color.r, color.g, color.b, color.a);
        rlglVertex2f(point[7].x, point[7].y);
        rlglVertex2f(point[6].x, point[6].y);
        rlglVertex2f(point[11].x, point[11].y);
        rlglVertex2f(point[8].x, point[8].y);
        rlglVertex2f(point[7].x, point[7].y);
        rlglVertex2f(point[11].x, point[11].y);

        // [9] Middle rlRectangle
        rlglColor4ub(color.r, color.g, color.b, color.a);
        rlglVertex2f(point[8].x, point[8].y);
        rlglVertex2f(point[11].x, point[11].y);
        rlglVertex2f(point[10].x, point[10].y);
        rlglVertex2f(point[9].x, point[9].y);
        rlglVertex2f(point[8].x, point[8].y);
        rlglVertex2f(point[10].x, point[10].y);
    rlglEnd();
#endif
}

// Draw rectangle with rounded edges outline
void rlDrawRectangleRoundedLines(rlRectangle rec, float roundness, int segments, float lineThick, rlColor color)
{
    if (lineThick < 0) lineThick = 0;

    // Not a rounded rectangle
    if (roundness <= 0.0f)
    {
        rlDrawRectangleLinesEx(CAST(rlRectangle){rec.x-lineThick, rec.y-lineThick, rec.width+2*lineThick, rec.height+2*lineThick}, lineThick, color);
        return;
    }

    if (roundness >= 1.0f) roundness = 1.0f;

    // Calculate corner radius
    float radius = (rec.width > rec.height)? (rec.height*roundness)/2 : (rec.width*roundness)/2;
    if (radius <= 0.0f) return;

    // Calculate number of segments to use for the corners
    if (segments < 4)
    {
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (int)(ceilf(2*RL_PI/th)/2.0f);
        if (segments <= 0) segments = 4;
    }

    float stepLength = 90.0f/(float)segments;
    const float outerRadius = radius + lineThick, innerRadius = radius;

    /*
    Quick sketch to make sense of all of this,
    marks the 16 + 4(corner centers P16-19) points we'll use

           P0 ================== P1
          // P8                P9 \\
         //                        \\
     P7 // P15                  P10 \\ P2
       ||   *P16             P17*    ||
       ||                            ||
       || P14                   P11  ||
     P6 \\  *P19             P18*   // P3
         \\                        //
          \\ P13              P12 //
           P5 ================== P4
    */
    const rlVector2 point[16] = {
        {(float)rec.x + innerRadius, rec.y - lineThick}, {(float)(rec.x + rec.width) - innerRadius, rec.y - lineThick}, { rec.x + rec.width + lineThick, (float)rec.y + innerRadius }, // PO, P1, P2
        {rec.x + rec.width + lineThick, (float)(rec.y + rec.height) - innerRadius}, {(float)(rec.x + rec.width) - innerRadius, rec.y + rec.height + lineThick}, // P3, P4
        {(float)rec.x + innerRadius, rec.y + rec.height + lineThick}, { rec.x - lineThick, (float)(rec.y + rec.height) - innerRadius}, {rec.x - lineThick, (float)rec.y + innerRadius}, // P5, P6, P7
        {(float)rec.x + innerRadius, rec.y}, {(float)(rec.x + rec.width) - innerRadius, rec.y}, // P8, P9
        { rec.x + rec.width, (float)rec.y + innerRadius }, {rec.x + rec.width, (float)(rec.y + rec.height) - innerRadius}, // P10, P11
        {(float)(rec.x + rec.width) - innerRadius, rec.y + rec.height}, {(float)rec.x + innerRadius, rec.y + rec.height}, // P12, P13
        { rec.x, (float)(rec.y + rec.height) - innerRadius}, {rec.x, (float)rec.y + innerRadius} // P14, P15
    };

    const rlVector2 centers[4] = {
        {(float)rec.x + innerRadius, (float)rec.y + innerRadius}, {(float)(rec.x + rec.width) - innerRadius, (float)rec.y + innerRadius}, // P16, P17
        {(float)(rec.x + rec.width) - innerRadius, (float)(rec.y + rec.height) - innerRadius}, {(float)rec.x + innerRadius, (float)(rec.y + rec.height) - innerRadius} // P18, P19
    };

    const float angles[4] = { 180.0f, 270.0f, 0.0f, 90.0f };

    if (lineThick > 1)
    {
#if defined(RL_SUPPORT_QUADS_DRAW_MODE)
        rlglSetTexture(texShapes.id);

        rlglBegin(RL_QUADS);

            // Draw all the 4 corners first: Upper Left Corner, Upper Right Corner, Lower Right Corner, Lower Left Corner
            for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
            {
                float angle = angles[k];
                const rlVector2 center = centers[k];
                for (int i = 0; i < segments; i++)
                {
                    rlglColor4ub(color.r, color.g, color.b, color.a);

                    rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
                    rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*innerRadius, center.y + sinf(RL_DEG2RAD*angle)*innerRadius);

                    rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
                    rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*innerRadius);

                    rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                    rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*outerRadius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*outerRadius);

                    rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                    rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*outerRadius, center.y + sinf(RL_DEG2RAD*angle)*outerRadius);

                    angle += stepLength;
                }
            }

            // Upper rectangle
            rlglColor4ub(color.r, color.g, color.b, color.a);
            rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(point[0].x, point[0].y);
            rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(point[8].x, point[8].y);
            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(point[9].x, point[9].y);
            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(point[1].x, point[1].y);

            // Right rectangle
            rlglColor4ub(color.r, color.g, color.b, color.a);
            rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(point[2].x, point[2].y);
            rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(point[10].x, point[10].y);
            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(point[11].x, point[11].y);
            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(point[3].x, point[3].y);

            // Lower rectangle
            rlglColor4ub(color.r, color.g, color.b, color.a);
            rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(point[13].x, point[13].y);
            rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(point[5].x, point[5].y);
            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(point[4].x, point[4].y);
            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(point[12].x, point[12].y);

            // Left rectangle
            rlglColor4ub(color.r, color.g, color.b, color.a);
            rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(point[15].x, point[15].y);
            rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(point[7].x, point[7].y);
            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(point[6].x, point[6].y);
            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(point[14].x, point[14].y);

        rlglEnd();
        rlglSetTexture(0);
#else
        rlglBegin(RL_TRIANGLES);

            // Draw all of the 4 corners first: Upper Left Corner, Upper Right Corner, Lower Right Corner, Lower Left Corner
            for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
            {
                float angle = angles[k];
                const rlVector2 center = centers[k];

                for (int i = 0; i < segments; i++)
                {
                    rlglColor4ub(color.r, color.g, color.b, color.a);

                    rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*innerRadius, center.y + sinf(RL_DEG2RAD*angle)*innerRadius);
                    rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*innerRadius);
                    rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*outerRadius, center.y + sinf(RL_DEG2RAD*angle)*outerRadius);

                    rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*innerRadius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*innerRadius);
                    rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*outerRadius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*outerRadius);
                    rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*outerRadius, center.y + sinf(RL_DEG2RAD*angle)*outerRadius);

                    angle += stepLength;
                }
            }

            // Upper rectangle
            rlglColor4ub(color.r, color.g, color.b, color.a);
            rlglVertex2f(point[0].x, point[0].y);
            rlglVertex2f(point[8].x, point[8].y);
            rlglVertex2f(point[9].x, point[9].y);
            rlglVertex2f(point[1].x, point[1].y);
            rlglVertex2f(point[0].x, point[0].y);
            rlglVertex2f(point[9].x, point[9].y);

            // Right rectangle
            rlglColor4ub(color.r, color.g, color.b, color.a);
            rlglVertex2f(point[10].x, point[10].y);
            rlglVertex2f(point[11].x, point[11].y);
            rlglVertex2f(point[3].x, point[3].y);
            rlglVertex2f(point[2].x, point[2].y);
            rlglVertex2f(point[10].x, point[10].y);
            rlglVertex2f(point[3].x, point[3].y);

            // Lower rectangle
            rlglColor4ub(color.r, color.g, color.b, color.a);
            rlglVertex2f(point[13].x, point[13].y);
            rlglVertex2f(point[5].x, point[5].y);
            rlglVertex2f(point[4].x, point[4].y);
            rlglVertex2f(point[12].x, point[12].y);
            rlglVertex2f(point[13].x, point[13].y);
            rlglVertex2f(point[4].x, point[4].y);

            // Left rectangle
            rlglColor4ub(color.r, color.g, color.b, color.a);
            rlglVertex2f(point[7].x, point[7].y);
            rlglVertex2f(point[6].x, point[6].y);
            rlglVertex2f(point[14].x, point[14].y);
            rlglVertex2f(point[15].x, point[15].y);
            rlglVertex2f(point[7].x, point[7].y);
            rlglVertex2f(point[14].x, point[14].y);
        rlglEnd();
#endif
    }
    else
    {
        // Use LINES to draw the outline
        rlglBegin(RL_LINES);

            // Draw all the 4 corners first: Upper Left Corner, Upper Right Corner, Lower Right Corner, Lower Left Corner
            for (int k = 0; k < 4; ++k) // Hope the compiler is smart enough to unroll this loop
            {
                float angle = angles[k];
                const rlVector2 center = centers[k];

                for (int i = 0; i < segments; i++)
                {
                    rlglColor4ub(color.r, color.g, color.b, color.a);
                    rlglVertex2f(center.x + cosf(RL_DEG2RAD*angle)*outerRadius, center.y + sinf(RL_DEG2RAD*angle)*outerRadius);
                    rlglVertex2f(center.x + cosf(RL_DEG2RAD*(angle + stepLength))*outerRadius, center.y + sinf(RL_DEG2RAD*(angle + stepLength))*outerRadius);
                    angle += stepLength;
                }
            }

            // And now the remaining 4 lines
            for (int i = 0; i < 8; i += 2)
            {
                rlglColor4ub(color.r, color.g, color.b, color.a);
                rlglVertex2f(point[i].x, point[i].y);
                rlglVertex2f(point[i + 1].x, point[i + 1].y);
            }

        rlglEnd();
    }
}

// Draw a triangle
// NOTE: Vertex must be provided in counter-clockwise order
void rlDrawTriangle(rlVector2 v1, rlVector2 v2, rlVector2 v3, rlColor color)
{
#if defined(RL_SUPPORT_QUADS_DRAW_MODE)
    rlglSetTexture(texShapes.id);

    rlglBegin(RL_QUADS);
        rlglColor4ub(color.r, color.g, color.b, color.a);

        rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(v1.x, v1.y);

        rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(v2.x, v2.y);

        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
        rlglVertex2f(v2.x, v2.y);

        rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
        rlglVertex2f(v3.x, v3.y);
    rlglEnd();

    rlglSetTexture(0);
#else
    rlglBegin(RL_TRIANGLES);
        rlglColor4ub(color.r, color.g, color.b, color.a);
        rlglVertex2f(v1.x, v1.y);
        rlglVertex2f(v2.x, v2.y);
        rlglVertex2f(v3.x, v3.y);
    rlglEnd();
#endif
}

// Draw a triangle using lines
// NOTE: Vertex must be provided in counter-clockwise order
void rlDrawTriangleLines(rlVector2 v1, rlVector2 v2, rlVector2 v3, rlColor color)
{
    rlglBegin(RL_LINES);
        rlglColor4ub(color.r, color.g, color.b, color.a);
        rlglVertex2f(v1.x, v1.y);
        rlglVertex2f(v2.x, v2.y);

        rlglVertex2f(v2.x, v2.y);
        rlglVertex2f(v3.x, v3.y);

        rlglVertex2f(v3.x, v3.y);
        rlglVertex2f(v1.x, v1.y);
    rlglEnd();
}

// Draw a triangle fan defined by points
// NOTE: First vertex provided is the center, shared by all triangles
// By default, following vertex should be provided in counter-clockwise order
void rlDrawTriangleFan(rlVector2 *points, int pointCount, rlColor color)
{
    if (pointCount >= 3)
    {
        rlglSetTexture(texShapes.id);
        rlglBegin(RL_QUADS);
            rlglColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 1; i < pointCount - 1; i++)
            {
                rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
                rlglVertex2f(points[0].x, points[0].y);

                rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                rlglVertex2f(points[i].x, points[i].y);

                rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
                rlglVertex2f(points[i + 1].x, points[i + 1].y);

                rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
                rlglVertex2f(points[i + 1].x, points[i + 1].y);
            }
        rlglEnd();
        rlglSetTexture(0);
    }
}

// Draw a triangle strip defined by points
// NOTE: Every new vertex connects with previous two
void rlDrawTriangleStrip(rlVector2 *points, int pointCount, rlColor color)
{
    if (pointCount >= 3)
    {
        rlglBegin(RL_TRIANGLES);
            rlglColor4ub(color.r, color.g, color.b, color.a);

            for (int i = 2; i < pointCount; i++)
            {
                if ((i%2) == 0)
                {
                    rlglVertex2f(points[i].x, points[i].y);
                    rlglVertex2f(points[i - 2].x, points[i - 2].y);
                    rlglVertex2f(points[i - 1].x, points[i - 1].y);
                }
                else
                {
                    rlglVertex2f(points[i].x, points[i].y);
                    rlglVertex2f(points[i - 1].x, points[i - 1].y);
                    rlglVertex2f(points[i - 2].x, points[i - 2].y);
                }
            }
        rlglEnd();
    }
}

// Draw a regular polygon of n sides (Vector version)
void rlDrawPoly(rlVector2 center, int sides, float radius, float rotation, rlColor color)
{
    if (sides < 3) sides = 3;
    float centralAngle = rotation*RL_DEG2RAD;
    float angleStep = 360.0f/(float)sides*RL_DEG2RAD;

#if defined(RL_SUPPORT_QUADS_DRAW_MODE)
    rlglSetTexture(texShapes.id);

    rlglBegin(RL_QUADS);
        for (int i = 0; i < sides; i++)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);
            float nextAngle = centralAngle + angleStep;

            rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(center.x, center.y);

            rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(center.x + cosf(centralAngle)*radius, center.y + sinf(centralAngle)*radius);

            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(center.x + cosf(nextAngle)*radius, center.y + sinf(nextAngle)*radius);

            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(center.x + cosf(centralAngle)*radius, center.y + sinf(centralAngle)*radius);

            centralAngle = nextAngle;
        }
    rlglEnd();
    rlglSetTexture(0);
#else
    rlglBegin(RL_TRIANGLES);
        for (int i = 0; i < sides; i++)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);

            rlglVertex2f(center.x, center.y);
            rlglVertex2f(center.x + cosf(centralAngle + angleStep)*radius, center.y + sinf(centralAngle + angleStep)*radius);
            rlglVertex2f(center.x + cosf(centralAngle)*radius, center.y + sinf(centralAngle)*radius);

            centralAngle += angleStep;
        }
    rlglEnd();
#endif
}

// Draw a polygon outline of n sides
void rlDrawPolyLines(rlVector2 center, int sides, float radius, float rotation, rlColor color)
{
    if (sides < 3) sides = 3;
    float centralAngle = rotation*RL_DEG2RAD;
    float angleStep = 360.0f/(float)sides*RL_DEG2RAD;

    rlglBegin(RL_LINES);
        for (int i = 0; i < sides; i++)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);

            rlglVertex2f(center.x + cosf(centralAngle)*radius, center.y + sinf(centralAngle)*radius);
            rlglVertex2f(center.x + cosf(centralAngle + angleStep)*radius, center.y + sinf(centralAngle + angleStep)*radius);

            centralAngle += angleStep;
        }
    rlglEnd();
}

void rlDrawPolyLinesEx(rlVector2 center, int sides, float radius, float rotation, float lineThick, rlColor color)
{
    if (sides < 3) sides = 3;
    float centralAngle = rotation*RL_DEG2RAD;
    float exteriorAngle = 360.0f/(float)sides*RL_DEG2RAD;
    float innerRadius = radius - (lineThick*cosf(RL_DEG2RAD*exteriorAngle/2.0f));

#if defined(RL_SUPPORT_QUADS_DRAW_MODE)
    rlglSetTexture(texShapes.id);

    rlglBegin(RL_QUADS);
        for (int i = 0; i < sides; i++)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);
            float nextAngle = centralAngle + exteriorAngle;

            rlglTexCoord2f(texShapesRec.x/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(center.x + cosf(centralAngle)*radius, center.y + sinf(centralAngle)*radius);

            rlglTexCoord2f(texShapesRec.x/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(center.x + cosf(centralAngle)*innerRadius, center.y + sinf(centralAngle)*innerRadius);

            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, (texShapesRec.y + texShapesRec.height)/texShapes.height);
            rlglVertex2f(center.x + cosf(nextAngle)*innerRadius, center.y + sinf(nextAngle)*innerRadius);

            rlglTexCoord2f((texShapesRec.x + texShapesRec.width)/texShapes.width, texShapesRec.y/texShapes.height);
            rlglVertex2f(center.x + cosf(nextAngle)*radius, center.y + sinf(nextAngle)*radius);

            centralAngle = nextAngle;
        }
    rlglEnd();
    rlglSetTexture(0);
#else
    rlglBegin(RL_TRIANGLES);
        for (int i = 0; i < sides; i++)
        {
            rlglColor4ub(color.r, color.g, color.b, color.a);
            float nextAngle = centralAngle + exteriorAngle;

            rlglVertex2f(center.x + cosf(nextAngle)*radius, center.y + sinf(nextAngle)*radius);
            rlglVertex2f(center.x + cosf(centralAngle)*radius, center.y + sinf(centralAngle)*radius);
            rlglVertex2f(center.x + cosf(centralAngle)*innerRadius, center.y + sinf(centralAngle)*innerRadius);

            rlglVertex2f(center.x + cosf(centralAngle)*innerRadius, center.y + sinf(centralAngle)*innerRadius);
            rlglVertex2f(center.x + cosf(nextAngle)*innerRadius, center.y + sinf(nextAngle)*innerRadius);
            rlglVertex2f(center.x + cosf(nextAngle)*radius, center.y + sinf(nextAngle)*radius);

            centralAngle = nextAngle;
        }
    rlglEnd();
#endif
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Splines functions
//----------------------------------------------------------------------------------

// Draw spline: linear, minimum 2 points
void DrawSplineLinear(rlVector2 *points, int pointCount, float thick, rlColor color)
{
    rlVector2 delta = { 0 };
    float length = 0.0f;
    float scale = 0.0f;

    for (int i = 0; i < pointCount - 1; i++)
    {
        delta = CAST(rlVector2){ points[i + 1].x - points[i].x, points[i + 1].y - points[i].y };
        length = sqrtf(delta.x*delta.x + delta.y*delta.y);

        if (length > 0) scale = thick/(2*length);

        rlVector2 radius = { -scale*delta.y, scale*delta.x };
        rlVector2 strip[4] = {
            { points[i].x - radius.x, points[i].y - radius.y },
            { points[i].x + radius.x, points[i].y + radius.y },
            { points[i + 1].x - radius.x, points[i + 1].y - radius.y },
            { points[i + 1].x + radius.x, points[i + 1].y + radius.y }
        };

        rlDrawTriangleStrip(strip, 4, color);
    }
#if defined(RL_SUPPORT_SPLINE_SEGMENT_CAPS)

#endif
}

// Draw spline: B-Spline, minimum 4 points
void DrawSplineBasis(rlVector2 *points, int pointCount, float thick, rlColor color)
{
    if (pointCount < 4) return;

    float a[4] = { 0 };
    float b[4] = { 0 };
    float dy = 0.0f;
    float dx = 0.0f;
    float size = 0.0f;

    rlVector2 currentPoint = { 0 };
    rlVector2 nextPoint = { 0 };
    rlVector2 vertices[2*SPLINE_SEGMENT_DIVISIONS + 2] = { 0 };

    for (int i = 0; i < (pointCount - 3); i++)
    {
        float t = 0.0f;
        rlVector2 p1 = points[i], p2 = points[i + 1], p3 = points[i + 2], p4 = points[i + 3];

        a[0] = (-p1.x + 3.0f*p2.x - 3.0f*p3.x + p4.x)/6.0f;
        a[1] = (3.0f*p1.x - 6.0f*p2.x + 3.0f*p3.x)/6.0f;
        a[2] = (-3.0f*p1.x + 3.0f*p3.x)/6.0f;
        a[3] = (p1.x + 4.0f*p2.x + p3.x)/6.0f;

        b[0] = (-p1.y + 3.0f*p2.y - 3.0f*p3.y + p4.y)/6.0f;
        b[1] = (3.0f*p1.y - 6.0f*p2.y + 3.0f*p3.y)/6.0f;
        b[2] = (-3.0f*p1.y + 3.0f*p3.y)/6.0f;
        b[3] = (p1.y + 4.0f*p2.y + p3.y)/6.0f;

        currentPoint.x = a[3];
        currentPoint.y = b[3];

        if (i == 0) rlDrawCircleV(currentPoint, thick/2.0f, color);   // Draw init line circle-cap

        if (i > 0)
        {
            vertices[0].x = currentPoint.x + dy*size;
            vertices[0].y = currentPoint.y - dx*size;
            vertices[1].x = currentPoint.x - dy*size;
            vertices[1].y = currentPoint.y + dx*size;
        }

        for (int j = 1; j <= SPLINE_SEGMENT_DIVISIONS; j++)
        {
            t = ((float)j)/((float)SPLINE_SEGMENT_DIVISIONS);

            nextPoint.x = a[3] + t*(a[2] + t*(a[1] + t*a[0]));
            nextPoint.y = b[3] + t*(b[2] + t*(b[1] + t*b[0]));

            dy = nextPoint.y - currentPoint.y;
            dx = nextPoint.x - currentPoint.x;
            size = 0.5f*thick/sqrtf(dx*dx+dy*dy);

            if ((i == 0) && (j == 1))
            {
                vertices[0].x = currentPoint.x + dy*size;
                vertices[0].y = currentPoint.y - dx*size;
                vertices[1].x = currentPoint.x - dy*size;
                vertices[1].y = currentPoint.y + dx*size;
            }

            vertices[2*j + 1].x = nextPoint.x - dy*size;
            vertices[2*j + 1].y = nextPoint.y + dx*size;
            vertices[2*j].x = nextPoint.x + dy*size;
            vertices[2*j].y = nextPoint.y - dx*size;

            currentPoint = nextPoint;
        }

        rlDrawTriangleStrip(vertices, 2*SPLINE_SEGMENT_DIVISIONS + 2, color);
    }

    rlDrawCircleV(currentPoint, thick/2.0f, color);   // Draw end line circle-cap
}

// Draw spline: Catmull-Rom, minimum 4 points
void DrawSplineCatmullRom(rlVector2 *points, int pointCount, float thick, rlColor color)
{
    if (pointCount < 4) return;

    float dy = 0.0f;
    float dx = 0.0f;
    float size = 0.0f;

    rlVector2 currentPoint = points[1];
    rlVector2 nextPoint = { 0 };
    rlVector2 vertices[2*SPLINE_SEGMENT_DIVISIONS + 2] = { 0 };

    rlDrawCircleV(currentPoint, thick/2.0f, color);   // Draw init line circle-cap

    for (int i = 0; i < (pointCount - 3); i++)
    {
        float t = 0.0f;
        rlVector2 p1 = points[i], p2 = points[i + 1], p3 = points[i + 2], p4 = points[i + 3];

        if (i > 0)
        {
            vertices[0].x = currentPoint.x + dy*size;
            vertices[0].y = currentPoint.y - dx*size;
            vertices[1].x = currentPoint.x - dy*size;
            vertices[1].y = currentPoint.y + dx*size;
        }

        for (int j = 1; j <= SPLINE_SEGMENT_DIVISIONS; j++)
        {
            t = ((float)j)/((float)SPLINE_SEGMENT_DIVISIONS);

            float q0 = (-1.0f*t*t*t) + (2.0f*t*t) + (-1.0f*t);
            float q1 = (3.0f*t*t*t) + (-5.0f*t*t) + 2.0f;
            float q2 = (-3.0f*t*t*t) + (4.0f*t*t) + t;
            float q3 = t*t*t - t*t;

            nextPoint.x = 0.5f*((p1.x*q0) + (p2.x*q1) + (p3.x*q2) + (p4.x*q3));
            nextPoint.y = 0.5f*((p1.y*q0) + (p2.y*q1) + (p3.y*q2) + (p4.y*q3));

            dy = nextPoint.y - currentPoint.y;
            dx = nextPoint.x - currentPoint.x;
            size = (0.5f*thick)/sqrtf(dx*dx + dy*dy);

            if ((i == 0) && (j == 1))
            {
                vertices[0].x = currentPoint.x + dy*size;
                vertices[0].y = currentPoint.y - dx*size;
                vertices[1].x = currentPoint.x - dy*size;
                vertices[1].y = currentPoint.y + dx*size;
            }

            vertices[2*j + 1].x = nextPoint.x - dy*size;
            vertices[2*j + 1].y = nextPoint.y + dx*size;
            vertices[2*j].x = nextPoint.x + dy*size;
            vertices[2*j].y = nextPoint.y - dx*size;

            currentPoint = nextPoint;
        }

        rlDrawTriangleStrip(vertices, 2*SPLINE_SEGMENT_DIVISIONS + 2, color);
    }

    rlDrawCircleV(currentPoint, thick/2.0f, color);   // Draw end line circle-cap
}

// Draw spline: Quadratic Bezier, minimum 3 points (1 control point): [p1, c2, p3, c4...]
void DrawSplineBezierQuadratic(rlVector2 *points, int pointCount, float thick, rlColor color)
{
    if (pointCount < 3) return;

    for (int i = 0; i < pointCount - 2; i++)
    {
        DrawSplineSegmentBezierQuadratic(points[i], points[i + 1], points[i + 2], thick, color);
    }
}

// Draw spline: Cubic Bezier, minimum 4 points (2 control points): [p1, c2, c3, p4, c5, c6...]
void DrawSplineBezierCubic(rlVector2 *points, int pointCount, float thick, rlColor color)
{
    if (pointCount < 4) return;

    for (int i = 0; i < pointCount - 3; i++)
    {
        DrawSplineSegmentBezierCubic(points[i], points[i + 1], points[i + 2], points[i + 3], thick, color);
    }
}

// Draw spline segment: Linear, 2 points
void DrawSplineSegmentLinear(rlVector2 p1, rlVector2 p2, float thick, rlColor color)
{
    // NOTE: For the linear spline we don't use subdivisions, just a single quad

    rlVector2 delta = { p2.x - p1.x, p2.y - p1.y };
    float length = sqrtf(delta.x*delta.x + delta.y*delta.y);

    if ((length > 0) && (thick > 0))
    {
        float scale = thick/(2*length);

        rlVector2 radius = { -scale*delta.y, scale*delta.x };
        rlVector2 strip[4] = {
            { p1.x - radius.x, p1.y - radius.y },
            { p1.x + radius.x, p1.y + radius.y },
            { p2.x - radius.x, p2.y - radius.y },
            { p2.x + radius.x, p2.y + radius.y }
        };

        rlDrawTriangleStrip(strip, 4, color);
    }
}

// Draw spline segment: B-Spline, 4 points
void DrawSplineSegmentBasis(rlVector2 p1, rlVector2 p2, rlVector2 p3, rlVector2 p4, float thick, rlColor color)
{
    const float step = 1.0f/SPLINE_SEGMENT_DIVISIONS;

    rlVector2 currentPoint = { 0 };
    rlVector2 nextPoint = { 0 };
    float t = 0.0f;

    rlVector2 points[2*SPLINE_SEGMENT_DIVISIONS + 2] = { 0 };

    float a[4] = { 0 };
    float b[4] = { 0 };

    a[0] = (-p1.x + 3*p2.x - 3*p3.x + p4.x)/6.0f;
    a[1] = (3*p1.x - 6*p2.x + 3*p3.x)/6.0f;
    a[2] = (-3*p1.x + 3*p3.x)/6.0f;
    a[3] = (p1.x + 4*p2.x + p3.x)/6.0f;

    b[0] = (-p1.y + 3*p2.y - 3*p3.y + p4.y)/6.0f;
    b[1] = (3*p1.y - 6*p2.y + 3*p3.y)/6.0f;
    b[2] = (-3*p1.y + 3*p3.y)/6.0f;
    b[3] = (p1.y + 4*p2.y + p3.y)/6.0f;

    currentPoint.x = a[3];
    currentPoint.y = b[3];

    for (int i = 0; i <= SPLINE_SEGMENT_DIVISIONS; i++)
    {
        t = step*(float)i;

        nextPoint.x = a[3] + t*(a[2] + t*(a[1] + t*a[0]));
        nextPoint.y = b[3] + t*(b[2] + t*(b[1] + t*b[0]));

        float dy = nextPoint.y - currentPoint.y;
        float dx = nextPoint.x - currentPoint.x;
        float size = (0.5f*thick)/sqrtf(dx*dx + dy*dy);

        if (i == 1)
        {
            points[0].x = currentPoint.x + dy*size;
            points[0].y = currentPoint.y - dx*size;
            points[1].x = currentPoint.x - dy*size;
            points[1].y = currentPoint.y + dx*size;
        }

        points[2*i + 1].x = nextPoint.x - dy*size;
        points[2*i + 1].y = nextPoint.y + dx*size;
        points[2*i].x = nextPoint.x + dy*size;
        points[2*i].y = nextPoint.y - dx*size;

        currentPoint = nextPoint;
    }

    rlDrawTriangleStrip(points, 2*SPLINE_SEGMENT_DIVISIONS+2, color);
}

// Draw spline segment: Catmull-Rom, 4 points
void DrawSplineSegmentCatmullRom(rlVector2 p1, rlVector2 p2, rlVector2 p3, rlVector2 p4, float thick, rlColor color)
{
    const float step = 1.0f/SPLINE_SEGMENT_DIVISIONS;

    rlVector2 currentPoint = p1;
    rlVector2 nextPoint = { 0 };
    float t = 0.0f;

    rlVector2 points[2*SPLINE_SEGMENT_DIVISIONS + 2] = { 0 };

    for (int i = 0; i <= SPLINE_SEGMENT_DIVISIONS; i++)
    {
        t = step*(float)i;

        float q0 = (-1*t*t*t) + (2*t*t) + (-1*t);
        float q1 = (3*t*t*t) + (-5*t*t) + 2;
        float q2 = (-3*t*t*t) + (4*t*t) + t;
        float q3 = t*t*t - t*t;

        nextPoint.x = 0.5f*((p1.x*q0) + (p2.x*q1) + (p3.x*q2) + (p4.x*q3));
        nextPoint.y = 0.5f*((p1.y*q0) + (p2.y*q1) + (p3.y*q2) + (p4.y*q3));

        float dy = nextPoint.y - currentPoint.y;
        float dx = nextPoint.x - currentPoint.x;
        float size = (0.5f*thick)/sqrtf(dx*dx + dy*dy);

        if (i == 1)
        {
            points[0].x = currentPoint.x + dy*size;
            points[0].y = currentPoint.y - dx*size;
            points[1].x = currentPoint.x - dy*size;
            points[1].y = currentPoint.y + dx*size;
        }

        points[2*i + 1].x = nextPoint.x - dy*size;
        points[2*i + 1].y = nextPoint.y + dx*size;
        points[2*i].x = nextPoint.x + dy*size;
        points[2*i].y = nextPoint.y - dx*size;

        currentPoint = nextPoint;
    }

    rlDrawTriangleStrip(points, 2*SPLINE_SEGMENT_DIVISIONS + 2, color);
}

// Draw spline segment: Quadratic Bezier, 2 points, 1 control point
void DrawSplineSegmentBezierQuadratic(rlVector2 p1, rlVector2 c2, rlVector2 p3, float thick, rlColor color)
{
    const float step = 1.0f/SPLINE_SEGMENT_DIVISIONS;

    rlVector2 previous = p1;
    rlVector2 current = { 0 };
    float t = 0.0f;

    rlVector2 points[2*SPLINE_SEGMENT_DIVISIONS + 2] = { 0 };

    for (int i = 1; i <= SPLINE_SEGMENT_DIVISIONS; i++)
    {
        t = step*(float)i;

        float a = powf(1.0f - t, 2);
        float b = 2.0f*(1.0f - t)*t;
        float c = powf(t, 2);

        // NOTE: The easing functions aren't suitable here because they don't take a control point
        current.y = a*p1.y + b*c2.y + c*p3.y;
        current.x = a*p1.x + b*c2.x + c*p3.x;

        float dy = current.y - previous.y;
        float dx = current.x - previous.x;
        float size = 0.5f*thick/sqrtf(dx*dx+dy*dy);

        if (i == 1)
        {
            points[0].x = previous.x + dy*size;
            points[0].y = previous.y - dx*size;
            points[1].x = previous.x - dy*size;
            points[1].y = previous.y + dx*size;
        }

        points[2*i + 1].x = current.x - dy*size;
        points[2*i + 1].y = current.y + dx*size;
        points[2*i].x = current.x + dy*size;
        points[2*i].y = current.y - dx*size;

        previous = current;
    }

    rlDrawTriangleStrip(points, 2*SPLINE_SEGMENT_DIVISIONS + 2, color);
}

// Draw spline segment: Cubic Bezier, 2 points, 2 control points
void DrawSplineSegmentBezierCubic(rlVector2 p1, rlVector2 c2, rlVector2 c3, rlVector2 p4, float thick, rlColor color)
{
    const float step = 1.0f/SPLINE_SEGMENT_DIVISIONS;

    rlVector2 previous = p1;
    rlVector2 current = { 0 };
    float t = 0.0f;

    rlVector2 points[2*SPLINE_SEGMENT_DIVISIONS + 2] = { 0 };

    for (int i = 1; i <= SPLINE_SEGMENT_DIVISIONS; i++)
    {
        t = step*(float)i;

        float a = powf(1.0f - t, 3);
        float b = 3.0f*powf(1.0f - t, 2)*t;
        float c = 3.0f*(1.0f - t)*powf(t, 2);
        float d = powf(t, 3);

        current.y = a*p1.y + b*c2.y + c*c3.y + d*p4.y;
        current.x = a*p1.x + b*c2.x + c*c3.x + d*p4.x;

        float dy = current.y - previous.y;
        float dx = current.x - previous.x;
        float size = 0.5f*thick/sqrtf(dx*dx+dy*dy);

        if (i == 1)
        {
            points[0].x = previous.x + dy*size;
            points[0].y = previous.y - dx*size;
            points[1].x = previous.x - dy*size;
            points[1].y = previous.y + dx*size;
        }

        points[2*i + 1].x = current.x - dy*size;
        points[2*i + 1].y = current.y + dx*size;
        points[2*i].x = current.x + dy*size;
        points[2*i].y = current.y - dx*size;

        previous = current;
    }

    rlDrawTriangleStrip(points, 2*SPLINE_SEGMENT_DIVISIONS + 2, color);
}

// Get spline point for a given t [0.0f .. 1.0f], Linear
rlVector2 GetSplinePointLinear(rlVector2 startPos, rlVector2 endPos, float t)
{
    rlVector2 point = { 0 };

    point.x = startPos.x*(1.0f - t) + endPos.x*t;
    point.y = startPos.y*(1.0f - t) + endPos.y*t;

    return point;
}

// Get spline point for a given t [0.0f .. 1.0f], B-Spline
rlVector2 GetSplinePointBasis(rlVector2 p1, rlVector2 p2, rlVector2 p3, rlVector2 p4, float t)
{
    rlVector2 point = { 0 };

    float a[4] = { 0 };
    float b[4] = { 0 };

    a[0] = (-p1.x + 3*p2.x - 3*p3.x + p4.x)/6.0f;
    a[1] = (3*p1.x - 6*p2.x + 3*p3.x)/6.0f;
    a[2] = (-3*p1.x + 3*p3.x)/6.0f;
    a[3] = (p1.x + 4*p2.x + p3.x)/6.0f;

    b[0] = (-p1.y + 3*p2.y - 3*p3.y + p4.y)/6.0f;
    b[1] = (3*p1.y - 6*p2.y + 3*p3.y)/6.0f;
    b[2] = (-3*p1.y + 3*p3.y)/6.0f;
    b[3] = (p1.y + 4*p2.y + p3.y)/6.0f;

    point.x = a[3] + t*(a[2] + t*(a[1] + t*a[0]));
    point.y = b[3] + t*(b[2] + t*(b[1] + t*b[0]));

    return point;
}

// Get spline point for a given t [0.0f .. 1.0f], Catmull-Rom
rlVector2 GetSplinePointCatmullRom(rlVector2 p1, rlVector2 p2, rlVector2 p3, rlVector2 p4, float t)
{
    rlVector2 point = { 0 };

    float q0 = (-1*t*t*t) + (2*t*t) + (-1*t);
    float q1 = (3*t*t*t) + (-5*t*t) + 2;
    float q2 = (-3*t*t*t) + (4*t*t) + t;
    float q3 = t*t*t - t*t;

    point.x = 0.5f*((p1.x*q0) + (p2.x*q1) + (p3.x*q2) + (p4.x*q3));
    point.y = 0.5f*((p1.y*q0) + (p2.y*q1) + (p3.y*q2) + (p4.y*q3));

    return point;
}

// Get spline point for a given t [0.0f .. 1.0f], Quadratic Bezier
rlVector2 GetSplinePointBezierQuad(rlVector2 startPos, rlVector2 controlPos, rlVector2 endPos, float t)
{
    rlVector2 point = { 0 };

    float a = powf(1.0f - t, 2);
    float b = 2.0f*(1.0f - t)*t;
    float c = powf(t, 2);

    point.y = a*startPos.y + b*controlPos.y + c*endPos.y;
    point.x = a*startPos.x + b*controlPos.x + c*endPos.x;

    return point;
}

// Get spline point for a given t [0.0f .. 1.0f], Cubic Bezier
rlVector2 GetSplinePointBezierCubic(rlVector2 startPos, rlVector2 startControlPos, rlVector2 endControlPos, rlVector2 endPos, float t)
{
    rlVector2 point = { 0 };

    float a = powf(1.0f - t, 3);
    float b = 3.0f*powf(1.0f - t, 2)*t;
    float c = 3.0f*(1.0f - t)*powf(t, 2);
    float d = powf(t, 3);

    point.y = a*startPos.y + b*startControlPos.y + c*endControlPos.y + d*endPos.y;
    point.x = a*startPos.x + b*startControlPos.x + c*endControlPos.x + d*endPos.x;

    return point;
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Collision Detection functions
//----------------------------------------------------------------------------------

// Check if point is inside rectangle
bool rlCheckCollisionPointRec(rlVector2 point, rlRectangle rec)
{
    bool collision = false;

    if ((point.x >= rec.x) && (point.x < (rec.x + rec.width)) && (point.y >= rec.y) && (point.y < (rec.y + rec.height))) collision = true;

    return collision;
}

// Check if point is inside circle
bool rlCheckCollisionPointCircle(rlVector2 point, rlVector2 center, float radius)
{
    bool collision = false;

    collision = rlCheckCollisionCircles(point, 0, center, radius);

    return collision;
}

// Check if point is inside a triangle defined by three points (p1, p2, p3)
bool rlCheckCollisionPointTriangle(rlVector2 point, rlVector2 p1, rlVector2 p2, rlVector2 p3)
{
    bool collision = false;

    float alpha = ((p2.y - p3.y)*(point.x - p3.x) + (p3.x - p2.x)*(point.y - p3.y)) /
                  ((p2.y - p3.y)*(p1.x - p3.x) + (p3.x - p2.x)*(p1.y - p3.y));

    float beta = ((p3.y - p1.y)*(point.x - p3.x) + (p1.x - p3.x)*(point.y - p3.y)) /
                 ((p2.y - p3.y)*(p1.x - p3.x) + (p3.x - p2.x)*(p1.y - p3.y));

    float gamma = 1.0f - alpha - beta;

    if ((alpha > 0) && (beta > 0) && (gamma > 0)) collision = true;

    return collision;
}

// Check if point is within a polygon described by array of vertices
// NOTE: Based on http://jeffreythompson.org/collision-detection/poly-point.php
bool rlCheckCollisionPointPoly(rlVector2 point, rlVector2 *points, int pointCount)
{
    bool collision = false;

    if (pointCount > 2)
    {
        for (int i = 0; i < pointCount - 1; i++)
        {
            rlVector2 vc = points[i];
            rlVector2 vn = points[i + 1];

            if ((((vc.y >= point.y) && (vn.y < point.y)) || ((vc.y < point.y) && (vn.y >= point.y))) &&
                 (point.x < ((vn.x - vc.x)*(point.y - vc.y)/(vn.y - vc.y) + vc.x))) collision = !collision;
        }
    }

    return collision;
}

// Check collision between two rectangles
bool rlCheckCollisionRecs(rlRectangle rec1, rlRectangle rec2)
{
    bool collision = false;

    if ((rec1.x < (rec2.x + rec2.width) && (rec1.x + rec1.width) > rec2.x) &&
        (rec1.y < (rec2.y + rec2.height) && (rec1.y + rec1.height) > rec2.y)) collision = true;

    return collision;
}

// Check collision between two circles
bool rlCheckCollisionCircles(rlVector2 center1, float radius1, rlVector2 center2, float radius2)
{
    bool collision = false;

    float dx = center2.x - center1.x;      // X distance between centers
    float dy = center2.y - center1.y;      // Y distance between centers

    float distance = sqrtf(dx*dx + dy*dy); // Distance between centers

    if (distance <= (radius1 + radius2)) collision = true;

    return collision;
}

// Check collision between circle and rectangle
// NOTE: Reviewed version to take into account corner limit case
bool rlCheckCollisionCircleRec(rlVector2 center, float radius, rlRectangle rec)
{
    bool collision = false;

    int recCenterX = (int)(rec.x + rec.width/2.0f);
    int recCenterY = (int)(rec.y + rec.height/2.0f);

    float dx = fabsf(center.x - (float)recCenterX);
    float dy = fabsf(center.y - (float)recCenterY);

    if (dx > (rec.width/2.0f + radius)) { return false; }
    if (dy > (rec.height/2.0f + radius)) { return false; }

    if (dx <= (rec.width/2.0f)) { return true; }
    if (dy <= (rec.height/2.0f)) { return true; }

    float cornerDistanceSq = (dx - rec.width/2.0f)*(dx - rec.width/2.0f) +
                             (dy - rec.height/2.0f)*(dy - rec.height/2.0f);

    collision = (cornerDistanceSq <= (radius*radius));

    return collision;
}

// Check the collision between two lines defined by two points each, returns collision point by reference
bool rlCheckCollisionLines(rlVector2 startPos1, rlVector2 endPos1, rlVector2 startPos2, rlVector2 endPos2, rlVector2 *collisionPoint)
{
    bool collision = false;

    float div = (endPos2.y - startPos2.y)*(endPos1.x - startPos1.x) - (endPos2.x - startPos2.x)*(endPos1.y - startPos1.y);

    if (fabsf(div) >= FLT_EPSILON)
    {
        collision = true;

        float xi = ((startPos2.x - endPos2.x)*(startPos1.x*endPos1.y - startPos1.y*endPos1.x) - (startPos1.x - endPos1.x)*(startPos2.x*endPos2.y - startPos2.y*endPos2.x))/div;
        float yi = ((startPos2.y - endPos2.y)*(startPos1.x*endPos1.y - startPos1.y*endPos1.x) - (startPos1.y - endPos1.y)*(startPos2.x*endPos2.y - startPos2.y*endPos2.x))/div;

        if (((fabsf(startPos1.x - endPos1.x) > FLT_EPSILON) && (xi < fminf(startPos1.x, endPos1.x) || (xi > fmaxf(startPos1.x, endPos1.x)))) ||
            ((fabsf(startPos2.x - endPos2.x) > FLT_EPSILON) && (xi < fminf(startPos2.x, endPos2.x) || (xi > fmaxf(startPos2.x, endPos2.x)))) ||
            ((fabsf(startPos1.y - endPos1.y) > FLT_EPSILON) && (yi < fminf(startPos1.y, endPos1.y) || (yi > fmaxf(startPos1.y, endPos1.y)))) ||
            ((fabsf(startPos2.y - endPos2.y) > FLT_EPSILON) && (yi < fminf(startPos2.y, endPos2.y) || (yi > fmaxf(startPos2.y, endPos2.y))))) collision = false;

        if (collision && (collisionPoint != 0))
        {
            collisionPoint->x = xi;
            collisionPoint->y = yi;
        }
    }

    return collision;
}

// Check if point belongs to line created between two points [p1] and [p2] with defined margin in pixels [threshold]
bool rlCheckCollisionPointLine(rlVector2 point, rlVector2 p1, rlVector2 p2, int threshold)
{
    bool collision = false;

    float dxc = point.x - p1.x;
    float dyc = point.y - p1.y;
    float dxl = p2.x - p1.x;
    float dyl = p2.y - p1.y;
    float cross = dxc*dyl - dyc*dxl;

    if (fabsf(cross) < (threshold*fmaxf(fabsf(dxl), fabsf(dyl))))
    {
        if (fabsf(dxl) >= fabsf(dyl)) collision = (dxl > 0)? ((p1.x <= point.x) && (point.x <= p2.x)) : ((p2.x <= point.x) && (point.x <= p1.x));
        else collision = (dyl > 0)? ((p1.y <= point.y) && (point.y <= p2.y)) : ((p2.y <= point.y) && (point.y <= p1.y));
    }

    return collision;
}

// Get collision rectangle for two rectangles collision
rlRectangle rlGetCollisionRec(rlRectangle rec1, rlRectangle rec2)
{
    rlRectangle overlap = { 0 };

    float left = (rec1.x > rec2.x)? rec1.x : rec2.x;
    float right1 = rec1.x + rec1.width;
    float right2 = rec2.x + rec2.width;
    float right = (right1 < right2)? right1 : right2;
    float top = (rec1.y > rec2.y)? rec1.y : rec2.y;
    float bottom1 = rec1.y + rec1.height;
    float bottom2 = rec2.y + rec2.height;
    float bottom = (bottom1 < bottom2)? bottom1 : bottom2;

    if ((left < right) && (top < bottom))
    {
        overlap.x = left;
        overlap.y = top;
        overlap.width = right - left;
        overlap.height = bottom - top;
    }

    return overlap;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------

// Cubic easing in-out
// NOTE: Used by rlDrawLineBezier() only
static float EaseCubicInOut(float t, float b, float c, float d)
{
    if ((t /= 0.5f*d) < 1) return 0.5f*c*t*t*t + b;

    t -= 2;

    return 0.5f*c*(t*t*t + 2.0f) + b;
}

RL_NS_END

#endif      // RL_SUPPORT_MODULE_RSHAPES
