#include "Vector3f.h"
#include "objects/group.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include "material.hpp"
#include "light.hpp"
#include "scene_parser.hpp"
#include "sampler/sampler_reflective.hpp"
#include "sampler/sampler_refractive.hpp"

const int MAX_DEPTH = 20;

Vector3f ShaderWhitted(Ray ray, SceneParser &parser, Group *baseGroup, int depth) {

    if (depth > MAX_DEPTH) {
        return Vector3f::ZERO;
    }

    Hit hit;
    if (!baseGroup->intersect(ray, hit, 0)) {
        return parser.getBackgroundColor();
    }
    Vector3f isecPoint = ray.pointAtParameter(hit.getT());

    Vector3f finalColor = Vector3f::ZERO;

    if (hit.getMaterial()->getType() == MaterialType::Phong) {
        for (int li = 0; li < parser.getNumLights(); ++li) {
            Light *light = parser.getLight(li);
            Vector3f dirToLight, lightColor;
            light->getIllumination(isecPoint, dirToLight, lightColor);

            Ray shadowRay = Ray(isecPoint, dirToLight);
            shadowRay.originOffset();
            Hit shadowHit;
            float shadowCoeff = 1.0f;
            if (baseGroup->intersect(shadowRay, shadowHit, 0)) {
                if (light->isBlocked(isecPoint, shadowHit.getT())) {
                    shadowCoeff = 0.5f;
                }
            }
            
            finalColor += shadowCoeff * hit.getMaterial()->Shade(ray, hit, dirToLight, lightColor);
            finalColor += hit.getMaterial()->getDiffuseColor() * 0.2f; // ambient
        }

    } else if (hit.getMaterial()->getType() == MaterialType::Reflective) {
        Vector3f dirOut;
        bool isSampled = dynamic_cast<SamplerReflective*>(hit.getMaterial()->getSampler())
            ->sample(hit.getMaterial(), -ray.getDirection(), hit.getNormal(), dirOut);
        
        if (isSampled) {
            Ray reflectRay(isecPoint, dirOut);
            reflectRay.originOffset();
            finalColor += ShaderWhitted(reflectRay, parser, baseGroup, depth + 1);
        }

    } else if (hit.getMaterial()->getType() == MaterialType::Refractive) {
        Vector3f dirOut;
        bool isSampled = dynamic_cast<SamplerRefractive*>(hit.getMaterial()->getSampler())
            ->sample(hit.getMaterial(), -ray.getDirection(), hit.getNormal(), dirOut);
        
        if (isSampled) {
            Ray refractRay(isecPoint, dirOut);
            refractRay.originOffset();
            finalColor += ShaderWhitted(refractRay, parser, baseGroup, depth + 1);
        }
    }

    return finalColor;
}
