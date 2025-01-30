#ifndef SAMPLER_UNIFORM_H
#define SAMPLER_UNIFORM_H

#include "sampler.hpp"
#include "utils.hpp"

class SamplerUniform : public Sampler {
public:
    SamplerUniform() = default;
    ~SamplerUniform() override = default;

    bool sample(const Material *material, const Vector3f &inDir, const Vector3f &normal, std::mt19937 &rng,
    /* return values */ Vector3f &outDir, float &pdf) override {
        std::uniform_real_distribution<float> dist(0, 1);
        float phi = 2 * M_PI * dist(rng);
        float z = dist(rng);          // cos(theta), actually 1 - dist(rng)
        float r = sqrtf(1 - z * z);

        outDir = hemiToWorld(Vector3f(r * cos(phi), r * sin(phi), z), normal);
        pdf = 1 / (2 * M_PI);

        return true;
    }

    float PDF(const Material *material, const Vector3f &inDir, const Vector3f &outDir, const Vector3f &normal) override {
        return 1 / (2 * M_PI);        
    }
};

#endif // SAMPLER_UNIFORM_H
