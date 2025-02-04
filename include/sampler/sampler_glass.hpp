#ifndef SAMPLER_GLASS_H
#define SAMPLER_GLASS_H

#include "Vector3f.h"
#include "sampler.hpp"

class SamplerGlass : public Sampler {
public:
    SamplerGlass() = default;
    ~SamplerGlass() override = default;

    // reference: https://henryzxu.github.io/pathtracing-p2/
    bool sample(const Material *material, const Vector3f &inDir, const Vector3f &normal, std::mt19937 &rng,
    /* return values */ Vector3f &outDir, float &pdf) override {

        float cosI = Vector3f::dot(inDir, normal);
        float etaI = 1.0f,  etaT = material->getRefractionIndex();
        Vector3f n = normal;

        if (cosI < 0.0f) { cosI = -cosI; std::swap(etaI, etaT); n = -normal; }

        float eta = etaI / etaT;
        float cosTSquare = 1.0f - eta * eta * (1.0f - cosI * cosI);

        if (cosTSquare <= 0.0f) { // total internal reflection
            outDir = -inDir + 2.0f * cosI * n;
        } else {
            float R0 = (etaI - etaT) * (etaI - etaT) / ((etaI + etaT) * (etaI + etaT)); // Schlick's approximation
            float R = R0 + (1.0f - R0) * powf(1.0f - cosI, 5.0f);

            std::uniform_real_distribution<float> dist(0.0f, 1.0f); 
            if (dist(rng) < R) { // reflection
                outDir = -inDir + 2.0f * cosI * n;
            } else {             // refraction
                outDir = -eta * inDir + (eta * cosI - sqrtf(cosTSquare)) * n;
                outDir.normalize();
            }
        }

        pdf = -1.0f;
        return true;
    }

    float PDF(const Material *material, const Vector3f &inDir, const Vector3f &outDir, const Vector3f &normal) override {
        return -1.0f;
    }
};

#endif // SAMPLER_GLASS_H
