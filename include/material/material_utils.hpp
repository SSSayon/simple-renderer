#ifndef MATERIAL_UTILS_H
#define MATERIAL_UTILS_H

#include <cstring>

#include "material_phong.hpp"
#include "material_reflective.hpp"
#include "material_refractive.hpp"
#include "material_lambert.hpp"
#include "material_modified_phong.hpp"
#include "material_light.hpp"

template<typename... Args>
Material* createMaterial(const char *typeName, Args... args) {

    if (!strcmp(typeName, "Phong"))                return new MaterialPhong(args...);
    else if (!strcmp(typeName, "Reflective"))      return new MaterialReflective(args...);
    else if (!strcmp(typeName, "Refractive"))      return new MaterialRefractive(args...);
    else if (!strcmp(typeName, "Lambert"))         return new MaterialLambert(args...);
    else if (!strcmp(typeName, "ModifiedPhong"))   return new MaterialModifiedPhong(args...);
    else if (!strcmp(typeName, "Light"))           return new MaterialLight(args...);

    else {
        printf("Unknown material type: '%s'\n", typeName);
        exit(0);
    }
}

inline Material* createTinyMaterial(const char *typeName, std::string textureFilename, const tinyobj::material_t &tinyMaterial) {
    return createMaterial(typeName, textureFilename,
        Vector3f(tinyMaterial.diffuse[0], tinyMaterial.diffuse[1], tinyMaterial.diffuse[2]),
        Vector3f(tinyMaterial.specular[0], tinyMaterial.specular[1], tinyMaterial.specular[2]),
        tinyMaterial.shininess,
        tinyMaterial.ior
    );
}

#endif // MATERIAL_UTILS_H
