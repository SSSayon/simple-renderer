#ifndef MATERIAL_UTILS_H
#define MATERIAL_UTILS_H

#include <cstring>

#include "Vector3f.h"
#include "material_phong.hpp"
#include "material_light.hpp"
#include "material_reflective.hpp"
#include "material_refractive.hpp"
#include "material_glass.hpp"
#include "material_lambert.hpp"
#include "material_modified_phong.hpp"
#include "material_cook_torrance.hpp"


template<typename... Args>
Material* createMaterial(const char *typeName, Args... args) {

    if (!strcmp(typeName, "Phong"))                return new MaterialPhong(args...);
    else if (!strcmp(typeName, "Light"))           return new MaterialLight(args...);
    else if (!strcmp(typeName, "Reflective"))      return new MaterialReflective(args...);
    else if (!strcmp(typeName, "Refractive"))      return new MaterialRefractive(args...);
    else if (!strcmp(typeName, "Glass"))           return new MaterialGlass(args...);
    else if (!strcmp(typeName, "Lambert"))         return new MaterialLambert(args...);
    else if (!strcmp(typeName, "ModifiedPhong"))   return new MaterialModifiedPhong(args...);
    else if (!strcmp(typeName, "CookTorrance"))    return new MaterialCookTorrance(args...);

    else {
        printf("Unknown material type: '%s'\n", typeName);
        exit(0);
    }
}

inline Material* createTinyMaterial(const char *typeName, std::string textureFilename, std::string normalTextureFilename, const tinyobj::material_t &tinyMaterial) {
    return createMaterial(typeName, textureFilename, normalTextureFilename,
        Vector3f(tinyMaterial.diffuse[0], tinyMaterial.diffuse[1], tinyMaterial.diffuse[2]),
        Vector3f(tinyMaterial.specular[0], tinyMaterial.specular[1], tinyMaterial.specular[2]),
        tinyMaterial.shininess,
        Vector3f(tinyMaterial.transmittance[0], tinyMaterial.transmittance[1], tinyMaterial.transmittance[2]),
        tinyMaterial.ior,
        tinyMaterial.roughness,
        Vector3f(tinyMaterial.specular[0], tinyMaterial.specular[1], tinyMaterial.specular[2]) // F0
    );
}

#endif // MATERIAL_UTILS_H
