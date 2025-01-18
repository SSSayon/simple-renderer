#ifndef PLANE_H
#define PLANE_H

#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

const static float EPSplane = 1e-8f;

class Plane : public Object3D {
public:
    Plane() {
        // do nothing
    }

    Plane(const Vector3f &normal, float d, Material *m) : Object3D(m) {
        this->normal = normal.normalized();
        this->d = d;
        // this->origin = normal * d;
    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) override {

        Vector3f ori = r.getOrigin();
        Vector3f dir = r.getDirection();

        float dir_dot_normal = Vector3f::dot(dir, normal);
        if (fabs(dir_dot_normal) < EPSplane) return false;

        float t = (d - Vector3f::dot(ori, normal)) / dir_dot_normal;
        if (t > h.getT() || t < tmin) return false;

        h.set(t, material, (dir_dot_normal > 0) ? -normal : normal);
        return true;
    }

protected:
    // Vector3f origin;
    Vector3f normal;
    float d;
};

#endif // PLANE_H
