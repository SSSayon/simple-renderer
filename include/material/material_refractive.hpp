#ifndef MATERIAL_REFRACTIVE_H
#define MATERIAL_REFRACTIVE_H

#include "Vector3f.h"
#include "material.hpp"
#include "sampler/sampler_refractive.hpp"

class MaterialRefractive : public Material {

public:
    template<typename... Args>
    MaterialRefractive(Args... args) : Material(args...) {
        typeName = "Refractive";
        sampler = new SamplerRefractive();
    } 

    Vector3f BRDF(const Vector3f &inDir, const Vector3f &outDir, 
                  const Vector3f &normal, const Vector2f &uv = Vector2f::ZERO) const override {
        return Vector3f(1.0f);
    }

};

#endif // MATERIAL_REFRACTIVE_H
