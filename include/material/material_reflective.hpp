#ifndef MATERIAL_REFLECTIVE_H
#define MATERIAL_REFLECTIVE_H

#include "Vector3f.h"
#include "material.hpp"
#include "sampler/sampler_reflective.hpp"

class MaterialReflective : public Material {

public:
    template<typename... Args>
    MaterialReflective(Args... args) : Material(args...) {
        typeName = "Reflective";
        sampler = new SamplerReflective();
    }

    Vector3f BRDF(const Vector3f &inDir, const Vector3f &outDir, 
                  const Vector3f &normal, const Vector2f &uv = Vector2f::ZERO) const override {
        return Vector3f(1.0f);
    }

};

#endif // MATERIAL_REFLECTIVE_H
