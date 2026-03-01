
#pragma once
#include <vector>
#include <string>
#include <cstring>
#include "../raylib-cpp/raylib-cpp.h"
#include "./bounds.hpp"

using namespace raylib;
namespace openAITD {
  
    inline void Vector3TransformRef(Vector3& result, const Matrix& mat)
    {
        float x = result.x;
        float y = result.y;
        float z = result.z;
        result.x = mat.m0 * x + mat.m4 * y + mat.m8 * z + mat.m12;
        result.y = mat.m1 * x + mat.m5 * y + mat.m9 * z + mat.m13;
        result.z = mat.m2 * x + mat.m6 * y + mat.m10 * z + mat.m14;
    }

    inline void MatrixInvertRef(Matrix& result)
    {
        Matrix mat = result;
        float a00 = mat.m0, a01 = mat.m1, a02 = mat.m2, a03 = mat.m3;
        float a10 = mat.m4, a11 = mat.m5, a12 = mat.m6, a13 = mat.m7;
        float a20 = mat.m8, a21 = mat.m9, a22 = mat.m10, a23 = mat.m11;
        float a30 = mat.m12, a31 = mat.m13, a32 = mat.m14, a33 = mat.m15;

        float b00 = a00 * a11 - a01 * a10;
        float b01 = a00 * a12 - a02 * a10;
        float b02 = a00 * a13 - a03 * a10;
        float b03 = a01 * a12 - a02 * a11;
        float b04 = a01 * a13 - a03 * a11;
        float b05 = a02 * a13 - a03 * a12;
        float b06 = a20 * a31 - a21 * a30;
        float b07 = a20 * a32 - a22 * a30;
        float b08 = a20 * a33 - a23 * a30;
        float b09 = a21 * a32 - a22 * a31;
        float b10 = a21 * a33 - a23 * a31;
        float b11 = a22 * a33 - a23 * a32;

        float invDet = 1.0f / (b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06);

        result.m0 = (a11 * b11 - a12 * b10 + a13 * b09) * invDet;
        result.m1 = (-a01 * b11 + a02 * b10 - a03 * b09) * invDet;
        result.m2 = (a31 * b05 - a32 * b04 + a33 * b03) * invDet;
        result.m3 = (-a21 * b05 + a22 * b04 - a23 * b03) * invDet;
        result.m4 = (-a10 * b11 + a12 * b08 - a13 * b07) * invDet;
        result.m5 = (a00 * b11 - a02 * b08 + a03 * b07) * invDet;
        result.m6 = (-a30 * b05 + a32 * b02 - a33 * b01) * invDet;
        result.m7 = (a20 * b05 - a22 * b02 + a23 * b01) * invDet;
        result.m8 = (a10 * b10 - a11 * b08 + a13 * b06) * invDet;
        result.m9 = (-a00 * b10 + a01 * b08 - a03 * b06) * invDet;
        result.m10 = (a30 * b04 - a31 * b02 + a33 * b00) * invDet;
        result.m11 = (-a20 * b04 + a21 * b02 - a23 * b00) * invDet;
        result.m12 = (-a10 * b09 + a11 * b07 - a12 * b06) * invDet;
        result.m13 = (a00 * b09 - a01 * b07 + a02 * b06) * invDet;
        result.m14 = (-a30 * b03 + a31 * b01 - a32 * b00) * invDet;
        result.m15 = (a20 * b03 - a21 * b01 + a22 * b00) * invDet;
    }

    inline void MatrixTransposeRef(Matrix& mat)
    {
        std::swap(mat.m1, mat.m4);
        std::swap(mat.m2, mat.m8);
        std::swap(mat.m3, mat.m12);
        std::swap(mat.m6, mat.m9);
        std::swap(mat.m7, mat.m13);
        std::swap(mat.m11, mat.m14);
    }

}