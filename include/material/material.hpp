#ifndef MATERIAL_H
#define MATERIAL_H

#include <tiny_obj_loader.h>
#include "Vector2f.h"
#include "Vector3f.h"
#include "texture.hpp"

class Sampler;

class Material {
public:

    explicit Material(std::string _textureFilename,
                      const Vector3f &_diffuseColor, const Vector3f &_specularColor, float _shininess,
                      float _refractionIndex):
        textureFilename(_textureFilename), 
        diffuseColor(_diffuseColor), specularColor(_specularColor), shininess(_shininess),
        refractionIndex(_refractionIndex)
    {
        if (textureFilename.empty()) texture = new Texture();
        else texture = new Texture(textureFilename);
    }

    Material(const Vector3f &_emitColor): // ONLY for area light
        emitColor(_emitColor)
    {}

    virtual ~Material() = default;

    const char *getTypeName() const { return typeName; }
    Sampler *getSampler() const { return sampler; }
    Texture *getTexture() const { return texture; }
    std::string getTextureFilename() const { return textureFilename; }
    Vector3f getDiffuseColor() const { return diffuseColor; }
    Vector3f getSpecularColor() const { return specularColor; }
    float getShininess() const { return shininess; }
    float getRefractionIndex() const { return refractionIndex; }
    Vector3f getEmitColor() const { return emitColor; }

    virtual Vector3f BRDF(const Vector3f &inDir, const Vector3f &outDir, 
                          const Vector3f &normal, const Vector2f &uv = Vector2f::ZERO) const = 0;

protected:
    const char *typeName;
    Sampler *sampler = nullptr;

    std::string textureFilename = "";
    Texture *texture;

    Vector3f diffuseColor;
    Vector3f specularColor;
    float shininess;

    float refractionIndex;

    Vector3f emitColor = Vector3f::ZERO;

};

#endif // MATERIAL_H
