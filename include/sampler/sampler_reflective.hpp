#ifndef SAMPLER_REFLECTIVE_H
#define SAMPLER_REFLECTIVE_H

#include "sampler.hpp"

class SamplerReflective : public Sampler {
public:
    SamplerReflective() = default;
    ~SamplerReflective() override = default;

    bool sample(const Material *material, const Vector3f &inDir, const Vector3f &normal,
    /* return values */ Vector3f &outDir) {
        outDir = -inDir + 2.0f * Vector3f::dot(inDir, normal) * normal;
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

#endif // SAMPLER_REFLECTIVE_H
