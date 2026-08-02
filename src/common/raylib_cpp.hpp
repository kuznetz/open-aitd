#pragma once
namespace raylib { 
	#include <raylib.h>
	#include <raymath.h>
	#include <rcamera.h>
	#include <rlgl.h>
	#include <external/stb_image_resize2.h>
	#include <external/cgltf.h>
	static Model LoadGLTF(const char *fileName);

	static inline Matrix MatrixRotateYZX(Vector3 angle)
	{
    float sy = sinf(angle.y), cy = cosf(angle.y);
    float sz = sinf(angle.z), cz = cosf(angle.z);
    float sx = sinf(angle.x), cx = cosf(angle.x);

    Matrix result = { 0 };

    // 1
    result.m0  = cy * cz;
    result.m1  = -cy * sz * cx + sy * sx;
    result.m2  =  cy * sz * sx + sy * cx;
    result.m3  = 0.0f;
    // 2
    result.m4  = sz;
    result.m5  = cz * cx;
    result.m6  = -cz * sx;
    result.m7  = 0.0f;
    // 3
    result.m8  = -sy * cz;
    result.m9  =  sy * sz * cx + cy * sx;
    result.m10 = -sy * sz * sx + cy * cx;
    result.m11 = 0.0f;
    // 4
    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = 0.0f;
    result.m15 = 1.0f;

    return result;
	}

}