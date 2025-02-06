#ifndef SAMPLER_COOK_TORRANCE_H
#define SAMPLER_COOK_TORRANCE_H

#include <cmath>
#include "Vector3f.h"
#include "sampler.hpp"
#include "sampler_cosine_weighted.hpp"
#include "utils.hpp"

class SamplerCookTorrance : public Sampler {
public:
    SamplerCookTorrance() = default;
    ~SamplerCookTorrance() override = default;

    // reference: https://schuttejoe.github.io/post/ggximportancesamplingpart1/
    bool sample(const Material *material, const Vector3f &inDir, const Vector3f &normal, std::mt19937 &rng,
    /* return values */ Vector3f &outDir, float &pdf) override {
        Vector3f diffuseColor = material->getDiffuseColor();
        Vector3f specularColor = material->getSpecularColor();
        float alpha = material->getRoughness() * material->getRoughness();
        float alphaSquare = alpha * alpha;

        auto f = [](const Vector3f &v) { return v.x() + v.y() + v.z(); };
        float P = f(specularColor) / (f(specularColor) + f(diffuseColor));
        P = std::max(0.1f, std::min(0.9f, P));
        std::uniform_real_distribution<float> dist(0, 1);

        Vector3f halfDir;

        if (dist(rng) < P) {            // sample specular
            // sample halfDir
            float phi = 2 * M_PI * dist(rng);
            float u = dist(rng);
            float z = sqrtf((1 - u) / (u * (alphaSquare - 1) + 1));
            float r = sqrtf(1 - z * z);
            halfDir = hemiToWorld(Vector3f(r * cos(phi), r * sin(phi), z), normal);

            // convert to outDir
            outDir = -inDir + 2.0f * Vector3f::dot(inDir, halfDir) * halfDir;
        } else {                        // sample diffuse
            SamplerCosineWeighted sampler;
            sampler.sample(material, inDir, normal, rng, outDir, pdf);
            halfDir = (inDir + outDir).normalized(); // DO NOT FORGET THIS!
        }

        float cosThetaO = Vector3f::dot(outDir, normal);
        if (cosThetaO <= 0) {
            return false;
        }

        float diffusePdf = cosThetaO / M_PI;
        
        float cosThetaH = Vector3f::dot(halfDir, normal);
        float denom = cosThetaH * cosThetaH * (alphaSquare - 1) + 1;
        float halfDirPdf = alphaSquare * cosThetaH / (M_PI * denom * denom + 1e-8f);
        float specularPdf = halfDirPdf / (4 * Vector3f::dot(inDir, halfDir) + 1e-8f); // Jacobian

        pdf = P * specularPdf + (1 - P) * diffusePdf;
        return true;
    }

    float PDF(const Material *material, const Vector3f &inDir, const Vector3f &outDir, const Vector3f &normal) override {
        Vector3f diffuseColor = material->getDiffuseColor();
        Vector3f specularColor = material->getSpecularColor();
        float alpha = material->getRoughness() * material->getRoughness();
        float alphaSquare = alpha * alpha;

        auto f = [](const Vector3f &v) { return v.x() + v.y() + v.z(); };
        float P = f(specularColor) / (f(specularColor) + f(diffuseColor));
        P = std::max(0.1f, std::min(0.9f, P));

        Vector3f halfDir = (inDir + outDir).normalized();
        float cosThetaH = Vector3f::dot(halfDir, normal);
        float cosThetaO = Vector3f::dot(outDir, normal);

        float denom = cosThetaH * cosThetaH * (alphaSquare - 1) + 1;
        float halfDirPdf = alphaSquare * cosThetaH / (M_PI * denom * denom + 1e-8f);
        float specularPdf = halfDirPdf / (4 * Vector3f::dot(inDir, halfDir) + 1e-8f); // Jacobian

        float diffusePdf = cosThetaO / M_PI;

        return P * specularPdf + (1 - P) * diffusePdf;
    }
};

#endif // SAMPLER_COOK_TORRANCE_H
