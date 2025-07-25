#ifndef SAMPLER_H
#define SAMPLER_H

#include <random>

#include "Vector3f.h"
#include "material/material.hpp"

class Sampler {

public:
    Sampler() = default;
    virtual ~Sampler() = default;

    virtual bool sample(const Material *material, const Vector3f &inDir, const Vector3f &normal, std::mt19937 &rng,
    /* return values */ Vector3f &outDir, float &pdf) = 0;

    virtual float PDF(const Material *material, const Vector3f &inDir, const Vector3f &outDir, const Vector3f &normal) = 0;
};

#endif // SAMPLER_H
