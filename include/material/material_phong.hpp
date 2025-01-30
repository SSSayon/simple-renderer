#ifndef MATERIAL_PHONG_H
#define MATERIAL_PHONG_H

#include "Vector3f.h"
#include "material.hpp"
#include "ray.hpp"
#include "hit.hpp"

// ONLY FOR Whitted-style ray casting
class MaterialPhong : public Material {

public:
    template<typename... Args>
    MaterialPhong(Args... args) : Material(args...) {
        typeName = "Phong";
    }

    Vector3f BRDF(const Vector3f &inDir, const Vector3f &outDir, 
                  const Vector3f &normal, const Vector2f &uv = Vector2f::ZERO) const override {
        return Vector3f::ZERO;
    } // NOT USED

    Vector3f Shade(const Ray &ray, const Hit &hit,
                   const Vector3f &dirToLight, const Vector3f &lightColor) {
        Vector3f shaded = Vector3f::ZERO;

        Vector3f normal = hit.getNormal();
        shaded += diffuseColor * std::max(0.0f, Vector3f::dot(dirToLight, normal));

        Vector3f reflect = 2.0f * Vector3f::dot(normal, dirToLight) * normal - dirToLight;
        shaded += specularColor * std::pow(
            std::max(0.0f, Vector3f::dot(-ray.getDirection(), reflect)), 
            shininess
        );

        return lightColor * shaded;
    }

};

#endif // MATERIAL_PHONG_H
