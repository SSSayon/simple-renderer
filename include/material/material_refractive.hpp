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
                  const Vector3f &normal, const Vector2f &uv = Vector2f::MINUS_ONE) const override {
        float cosThetaI = Vector3f::dot(inDir, normal);
        float cosThetaO = Vector3f::dot(outDir, normal);
        if (cosThetaI * cosThetaO > 0.0f) { // total internal reflection
            return Vector3f(1.0f);
        } else { // refraction
            if (cosThetaI > 0.0f) { // from outside to inside
                return Vector3f(1.0f / (refractionIndex * refractionIndex)); // the ior^2 term accounts for energy loss
            } else {                // from inside to outside
                return Vector3f(refractionIndex * refractionIndex);
            }
        }
    }

};

#endif // MATERIAL_REFRACTIVE_H
