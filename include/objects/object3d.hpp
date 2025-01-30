#ifndef OBJECT3D_H
#define OBJECT3D_H

#include <random>

#include <tiny_obj_loader.h>

#include "Vector3f.h"
#include "ray.hpp"
#include "hit.hpp"
#include "material/material.hpp"

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

    // Intersect Ray with this object. If hit, store information in hit structure.
    virtual bool intersect(const Ray &r, Hit &h, float tmin) = 0;
    // only for MIS
    virtual bool intersect(const Ray &r, Hit &h, float tmin, float &pdf) = 0;

    virtual Vector3f samplePoint(std::mt19937 &rng, float &pdf) = 0;

protected:
    Material *material;
};

#endif

