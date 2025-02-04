#ifndef OBJECT3D_H
#define OBJECT3D_H

#include <random>
#include <vector>

#include <tiny_obj_loader.h>

#include "Vector3f.h"
#include "ray.hpp"
#include "hit.hpp"
#include "material/material.hpp"
#include "accel/aabb.hpp"

// Base class for all 3d entities.
class Object3D {
public:
    Object3D() : material(nullptr) {}

    virtual ~Object3D() = default;

    explicit Object3D(Material *material) {
        this->material = material;
    }

    Material *getMaterial() const { 
        return material; 
    }

    /// Intersect Ray with this object. If hit, store information in hit structure.
    virtual bool intersect(const Ray &r, Hit &h, float tmin) const = 0;
    /// for MIS
    /// @param pdf return the corresponding NEE pdf of sampling this point
    virtual bool intersect(const Ray &r, Hit &h, float tmin, float &pdf) = 0;

    /// for NEE/MIS, sample a point on the object
    virtual Vector3f samplePoint(std::mt19937 &rng, float &pdf) = 0;

    /// get the AABB of the object
    virtual AABB *getAABB() const = 0;
    /// only used in Group for BVH construction, gathering all objects
    virtual std::vector<const Object3D*> getObjects() const = 0;
    /// handle Plane separately when building BVH, as its AABB is infinite
    virtual bool isPlane() const { return false; }

protected:
    Material *material;
    mutable AABB *aabb = nullptr;
};

#endif
