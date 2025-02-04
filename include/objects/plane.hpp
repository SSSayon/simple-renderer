#ifndef PLANE_H
#define PLANE_H

#include "accel/aabb.hpp"
#include "object3d.hpp"
#include <vecmath.h>
#include <cmath>

class Plane : public Object3D {
public:
    Plane() = delete;

    // plane: <p, normal> = d
    Plane(const Vector3f &normal, float d, Material *m) : Object3D(m) {
        this->normal = normal.normalized();
        this->d = d;
    }

    ~Plane() override = default;

    bool intersect(const Ray &r, Hit &h, float tmin) const override {

        Vector3f ori = r.getOrigin();
        Vector3f dir = r.getDirection();

        float dir_dot_normal = Vector3f::dot(dir, normal);
        if (fabs(dir_dot_normal) < EPSplane) return false;

        float t = (d - Vector3f::dot(ori, normal)) / dir_dot_normal;
        if (t > h.getT() || t < tmin) return false;

        h.set(t, material, (dir_dot_normal > 0) ? -normal : normal);
        return true;
    }
    bool intersect(const Ray &r, Hit &h, float tmin, float &pdf) override {
        std::cerr << "Plane::intersect method (with pdf) should NOT be called!" << std::endl;
        exit(1);
    }

    Vector3f samplePoint(std::mt19937 &rng, float &pdf) override {
        std::cerr << "Plane::samplePoint method should NOT be called!" << std::endl;
        exit(1);
    }

    AABB *getAABB() const override {
        if (aabb != nullptr) return aabb;
        aabb = new AABB(Vector3f::MINUS_INF, Vector3f::INF);
        return aabb;
    }

    std::vector<const Object3D*> getObjects() const override {
        return {this};
    }

    bool isPlane() const override { return true; }

protected:
    Vector3f normal;
    float d;

    static constexpr float EPSplane = 1e-8f;
};

#endif // PLANE_H
