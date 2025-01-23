#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Vector2f.h"
#include "Vector3f.h"
#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

class Triangle: public Object3D {

public:
    Triangle() = delete;

    // a b c are three vertex positions of the triangle
    Triangle(const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m,
             const Vector3f& na = Vector3f::ZERO, const Vector3f& nb = Vector3f::ZERO, const Vector3f& nc = Vector3f::ZERO,
             const Vector2f& ta = Vector2f::ZERO, const Vector2f& tb = Vector2f::ZERO, const Vector2f& tc = Vector2f::ZERO): 
        Object3D(m) 
    {
        v1 = a; v2 = b; v3 = c;
        vn1 = na; vn2 = nb; vn3 = nc;
        uv1 = ta; uv2 = tb; uv3 = tc;

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

        Vector3f n = normal;
        if (vn1 != Vector3f::ZERO) {
            n = (1 - u - v) * vn1 + u * vn2 + v * vn3;
            n.normalize();
        }

        h.set(t, material, n);
        return true;
    }

protected:
    
    Vector3f v1, v2, v3;    // vertex
    Vector3f vn1, vn2, vn3; // vertex normal
    Vector2f uv1, uv2, uv3; // vertex texture coordinate

    Vector3f normal;        // face normal

    static constexpr float EPStriangle = 1e-8f;
};

#endif // TRIANGLE_H
