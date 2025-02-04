#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <tiny_obj_loader.h>
#include "Vector2f.h"
#include "Vector3f.h"
#include "texture.hpp"

class Sampler;

class Material {
public:

    explicit Material(std::string _textureFilename, std::string _normalTextureFilename,
                      const Vector3f &_diffuseColor, const Vector3f &_specularColor, float _shininess,
                      const Vector3f &_transmissiveColor, float _refractionIndex, 
                      float _roughness, const Vector3f &_F0):
        textureFilename(_textureFilename), normalTextureFilename(_normalTextureFilename),
        diffuseColor(_diffuseColor), specularColor(_specularColor), shininess(_shininess),
        transmissiveColor(_transmissiveColor), refractionIndex(_refractionIndex), 
        roughness(_roughness), F0(_F0)
    {
        if (textureFilename.empty()) texture = new Texture();
        else texture = new Texture(textureFilename);

        if (normalTextureFilename.empty()) normalTexture = new Texture();
        else normalTexture = new Texture(normalTextureFilename, true);
    }

    Material(const Vector3f &_emitColor): // ONLY for area light
        emitColor(_emitColor)
    {}

    virtual ~Material() = default;

    const char *getTypeName() const { return typeName; }
    Sampler *getSampler() const { return sampler; }
    std::string getTextureFilename() const { return textureFilename; }
    Texture *getTexture() const { return texture; }
    std::string getNormalTextureFilename() const { return normalTextureFilename; }
    Texture *getNormalTexture() const { return normalTexture; }
    Vector3f getDiffuseColor() const { return diffuseColor; }
    Vector3f getSpecularColor() const { return specularColor; }
    float getShininess() const { return shininess; }
    Vector3f getTransmissiveColor() const { return transmissiveColor; }
    float getRefractionIndex() const { return refractionIndex; }
    float getRoughness() const { return roughness; }
    Vector3f getF0() const { return F0; }
    Vector3f getEmitColor() const { return emitColor; }

    virtual Vector3f BRDF(const Vector3f &inDir, const Vector3f &outDir, 
                          const Vector3f &normal, const Vector2f &uv = Vector2f::MINUS_ONE) const = 0;

protected:
    const char *typeName;
    Sampler *sampler = nullptr;

    std::string textureFilename = "";
    Texture *texture;

    std::string normalTextureFilename = "";
    Texture *normalTexture;

    Vector3f diffuseColor;
    Vector3f specularColor;
    float shininess;

    Vector3f transmissiveColor;
    float refractionIndex;

    float roughness;
    Vector3f F0;

    Vector3f emitColor = Vector3f::ZERO;

};

#endif // MATERIAL_H
