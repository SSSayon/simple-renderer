#ifndef SAMPLER_H
#define SAMPLER_H

#include <random>
#include "Vector3f.h"
#include "material.hpp"

class Sampler {

public:
    Sampler() = default;
    virtual ~Sampler() = default;

    virtual bool sample(const Material *material, const Vector3f &dir, const Vector3f &normal, std::mt19937 &rng,
    /* return values */ Vector3f &dirOut, float &pdf) = 0;

    virtual float samplePdf(const Material *material, const Vector3f &dir, const Vector3f &normal) = 0;
};

#endif // SAMPLER_H
