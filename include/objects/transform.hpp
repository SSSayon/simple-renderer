#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <cstdlib>
#include <iostream>
#include <vecmath.h>
#include "Vector3f.h"
#include "accel/aabb.hpp"
#include "object3d.hpp"

// transforms a 3D point using a matrix, returning a 3D point
static Vector3f transformPoint(const Matrix4f &mat, const Vector3f &point) {
    return (mat * Vector4f(point, 1)).xyz();
}

// transform a 3D direction using a matrix, returning a direction
static Vector3f transformDirection(const Matrix4f &mat, const Vector3f &dir) {
    return (mat * Vector4f(dir, 0)).xyz();
}

class Transform : public Object3D {
public:
    Transform() {}

    Transform(const Matrix4f &m, Object3D *obj) : o(obj) {
        M = m;
        transform = m.inverse();
    }

    ~Transform() {
    }

    bool intersect(const Ray &r, Hit &h, float tmin) const override {
        Vector3f trSource = transformPoint(transform, r.getOrigin());
        Vector3f trDirection = transformDirection(transform, r.getDirection());
        Ray tr(trSource, trDirection);
        bool inter = o->intersect(tr, h, tmin);
        if (inter) {
            h.set(h.getT(), h.getMaterial(), transformDirection(transform.transposed(), h.getNormal()).normalized());
        }
        return inter;
    }
    bool intersect(const Ray &r, Hit &h, float tmin, float &pdf) override {
        std::cerr << "Transform::intersect method (with pdf) should NOT be called!" << std::endl;
        exit(1);
    }

    Vector3f samplePoint(std::mt19937 &rng, float &pdf) override {
        std::cerr << "Transform::samplePoint method should NOT be called!" << std::endl;
        exit(1);
    }

    AABB *getAABB() const override {
        if (aabb != nullptr) return aabb;

        if (o->isPlane()) {
            aabb = o->getAABB();
            return aabb;
        }

        AABB *aabb = o->getAABB();
        Vector3f pMin = aabb->pMin;
        Vector3f pMax = aabb->pMax;

        Vector3f p[8];
        p[0] = transformPoint(M, pMin);
        p[1] = transformPoint(M, Vector3f(pMin.x(), pMin.y(), pMax.z()));
        p[2] = transformPoint(M, Vector3f(pMin.x(), pMax.y(), pMin.z()));
        p[3] = transformPoint(M, Vector3f(pMin.x(), pMax.y(), pMax.z()));
        p[4] = transformPoint(M, Vector3f(pMax.x(), pMin.y(), pMin.z()));
        p[5] = transformPoint(M, Vector3f(pMax.x(), pMin.y(), pMax.z()));
        p[6] = transformPoint(M, Vector3f(pMax.x(), pMax.y(), pMin.z()));
        p[7] = transformPoint(M, pMax);

        pMin = pMax = p[0];
        for (int i = 1; i < 8; i++) {
            pMin = Vector3f::min(pMin, p[i]);
            pMax = Vector3f::max(pMax, p[i]);
        }

        aabb = new AABB(pMin, pMax);
        return aabb;
    }

    // assume only one AABB
    std::vector<const Object3D*> getObjects() const override {
        return {this};
    }

    bool isPlane() const override {
        return o->isPlane();
    }

protected:
    Object3D *o; // un-transformed object
    Matrix4f M, transform;
};

#endif //TRANSFORM_H
