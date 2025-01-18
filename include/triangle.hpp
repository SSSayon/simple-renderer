#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Vector3f.h"
#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

const static float EPStriangle = 1e-8f;

class Triangle: public Object3D {

public:
    Triangle() = delete;

    // a b c are three vertex positions of the triangle
    Triangle(const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m) : Object3D(m) {
        v1 = a;
        v2 = b;
        v3 = c;
        normal = Vector3f::cross(v2 - v1, v3 - v1).normalized();
    }

    bool intersect(const Ray& r,  Hit& h , float tmin) override {
        Vector3f o = r.getOrigin();
        Vector3f d = r.getDirection();

        Vector3f e1 = v2 - v1;
        Vector3f e2 = v3 - v1;

        Vector3f d_cross_e2 = Vector3f::cross(d, e2);
        float det = Vector3f::dot(e1, d_cross_e2);

        if (fabs(det) < EPStriangle) return false;

        float inv_det = 1 / det;

        Vector3f o_minus_v1 = o - v1;
        float u = inv_det * Vector3f::dot(o_minus_v1, d_cross_e2);
        if (u < 0 || u > 1) return false;

        Vector3f o_minus_v1_cross_e1 = Vector3f::cross(o_minus_v1, e1);
        float v = inv_det * Vector3f::dot(d, o_minus_v1_cross_e1);
        if (v < 0 || u + v > 1) return false;

        float t = inv_det * Vector3f::dot(e2, o_minus_v1_cross_e1);
        if (t > h.getT() || t < tmin) return false;

        h.set(t, material, (Vector3f::dot(normal, d) > 0) ? -normal : normal);
        return true;
    }

    Vector3f normal;

protected:
    Vector3f v1, v2, v3;
};

#endif // TRIANGLE_H
