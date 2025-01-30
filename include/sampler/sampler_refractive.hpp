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
        float cos_i = Vector3f::dot(inDir, normal);
        float eta_i = 1.0f,  eta_t = material->getRefractionIndex();
        Vector3f n = normal;

        if (cos_i < 0.0f) { cos_i = -cos_i; std::swap(eta_i, eta_t); n = -normal; }

        float eta = eta_i / eta_t;
        float cos_t_squared = 1.0f - eta * eta * (1.0f - cos_i * cos_i);

        if (cos_t_squared <= 0.0f) { // total internal reflection
            outDir = -inDir + 2.0f * cos_i * n;
        } else {                     // refraction
            outDir = -eta * inDir + (eta * cos_i - sqrtf(cos_t_squared)) * n;
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
