#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <cmath>

#include "Vector3f.h"

inline Vector3f replaceBad(const Vector3f &v) {
    if (std::isnan(v.x()) | std::isnan(v.y()) | std::isnan(v.z()) |
        std::isinf(v.x()) | std::isinf(v.y()) | std::isinf(v.z())) {
        return Vector3f(0.0f);
    }
    return v;
}

inline Vector3f hemiToWorld(const Vector3f &dir, const Vector3f &normal) {
    Vector3f tangent, bitangent;
    if (fabs(normal.x()) > fabs(normal.y())) {
        tangent = Vector3f(normal.z(), 0, -normal.x()).normalized();
    } else {
        tangent = Vector3f(0, -normal.z(), normal.y()).normalized();
    }
    bitangent = Vector3f::cross(normal, tangent);
    return dir.x() * tangent + dir.y() * bitangent + dir.z() * normal;
}

// ====================================================================
// ====================================================================

inline void showProgress(float progress) {
    int barWidth = 70;
    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
}

#endif // UTILS_H
