#include "Vector3f.h"
#include "objects/group.hpp"
#include "material/material.hpp"
#include "ray.hpp"
#include "hit.hpp"

#ifndef RR
#define RR 0.9f
#endif
#define MAX_DEPTH_NAIVE 15

Vector3f ShaderNaive(Ray ray, Group *baseGroup, Group *lightGroup, const Vector3f &backgroundColor, std::mt19937 &rng) {
    std::uniform_real_distribution<float> dist(0, 1); 

    Vector3f weight(1.0f);
    Vector3f finalColor = Vector3f::ZERO;
    for (int depth = 0; depth < MAX_DEPTH_NAIVE; ++depth) {

        Hit hit;
        bool isBaseGroupHit = baseGroup->intersect(ray, hit, 0);
        Hit lightHit = hit; // GIVE SPECIAL NOTICE HERE!
        bool isLightGroupHit = lightGroup->intersect(ray, lightHit, 0);

        // hit light
        if (isLightGroupHit) {
            finalColor += dynamic_cast<MaterialLight*>(lightHit.getMaterial())->emit(-ray.getDirection(), lightHit.getEmitNormal())
                        * weight;
            hit = lightHit; // GIVE SPECIAL NOTICE HERE!
        }

        // does not hit any object
        if (!isLightGroupHit && !isBaseGroupHit) {
            finalColor += backgroundColor * weight;
            break;
        }

        // hit something
        Vector3f outDir;
        float pdf;
        bool isSampled = hit.getMaterial()->getSampler()->sample(hit.getMaterial(), -ray.getDirection(), hit.getNormal(), rng,
                                                                 outDir, pdf);
        if (!isSampled) break;

        if (pdf > 0) {
            weight = weight * hit.getMaterial()->BRDF(outDir, -ray.getDirection(), hit.getNormal(), hit.getUV()) 
                            * std::max(0.0f, Vector3f::dot(hit.getNormal(), outDir))
                            / pdf;
        } else {
            weight = weight * hit.getMaterial()->BRDF(outDir, -ray.getDirection(), hit.getNormal(), hit.getUV()) / (-pdf);
        }

        ray = Ray(ray.pointAtParameter(hit.getT()), outDir);
        ray.originOffset();

        // Russian Roulette
        if (dist(rng) > std::max(0.05f, RR - depth * 0.05f)) break;
        weight = weight / RR;
    }

    // return Vector3f::clamp(replaceBad(finalColor), 0.0f, 20.0f);
    return finalColor;
}
