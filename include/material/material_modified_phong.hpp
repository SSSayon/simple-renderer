#ifndef MATERIAL_MODIFIED_PHONG_H
#define MATERIAL_MODIFIED_PHONG_H

#include <cmath>
#include "material.hpp"
#include "sampler/sampler_cosine_weighted.hpp"
#include "sampler/sampler_modified_phong.hpp"

class MaterialModifiedPhong : public Material {

public:
    template<typename... Args>
    MaterialModifiedPhong(Args... args) : Material(args...) {
        typeName = "ModifiedPhong";
        // sampler = new SamplerCosineWeighted();
        sampler = new SamplerModifiedPhong();
    }

    Vector3f BRDF(const Vector3f &inDir, const Vector3f &outDir, 
                  const Vector3f &normal, const Vector2f &uv = Vector2f::ZERO) const override {
        Vector3f reflectDir = -inDir + 2.0f * Vector3f::dot(normal, inDir) * normal;

        if (uv == Vector2f::ZERO) {
            return diffuseColor / M_PI 
                 + specularColor * std::pow(std::max(0.0f, Vector3f::dot(outDir, reflectDir)), 
                                            shininess) 
                                 * (shininess + 2.0f) / (2.0f * M_PI); // no exact normalization term exists
        } else {
            return diffuseColor * texture->getColor(uv) / M_PI 
                 + specularColor * std::pow(std::max(0.0f, Vector3f::dot(outDir, reflectDir)), 
                                            shininess) 
                                 * (shininess + 2.0f) / (2.0f * M_PI);
        }
    }

};

#endif // MATERIAL_MODIFIED_PHONG_H
