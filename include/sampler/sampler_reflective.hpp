#ifndef SAMPLER_REFLECTIVE_H
#define SAMPLER_REFLECTIVE_H

#include "sampler.hpp"

class SamplerReflective : public Sampler {
public:
    SamplerReflective() = default;
    ~SamplerReflective() override = default;

    bool sample(const Material *material, const Vector3f &dir, const Vector3f &normal,
    /* return values */ Vector3f &dirOut) {
        dirOut = -dir + 2.0f * Vector3f::dot(normal, dir) * normal;
        return true;
    }
    bool sample(const Material *material, const Vector3f &dir, const Vector3f &normal, std::mt19937 &rng,
    /* return values */ Vector3f &dirOut, float &pdf) override {
        pdf = -1.0f;
        return sample(material, dir, normal, dirOut);
    }

    float samplePdf(const Material *material, const Vector3f &dir, const Vector3f &normal) override {
        return -1.0f;
    }
};

#endif // SAMPLER_REFLECTIVE_H
