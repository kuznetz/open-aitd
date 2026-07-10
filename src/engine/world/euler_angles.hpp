#pragma once
#include "../../raylib-cpp/raylib-cpp.h"

namespace openAITD {

  using namespace raylib;
  class EulerAngles : public Vector3 {
  public:

    EulerAngles() : Vector3{0, 0, 0} {}
    EulerAngles(float x, float y, float z) : Vector3{x, y, z} {}

    static float NormalizeAngle(float angle) {
        angle = fmod(angle, 2*PI);
        if (angle > PI) angle -= 2*PI;
        if (angle < -PI) angle += 2*PI;
        return angle;
    }

    static float AngleDifference(float a, float b) {
        float diff = fmod(b - a, 2.0f * PI);
        if (diff > PI) diff -= 2.0f * PI;
        if (diff < -PI) diff += 2.0f * PI;
        return diff;
    }    

    EulerAngles GetNormalized() const {
        EulerAngles result;
        result.x = NormalizeAngle(x);
        result.y = NormalizeAngle(y);
        result.z = NormalizeAngle(z);
        return result;
    }

    bool IsNearlyEqual(const EulerAngles& other, const float epsilon = 0.001f) const {
        const EulerAngles a = GetNormalized();
        const EulerAngles b = other.GetNormalized();
        return (std::fabs(AngleDifference(a.x, b.x)) < epsilon) &&
               (std::fabs(AngleDifference(a.y, b.y)) < epsilon) &&
               (std::fabs(AngleDifference(a.z, b.z)) < epsilon);
    }    

  };

}