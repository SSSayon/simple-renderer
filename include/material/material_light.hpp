#ifndef MATERIAL_LIGHT_H
#define MATERIAL_LIGHT_H

#include "material.hpp"
#include <cassert>
#include <iostream>

class MaterialLight : public Material {
public:
    template<typename... Args>
    MaterialLight(Args... args) : Material(args...) { // emitColor has been set
        typeName = "Light";
    }

    void setBaseMaterial(Material *material) {
        baseMaterial = material;
        sampler = material->getSampler();
        textureFilename = material->getTextureFilename();
        texture = material->getTexture();
        diffuseColor = material->getDiffuseColor();
        specularColor = material->getSpecularColor();
        shininess = material->getShininess();
        refractionIndex = material->getRefractionIndex();
    }

    Vector3f BRDF(const Vector3f &inDir, const Vector3f &outDir, 
                  const Vector3f &normal, const Vector2f &uv = Vector2f::ZERO) const override {
        if (baseMaterial) return baseMaterial->BRDF(inDir, outDir, normal, uv);
        std::cerr << "MaterialLight has not been assigned a baseMaterial!" << std::endl;
        exit(1);
    }

    Vector3f emit(const Vector3f &outDir, const Vector3f &normal) const {
        assert(emitColor != Vector3f::ZERO);
        return emitColor * std::max(0.0f, Vector3f::dot(normal, outDir));
    }

private:
    Material *baseMaterial = nullptr;
};

#endif // MATERIAL_LIGHT_H
