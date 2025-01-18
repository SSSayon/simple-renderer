#ifndef SPHERE_H
#define SPHERE_H

#include "Vector3f.h"
#include "object3d.hpp"
#include <utility>
#include <vecmath.h>
#include <cmath>

class Sphere : public Object3D {
public:
    Sphere() {
        // do nothing, please
    }

    Sphere(const Vector3f &center, float radius, Material *material) : Object3D(material) {
        this->center = center;
        this->radius = radius;
    }

    ~Sphere() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {

        Vector3f o = r.getOrigin();
        Vector3f d = r.getDirection();
        
        float a = Vector3f::dot(d, d);
        float b = 2 * Vector3f::dot(d, o - center);
        float c = Vector3f::dot(o - center, o - center) - radius * radius;

        float delta = b * b - 4 * a * c;
        if (delta < 0) return false;

        float root_delta = std::sqrt(delta);
        float q = (b < 0) ? -0.5 * (b - root_delta) : -0.5 * (b + root_delta); 
        float t0 = q / a;
        float t1 = c / q;
        if (t0 > t1) std::swap(t0, t1);
                        
        if (t1 < 0) return false;
        float t = (t0 < 0) ? t1 : t0;
        if (t > h.getT() || t < tmin) return false;

        Vector3f n = (r.pointAtParameter(t) - center).normalized();
        h.set(t, material, n);
        return true;
    }

protected:
    float radius;
    Vector3f center;
};


#endif // SPHERE_H
