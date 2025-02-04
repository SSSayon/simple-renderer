#ifndef SAMPLER_REFRACTIVE_H
#define SAMPLER_REFRACTIVE_H

#include "Vector3f.h"
#include "sampler.hpp"

class SamplerRefractive : public Sampler {
public:
    SamplerRefractive() = default;
    ~SamplerRefractive() override = default;

    bool sample(const Material *material, const Vector3f &inDir, const Vector3f &normal,
    /* return values */ Vector3f &outDir) {
        float cosI = Vector3f::dot(inDir, normal);
        float etaI = 1.0f,  etaT = material->getRefractionIndex();
        Vector3f n = normal;

        if (cosI < 0.0f) { cosI = -cosI; std::swap(etaI, etaT); n = -normal; }

        float eta = etaI / etaT;
        float cosTSquare = 1.0f - eta * eta * (1.0f - cosI * cosI);

        if (cosTSquare <= 0.0f) { // total internal reflection
            outDir = -inDir + 2.0f * cosI * n;
        } else {                     // refraction
            outDir = -eta * inDir + (eta * cosI - sqrtf(cosTSquare)) * n;
            outDir.normalize();
        }
        return true;
    }
    bool sample(const Material *material, const Vector3f &inDir, const Vector3f &normal, std::mt19937 &rng,
    /* return values */ Vector3f &outDir, float &pdf) override {
        pdf = -1.0f;
        return sample(material, inDir, normal, outDir);
    }

    float PDF(const Material *material, const Vector3f &inDir, const Vector3f &outDir, const Vector3f &normal) override {
        return -1.0f;
    }
};

#endif // SAMPLER_REFRACTIVE_H
