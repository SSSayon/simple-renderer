#ifndef MATERIAL_COOK_TORRANCE_H
#define MATERIAL_COOK_TORRANCE_H

#include "Vector3f.h"
#include "material.hpp"
#include "sampler/sampler_uniform.hpp"
#include "sampler/sampler_cook_torrance.hpp"

class MaterialCookTorrance : public Material {
public:
    template<typename... Args>
    MaterialCookTorrance(Args... args) : Material(args...) {
        typeName = "ModifiedPhong";
        // sampler = new SamplerUniform();
        sampler = new SamplerCookTorrance();

        alpha = roughness * roughness;
        alphaSquare = alpha * alpha;
        k = alphaSquare / 2.0f;
    }

    // see https://zhuanlan.zhihu.com/p/473834300 (F: wrong in the article)
    // F: Fresnel term,             using Fresnel-Schlick's approximation
    // G: Geometry term,            using Smith's method and Schlick's approximation
    // D: Normal distribution term, using GGX distribution
    Vector3f BRDF(const Vector3f &inDir, const Vector3f &outDir, 
                  const Vector3f &normal, const Vector2f &uv = Vector2f::MINUS_ONE) const override {
        Vector3f halfDir = (inDir + outDir).normalized();
        float cosThetaH = Vector3f::dot(halfDir, normal);
        float cosThetaO = Vector3f::dot(outDir, normal);
        float cosThetaI = Vector3f::dot(inDir, normal);

        // Fresnel term
        Vector3f F = F0 + (1 - F0) * powf(1 - Vector3f::dot(halfDir, outDir), 5);

        // Geometry term
        auto Gsub = [](float cosTheta, float k) {
            return cosTheta / (cosTheta * (1 - k) + k);
        };
        float G = Gsub(cosThetaO, k) * Gsub(cosThetaI, k);

        // Normal distribution term
        float denom = cosThetaH * cosThetaH * (alphaSquare - 1) + 1;
        float D = alphaSquare / (M_PI * denom * denom + 1e-8f);

        // BRDF
        if (uv == Vector2f::MINUS_ONE) {
            return diffuseColor / M_PI 
                 + specularColor * F * G * D / (4 * cosThetaI * cosThetaO);
        } else {
            return diffuseColor * texture->getColor(uv) / M_PI 
                 + specularColor * F * G * D / (4 * cosThetaI * cosThetaO);
        }
    }

private:
    float alpha, alphaSquare;
    float k;
};

#endif // MATERIAL_COOK_TORRANCE_H
