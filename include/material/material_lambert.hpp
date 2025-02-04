#ifndef MATERIAL_LAMBERT_H
#define MATERIAL_LAMBERT_H

#include "material.hpp"
#include "sampler/sampler_uniform.hpp"
#include "sampler/sampler_cosine_weighted.hpp"

class MaterialLambert : public Material {

public:
    template<typename... Args>
    MaterialLambert(Args... args) : Material(args...) {
        typeName = "Lambert";
        // sampler = new SamplerUniform();
        sampler = new SamplerCosineWeighted();
    }

    Vector3f BRDF(const Vector3f &inDir, const Vector3f &outDir, 
                  const Vector3f &normal, const Vector2f &uv = Vector2f::MINUS_ONE) const override {
        if (uv == Vector2f::MINUS_ONE) {
            return diffuseColor / M_PI;
        } else {
            return diffuseColor * texture->getColor(uv) / M_PI;
        }
    }
};

#endif // MATERIAL_LAMBERT_H
