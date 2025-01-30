#ifndef SAMPLER_COSINE_WEIGHTED_H
#define SAMPLER_COSINE_WEIGHTED_H

#include "sampler.hpp"
#include "utils.hpp"

class SamplerCosineWeighted : public Sampler {
public:
    SamplerCosineWeighted() = default;
    ~SamplerCosineWeighted() override = default;

    bool sample(const Material *material, const Vector3f &inDir, const Vector3f &normal, std::mt19937 &rng,
    /* return values */ Vector3f &outDir, float &pdf) override {
        std::uniform_real_distribution<float> dist(0, 1);
        float phi = 2 * M_PI * dist(rng);
        float z = sqrtf(dist(rng)); // cos(theta)
        float r = sqrtf(1 - z * z);

        outDir = hemiToWorld(Vector3f(r * cos(phi), r * sin(phi), z), normal);
        pdf = z / M_PI; // z = cos(theta) = dot(normal, outDir)

        return true;
    }

    float PDF(const Material *material, const Vector3f &inDir, const Vector3f &outDir, const Vector3f &normal) override {
        return std::max(0.0f, Vector3f::dot(normal, outDir)) / M_PI;
    }
};

#endif // SAMPLER_COSINE_WEIGHTED_H
