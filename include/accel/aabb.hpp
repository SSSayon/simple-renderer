#ifndef AABB_H
#define AABB_H

#include "Vector3f.h"
#include "ray.hpp"

class AABB {
public:
    Vector3f pMin, pMax;

    AABB() {
        pMin = Vector3f::INF;
        pMax = Vector3f::MINUS_INF;
    }
    AABB(const Vector3f& _pMin, const Vector3f& _pMax) : pMin(_pMin), pMax(_pMax) {}
    AABB(const Vector3f& p) : pMin(p), pMax(p) {}
    AABB(const AABB &aabb) : pMin(aabb.pMin), pMax(aabb.pMax) {}

    Vector3f centroid() const { 
        return 0.5f * (pMin + pMax); 
    }

    float surfaceArea() const {
        Vector3f d = pMax - pMin;
        return 2 * (d.x() * d.y() + d.y() * d.z() + d.z() * d.x());
    }

    void union_(const AABB &aabb) {
        pMin = Vector3f::min(pMin, aabb.pMin);
        pMax = Vector3f::max(pMax, aabb.pMax);
    }

    void union_(const Vector3f &p) {
        pMin = Vector3f::min(pMin, p);
        pMax = Vector3f::max(pMax, p);
    }

    friend AABB *union_(const AABB &a, const AABB &b) {
        AABB *ret = new AABB(a);
        ret->union_(b);
        return ret;
    }

    // Slab method, see https://tavianator.com/2011/ray_box.html, https://tavianator.com/2015/ray_box_nan.html
    bool intersectAABB(const Ray &r, float tmin, float tmax) const {
        float t1 = (pMin.x() - r.getOrigin().x()) * r.getInvDirection().x();
        float t2 = (pMax.x() - r.getOrigin().x()) * r.getInvDirection().x();
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
        if (tmax <= tmin) return false;

        t1 = (pMin.y()  - r.getOrigin().y()) * r.getInvDirection().y();
        t2 = (pMax.y() - r.getOrigin().y()) * r.getInvDirection().y();
        tmin = std::max(tmin, std::min(std::min(t1, t2), tmax)); // handle NaN
        tmax = std::min(tmax, std::max(std::max(t1, t2), tmin));
        if (tmax <= tmin) return false;

        t1 = (pMin.z()- r.getOrigin().z()) * r.getInvDirection().z();
        t2 = (pMax.z() - r.getOrigin().z()) * r.getInvDirection().z();
        tmin = std::max(tmin, std::min(std::min(t1, t2), tmax));
        tmax = std::min(tmax, std::max(std::max(t1, t2), tmin));
        return tmax > tmin;
    }
    static constexpr float EPSaabb = 1e-6f;

};

#endif // AABB_H
