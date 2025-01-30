#include "Vector3f.h"
#include "objects/object3d.hpp"
#include "objects/group.hpp"
#include "material/material.hpp"
#include "material/material_light.hpp"
#include "ray.hpp"
#include "hit.hpp"

#ifndef RR
#define RR 0.9f
#endif
#define MAX_DEPTH_NEE 15

Vector3f ShaderNEE(Ray ray, Group *baseGroup, Group *lightGroup, const Vector3f &backgroundColor, std::mt19937 &rng) {
    std::uniform_real_distribution<float> dist(0, 1); 

    Vector3f weight(1.0f);
    Vector3f finalColor = Vector3f::ZERO;
    float lastPdf = -1.0f;
    for (int depth = 0; depth < MAX_DEPTH_NEE; ++depth) {

        Hit hit;
        bool isBaseGroupHit = baseGroup->intersect(ray, hit, 0);
        Hit lightHit = hit; // GIVE SPECIAL NOTICE HERE!
        bool isLightGroupHit = lightGroup->intersect(ray, lightHit, 0);

        // hit light
        if (isLightGroupHit) {
            if (lastPdf < 0) { // last hit object is specular, thus haven't sampled the light
                finalColor += dynamic_cast<MaterialLight*>(lightHit.getMaterial())->emit(-ray.getDirection(), lightHit.getEmitNormal())
                            * weight;
            }
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

        Vector3f isectPoint = ray.pointAtParameter(hit.getT());

        if (pdf > 0) { // non-specular, apply NEE and sample the light

            // -------------------------------------- NEE --------------------------------------
            float sampleLightPdf;
            Object3D *light = lightGroup->sampleObject(rng, sampleLightPdf);
            float sampleLightPointPdf;
            Vector3f lightPoint = light->samplePoint(rng, sampleLightPointPdf);

            Ray sampledLightRay(isectPoint, (lightPoint - isectPoint).normalized());
            sampledLightRay.originOffset();

            float cosTheta = Vector3f::dot(sampledLightRay.getDirection(), hit.getNormal());
            if (cosTheta > 1e-3f) {

                Hit sampledLightHit;
                baseGroup->intersect(sampledLightRay, sampledLightHit, 0);
                if (lightGroup->intersect(sampledLightRay, sampledLightHit, 0)) {
                    if ((sampledLightRay.pointAtParameter(sampledLightHit.getT()) - lightPoint).squaredLength() < 1e-4f) {

                        float cosThetaLight = Vector3f::dot(-sampledLightRay.getDirection(), sampledLightHit.getEmitNormal());
                        if (cosThetaLight > 1e-3f) {

                            finalColor += dynamic_cast<MaterialLight*>(sampledLightHit.getMaterial())->emit(-sampledLightRay.getDirection(), sampledLightHit.getEmitNormal())
                                        * weight 
                                        * hit.getMaterial()->BRDF(sampledLightRay.getDirection(), -ray.getDirection(), hit.getNormal(), hit.getUV()) 
                                        * cosTheta
                                        / sampleLightPdf 
                                        / sampleLightPointPdf * cosThetaLight / (lightPoint - isectPoint).squaredLength();
                                                             // cos(theta) / r^2, convert the pdf from area measure to solid angle measure
                        }
                    }
                }
            }
            // -------------------------------------- NEE --------------------------------------

            weight = weight * hit.getMaterial()->BRDF(outDir, -ray.getDirection(), hit.getNormal(), hit.getUV()) 
                            * std::max(0.0f, Vector3f::dot(hit.getNormal(), outDir))
                            / pdf;

        } else {       // specular, do nothing
            weight = weight * hit.getMaterial()->BRDF(outDir, -ray.getDirection(), hit.getNormal(), hit.getUV()) / (-pdf);
        }

        ray = Ray(isectPoint, outDir);
        ray.originOffset();
        lastPdf = pdf;

        // Russian Roulette
        if (dist(rng) > std::max(0.05f, RR - depth * 0.05f)) break;
        weight = weight / RR;
    }

    return finalColor;
    // return Vector3f::clamp(replaceBad(finalColor), 0.0f, 20.0f);
}
