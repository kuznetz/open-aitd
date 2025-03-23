#pragma once
#include "rlstencil.h"
#include "raymath.h"
#include <raylib.h>
#include <rlgl.h>

//PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA

void BeginMaskCamera( float z=0 )
{
    rlDrawRenderBatchActive();      // Update and draw internal render batch
    rlMatrixMode(RL_PROJECTION);    // Switch to projection matrix
    rlPushMatrix();                 // Save previous matrix
    rlLoadIdentity();               // Reset current matrix (projection)
    rlOrtho(0, 320, 200, 0, rlGetCullDistanceNear(), rlGetCullDistanceFar());
    rlMatrixMode(RL_MODELVIEW);     // Switch back to modelview matrix
    rlLoadIdentity();               // Reset current matrix (modelview)
    rlTranslatef(0, 0, z);
}

// Ends 3D mode and returns to default 2D orthographic mode
void EndMaskCamera(Camera3D camera)
{
    rlDrawRenderBatchActive();      // Update and draw internal render batch
    rlMatrixMode(RL_PROJECTION);    // Switch to projection matrix
    rlPopMatrix();                  // Restore previous matrix (projection) from matrix stack
    rlMatrixMode(RL_MODELVIEW);     // Switch back to modelview matrix
    rlLoadIdentity();               // Reset current matrix (modelview)
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);
    rlMultMatrixf(MatrixToFloat(matView));  // Multiply modelview matrix by view matrix 
}

void DrawMask(Texture2D texture, Rectangle dest, float z)
{
    rlSetTexture(texture.id);
    rlBegin(RL_QUADS);
    rlColor4ub(255, 255, 255, 255);
    rlNormal3f(0.0f, 0.0f, 1.0f);
    // Top-left corner for texture and quad
    rlTexCoord2f(0, 0);
    rlVertex3f(dest.x, dest.y, z);
    // Bottom-left
    rlTexCoord2f(0, 1);
    rlVertex3f(dest.x, dest.y + dest.height, z);
    // Bottom-right corner for texture and quad
    rlTexCoord2f(1, 1);
    rlVertex3f(dest.x + dest.width, dest.y + dest.height, z);
    // Top-right corner for texture and quad
    rlTexCoord2f(1, 0);
    rlVertex3f(dest.x + dest.width, dest.y, z);
    rlEnd();
    rlSetTexture(0);
}