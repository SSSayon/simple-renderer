#ifndef MATERIAL_GLASS_H
#define MATERIAL_GLASS_H

#include "Vector3f.h"
#include "material.hpp"
#include "sampler/sampler_glass.hpp"

class MaterialGlass : public Material {

public:
    template<typename... Args>
    MaterialGlass(Args... args) : Material(args...) {
        typeName = "Refractive";
        sampler = new SamplerGlass();
    } 

    Vector3f BRDF(const Vector3f &inDir, const Vector3f &outDir, 
                  const Vector3f &normal, const Vector2f &uv = Vector2f::MINUS_ONE) const override {
        float cosThetaI = Vector3f::dot(inDir, normal);
        float cosThetaO = Vector3f::dot(outDir, normal);
        if (cosThetaI * cosThetaO > 0.0f) { // reflection or total internal reflection
            return specularColor;
        } else { // refraction
            if (cosThetaI > 0.0f) { // from outside to inside
                return Vector3f(transmissiveColor / (refractionIndex * refractionIndex));
            } else {                // from inside to outside
                return Vector3f(transmissiveColor * refractionIndex * refractionIndex);
            }
        }
    }

};

#endif // MATERIAL_GLASS_H
