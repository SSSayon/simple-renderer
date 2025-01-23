#ifndef MATERIAL_H
#define MATERIAL_H

#include <cmath>

#include <tiny_obj_loader.h>

#include "Vector3f.h"
#include "ray.hpp"
#include "hit.hpp"

class Sampler;

enum class MaterialType {
    Phong,
    Reflective,
    Refractive
};

class Material {
public:

    explicit Material(const MaterialType &_materialType, Sampler *_sampler, 
                      const Vector3f &_diffuseColor, const Vector3f &_specularColor = Vector3f::ZERO, float _shininess = 0.0f,
                      float _refractionIndex = 1.0f) :
        materialType(_materialType), sampler(_sampler),
        diffuseColor(_diffuseColor), specularColor(_specularColor), shininess(_shininess),
        refractionIndex(_refractionIndex) 
    {}

    virtual ~Material() = default;

    MaterialType getType() const { return materialType; }
    Sampler *getSampler() const { return sampler; }
    Vector3f getDiffuseColor() const { return diffuseColor; }
    Vector3f getSpecularColor() const { return specularColor; }
    float getShininess() const { return shininess; }
    float getRefractionIndex() const { return refractionIndex; }

    // keep the same material type (thus also the same sampler, etc.)
    Material *createTinyMaterial(const tinyobj::material_t &tinyMaterial) { 
        return new Material(
            materialType, sampler,
            Vector3f(tinyMaterial.diffuse[0], tinyMaterial.diffuse[1], tinyMaterial.diffuse[2]),
            Vector3f(tinyMaterial.specular[0], tinyMaterial.specular[1], tinyMaterial.specular[2]),
            tinyMaterial.shininess,
            tinyMaterial.ior
        );
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
    MaterialType materialType;
    Sampler *sampler;

    Vector3f diffuseColor;
    Vector3f specularColor;
    float shininess;

    float refractionIndex;
};

#endif // MATERIAL_H
