#ifndef MATERIAL_H
#define MATERIAL_H

#include <cmath>
#include <vecmath.h>

#include "Vector3f.h"
#include "ray.hpp"
#include "hit.hpp"

class Material {
public:

    explicit Material(const Vector3f &d_color, const Vector3f &s_color = Vector3f::ZERO, float s = 0) :
            diffuseColor(d_color), specularColor(s_color), shininess(s) {

    }

    virtual ~Material() = default;

    virtual Vector3f getDiffuseColor() const {
        return diffuseColor;
    }


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

protected:
    Vector3f diffuseColor;
    Vector3f specularColor;
    float shininess;
};


#endif // MATERIAL_H
