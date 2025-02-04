#ifndef SPHERE_H
#define SPHERE_H

#include "Vector3f.h"
#include "object3d.hpp"
#include <utility>
#include <vecmath.h>

class Sphere : public Object3D {
public:
    Sphere() = delete;

    Sphere(const Vector3f &center, float radius, Material *material) : Object3D(material) {
        this->center = center;
        this->radius = radius;
    }

    ~Sphere() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) const override {

        Vector3f o = r.getOrigin();
        Vector3f d = r.getDirection();
        
        Vector3f o_minus_center = o - center;
        float a = Vector3f::dot(d, d);
        float b = 2 * Vector3f::dot(d, o_minus_center);
        float c = Vector3f::dot(o_minus_center, o_minus_center) - radius * radius;

        float delta = b * b - 4 * a * c;
        if (delta < EPSsphere) return false;

        float root_delta = sqrtf(delta);
        float q = (b < 0) ? -0.5 * (b - root_delta) : -0.5 * (b + root_delta); 
        float t0 = q / a;
        float t1 = c / q;
        if (t0 > t1) std::swap(t0, t1);

        if (t0 < tmin) t0 = t1;
        if (t0 < tmin || t0 > h.getT()) return false;

        h.set(t0, material, (r.pointAtParameter(t0) - center).normalized());
        return true;
    }
    bool intersect(const Ray &r, Hit &h, float tmin, float &pdf) override {
        std::cerr << "Sphere::intersect method (with pdf) should NOT be called!" << std::endl;
        exit(1);
    }

    Vector3f samplePoint(std::mt19937 &rng, float &pdf) override {
        std::cerr << "Sphere::samplePoint method should NOT be called!" << std::endl;
        exit(1);
    }

    AABB *getAABB() const override {
        if (aabb != nullptr) return aabb;

        Vector3f pMin = center - Vector3f(radius, radius, radius);
        Vector3f pMax = center + Vector3f(radius, radius, radius);

        aabb = new AABB(pMin, pMax);
        return aabb;
    }

    std::vector<const Object3D*> getObjects() const override {
        return {this};
    }

protected:
    float radius;
    Vector3f center;

    static constexpr float EPSsphere = 1e-4;
};


#endif // SPHERE_H
