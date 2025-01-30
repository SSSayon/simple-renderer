#include "Vector3f.h"
#include "objects/group.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include "material/material.hpp"
#include "material/material_phong.hpp"
#include "light.hpp"
#include "scene_parser.hpp"
#include "sampler/sampler_reflective.hpp"
#include "sampler/sampler_refractive.hpp"

const int MAX_DEPTH_WHITTED = 20;

Vector3f ShaderWhitted(Ray ray, SceneParser &parser, Group *baseGroup, int depth) {

    if (depth > MAX_DEPTH_WHITTED) {
        return Vector3f::ZERO;
    }

    Hit hit;
    if (!baseGroup->intersect(ray, hit, 0)) {
        return parser.getBackgroundColor();
    }
    Vector3f isectPoint = ray.pointAtParameter(hit.getT());

    Vector3f finalColor = Vector3f::ZERO;

    if (!strcmp(hit.getMaterial()->getTypeName(), "Phong")) {
        for (int li = 0; li < parser.getNumLights(); ++li) {
            Light *light = parser.getLight(li);
            Vector3f dirToLight, lightColor;
            light->getIllumination(isectPoint, dirToLight, lightColor);

            Ray shadowRay = Ray(isectPoint, dirToLight);
            shadowRay.originOffset();
            Hit shadowHit;
            float shadowCoeff = 1.0f;
            if (baseGroup->intersect(shadowRay, shadowHit, 0)) {
                if (light->isBlocked(isectPoint, shadowRay.pointAtParameter(shadowHit.getT()))) {
                    shadowCoeff = 0.5f;
                }
            }
            finalColor += shadowCoeff * dynamic_cast<MaterialPhong*>(hit.getMaterial())
                ->Shade(ray, hit, dirToLight, lightColor);
            finalColor += hit.getMaterial()->getDiffuseColor() * 0.2f; // ambient
        }

    } else if (!strcmp(hit.getMaterial()->getTypeName(), "Reflective")) {
        Vector3f outDir;
        bool isSampled = dynamic_cast<SamplerReflective*>(hit.getMaterial()->getSampler())
            ->sample(hit.getMaterial(), -ray.getDirection(), hit.getNormal(), outDir);
        
        if (isSampled) {
            Ray reflectRay(isectPoint, outDir);
            reflectRay.originOffset();
            finalColor += ShaderWhitted(reflectRay, parser, baseGroup, depth + 1);
        }

    } else if (!strcmp(hit.getMaterial()->getTypeName(), "Refractive")) {
        Vector3f outDir;
        bool isSampled = dynamic_cast<SamplerRefractive*>(hit.getMaterial()->getSampler())
            ->sample(hit.getMaterial(), -ray.getDirection(), hit.getNormal(), outDir);
        
        if (isSampled) {
            Ray refractRay(isectPoint, outDir);
            refractRay.originOffset();
            finalColor += ShaderWhitted(refractRay, parser, baseGroup, depth + 1);
        }
    } else {
        printf("Unknown material type: '%s'\n", hit.getMaterial()->getTypeName());
        exit(0);
    }

    return finalColor;
}
