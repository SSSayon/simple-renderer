#ifndef SAMPLER_MODIFIED_PHONG_H
#define SAMPLER_MODIFIED_PHONG_H

#include "Vector3f.h"
#include "sampler.hpp"
#include "sampler_cosine_weighted.hpp"
#include "sampler_uniform.hpp"

class SamplerModifiedPhong : public Sampler {
public:
    SamplerModifiedPhong() = default;
    ~SamplerModifiedPhong() override = default;

    bool sample(const Material *material, const Vector3f &inDir, const Vector3f &normal, std::mt19937 &rng,
    /* return values */ Vector3f &outDir, float &pdf) override {
        Vector3f diffuseColor = material->getDiffuseColor();
        Vector3f specularColor = material->getSpecularColor();
        float shininess = material->getShininess();

        Vector3f reflectDir = -inDir + 2.0f * Vector3f::dot(normal, inDir) * normal;

        auto f = [](const Vector3f &v) { return v.x() + v.y() + v.z(); };
        float P = f(specularColor) / (f(specularColor) + f(diffuseColor));
        std::uniform_real_distribution<float> dist(0, 1);

        if (dist(rng) < P) {            // sample specular
            float phi = 2 * M_PI * dist(rng);                       // NOTE: w.r.t. reflectDir
            float z = powf(dist(rng), 1.0f / (shininess + 1)); // cos(theta), w.r.t. reflectDir
            float r = sqrtf(1 - z * z);

            outDir = hemiToWorld(Vector3f(r * cos(phi), r * sin(phi), z), reflectDir);

        } else {                        // sample diffuse (cosine-weighted)
            SamplerCosineWeighted sampler;
            sampler.sample(material, inDir, normal, rng, outDir, pdf);
        }

        float cosTheta = Vector3f::dot(outDir, normal); // w.r.t. normal
        if (cosTheta <= 0) { // this tailored sample strategy failed, turn to a safe strategy
            // SamplerCosineWeighted sampler;
            SamplerUniform sampler; // faster, almost the same quality as cosine-weighted
            sampler.sample(material, inDir, normal, rng, outDir, pdf);
            return true;
        }

        float diffusePdf = cosTheta / M_PI;
        float specularPdf = powf(std::max(0.0f, Vector3f::dot(outDir, reflectDir)), shininess) * (shininess + 1) / (2 * M_PI);
        pdf = P * specularPdf + (1 - P) * diffusePdf;

        return true;
    }

    float PDF(const Material *material, const Vector3f &inDir, const Vector3f &outDir, const Vector3f &normal) override {
        Vector3f diffuseColor = material->getDiffuseColor();
        Vector3f specularColor = material->getSpecularColor();
        float shininess = material->getShininess();

        auto f = [](const Vector3f &v) { return v.x() + v.y() + v.z(); };
        float P = f(specularColor) / (f(specularColor) + f(diffuseColor));

        Vector3f reflectDir = -inDir + 2.0f * Vector3f::dot(inDir, normal) * normal;
        float specularPdf = powf(std::max(0.0f, Vector3f::dot(outDir, reflectDir)), shininess) * (shininess + 1) / (2 * M_PI);
        float diffusePdf  = std::max(0.0f, Vector3f::dot(outDir, normal)) / M_PI;

        return P * specularPdf + (1 - P) * diffusePdf; // just an approximation though...
    }
};

#endif // SAMPLER_MODIFIED_PHONG_H
