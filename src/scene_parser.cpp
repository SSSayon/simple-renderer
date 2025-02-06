#include "scene_parser.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <chrono>

#include <tiny_obj_loader.h>

#include "camera.hpp"
#include "light.hpp"
#include "material/material.hpp"
#include "material/material_light.hpp"
#include "material/material_utils.hpp"
#include "objects/object3d.hpp"
#include "objects/group.hpp"
#include "objects/sphere.hpp"
#include "objects/plane.hpp"
#include "objects/triangle.hpp"
#include "objects/transform.hpp"


#define DegreesToRadians(x) ((M_PI * x) / 180.0f)

SceneParser::SceneParser(const char *filename) {

    std::cout << "Parsing scene..." << std::endl;
    auto begin = std::chrono::high_resolution_clock::now();

    // initialize some reasonable default values
    group = nullptr;
    lightGroup = new Group();
    camera = nullptr;
    background_color = Vector3f(0.0f);
    num_lights = 0;
    lights = nullptr;
    num_materials = 0;
    materials = nullptr;
    current_material = nullptr;

    // parse the file
    assert(filename != nullptr);
    const char *ext = &filename[strlen(filename) - 4];

    if (strcmp(ext, ".txt") != 0) {
        printf("wrong file name extension\n");
        exit(0);
    }
    file = fopen(filename, "r");

    if (file == nullptr) {
        printf("cannot open scene file\n");
        exit(0);
    }
    parseFile();
    fclose(file);
    file = nullptr;

    if (num_lights == 0) {
        printf("WARNING:    No lights specified\n");
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    if (duration > 1000) {
        std::cout << "Scene parsed in " << duration / 1000.0 << "s" << std::endl;
    } else {
        std::cout << "Scene parsed in " << duration << "ms" << std::endl;
    }
}

SceneParser::~SceneParser() {

    delete group;
    delete lightGroup;
    delete camera;

    int i;
    for (i = 0; i < num_materials; i++) {
        delete materials[i];
    }
    delete[] materials;
}

// ====================================================================
// ====================================================================

void SceneParser::parseFile() {
    //
    // at the top level, the scene can have a camera, 
    // background color and a group of objects
    // (we add lights and other things in future assignments)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    while (getToken(token)) {
        if (!strcmp(token, "PerspectiveCamera")) {
            parsePerspectiveCamera();
        } else if (!strcmp(token, "Background")) {
            parseBackground();
        } else if (!strcmp(token, "Materials")) {
            parseMaterials();
        } else if (!strcmp(token, "Lights")) {
            parseLights();
        } else if (!strcmp(token, "Group")) {
            group = parseGroup();
        } else {
            printf("Unknown token in parseFile: '%s'\n", token);
            exit(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser::parsePerspectiveCamera() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the camera parameters
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "center"));
    Vector3f center = readVector3f();
    getToken(token);
    assert (!strcmp(token, "direction"));
    Vector3f direction = readVector3f();
    getToken(token);
    assert (!strcmp(token, "up"));
    Vector3f up = readVector3f();
    getToken(token);
    assert (!strcmp(token, "angle"));
    float angle_degrees = readFloat();
    float angle_radians = DegreesToRadians(angle_degrees);
    getToken(token);
    assert (!strcmp(token, "width"));
    int width = readInt();
    getToken(token);
    assert (!strcmp(token, "height"));
    int height = readInt();

    float aperture = 0.0f, focalLength = 1.0f;
    getToken(token);
    if (!strcmp(token, "aperture")) {
        aperture = readFloat();
        getToken(token);
        assert (!strcmp(token, "focalLength"));
        focalLength = readFloat();
        getToken(token);
    }

    assert (!strcmp(token, "}"));
    camera = new PerspectiveCamera(center, direction, up, width, height, angle_radians, aperture, focalLength);
}

void SceneParser::parseBackground() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    // read in the background color
    getToken(token);
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (!strcmp(token, "}")) {
            break;
        } else if (!strcmp(token, "color")) {
            background_color = readVector3f();
        } else {
            printf("Unknown token in parseBackground: '%s'\n", token);
            assert(0);
        }
    }
}

// ====================================================================
// ====================================================================

void SceneParser::parseLights() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in the number of objects
    getToken(token);
    assert (!strcmp(token, "numLights"));
    num_lights = readInt();
    lights = new Light *[num_lights];
    // read in the objects
    int count = 0;
    while (num_lights > count) {
        getToken(token);
        if (!strcmp(token, "MaterialIndex")) {
            // change the current material
            int index = readInt();
            assert (index >= 0 && index <= getNumMaterials());
            current_material = getMaterial(index);
        } else {
            if (strcmp(token, "DirectionalLight") == 0) {
                lights[count] = parseDirectionalLight();
            } else if (strcmp(token, "PointLight") == 0) {
                lights[count] = parsePointLight();
            } else if (strcmp(token, "AreaLight") == 0) {
                Object3D *object = parseAreaLight();
                lightGroup->addObject(-1, object);
            } else {
                printf("Unknown token in parseLight: '%s'\n", token);
                exit(0);
            }
            count++;
        }
    }
    getToken(token);
    assert (!strcmp(token, "}"));
}

Light *SceneParser::parseDirectionalLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "direction"));
    Vector3f direction = readVector3f();
    getToken(token);
    assert (!strcmp(token, "color"));
    Vector3f color = readVector3f();
    getToken(token);
    assert (!strcmp(token, "}"));
    return new DirectionalLight(direction, color);
}

Light *SceneParser::parsePointLight() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "position"));
    Vector3f position = readVector3f();
    getToken(token);
    assert (!strcmp(token, "color"));
    Vector3f color = readVector3f();
    getToken(token);
    assert (!strcmp(token, "}"));
    return new PointLight(position, color);
}

Object3D *SceneParser::parseAreaLight() { // ONLY SUPPORT (Transform +) Triangle / TinyObj 
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));

    bool isSingleSided = true;
    getToken(token);
    if (!strcmp(token, "DoubleSided")) {
        isSingleSided = false;
        getToken(token);
    }

    assert (!strcmp(token, "emitColor"));
    Vector3f emitColor = readVector3f();

    MaterialLight *material = new MaterialLight(emitColor);
    assert (current_material != nullptr);
    material->setBaseMaterial(current_material);
    Material *current_material_backup = current_material;
    current_material = material;

    Object3D *object = nullptr;
    getToken(token);
    if (!strcmp(token, "Transform")) {
        object = parseTransform(true, isSingleSided);
    } else {
        if (!strcmp(token, "Triangle")) {
            object = parseTriangle(Matrix4f::identity());
        } else if (!strcmp(token, "TinyObj")) {
            object = parseTinyObj(Matrix4f::identity(), isSingleSided);
        } else {
            printf("Arealight can only be Triangle or TinyObj, unknown token in parseAreaLight: '%s'\n", token);
            exit(0);
        }
    }
    current_material = current_material_backup;

    getToken(token);
    assert (!strcmp(token, "}"));
    return object;
}

// ====================================================================
// ====================================================================

void SceneParser::parseMaterials() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in the number of objects
    getToken(token);
    assert (!strcmp(token, "numMaterials"));
    num_materials = readInt();
    materials = new Material *[num_materials];
    // read in the objects
    int count = 0;
    while (num_materials > count) {
        getToken(token);
        materials[count] = parseMaterial(token);
        count++;
    }
    getToken(token);
    assert (!strcmp(token, "}"));
}


Material *SceneParser::parseMaterial(const char *typeName) {
    char token[MAX_PARSER_TOKEN_LENGTH];
    char textureFilename[MAX_PARSER_TOKEN_LENGTH], normalTextureFilename[MAX_PARSER_TOKEN_LENGTH];
    textureFilename[0] = 0, normalTextureFilename[0] = 0;
    Vector3f diffuseColor(1.0f), specularColor(0.0f), transmissiveColor(1.0f), F0(0.0f);;
    float shininess = 0.0f, refractionIndex = 1.0f, roughness = 0.0f;
    getToken(token);
    assert (!strcmp(token, "{"));
    while (true) {
        getToken(token);
        if (strcmp(token, "diffuseColor") == 0) {
            diffuseColor = readVector3f();
        } else if (strcmp(token, "specularColor") == 0) {
            specularColor = readVector3f();
        } else if (strcmp(token, "shininess") == 0) {
            shininess = readFloat();
        } else if (strcmp(token, "transmissiveColor") == 0) {
            transmissiveColor = readVector3f();
        } else if (strcmp(token, "refractionIndex") == 0) {
            refractionIndex = readFloat();
        } else if (strcmp(token, "roughness") == 0) {
            roughness = readFloat();
        } else if (strcmp(token, "F0") == 0) {
            F0 = readVector3f();
        } else if (strcmp(token, "texture") == 0) {
            getToken(textureFilename);
        } else if (strcmp(token, "normalTexture") == 0) {
            getToken(normalTextureFilename);
        } else {
            assert (!strcmp(token, "}"));
            break;
        }
    }
    auto *answer = createMaterial(typeName, 
        (std::string) textureFilename, (std::string) normalTextureFilename,
        diffuseColor, specularColor, shininess, 
        transmissiveColor, refractionIndex, 
        roughness, F0
    );
    return answer;
}

// ====================================================================
// ====================================================================

Object3D *SceneParser::parseObject(char token[MAX_PARSER_TOKEN_LENGTH]) {
    Object3D *answer = nullptr;
    if (!strcmp(token, "Group")) {
        answer = (Object3D *) parseGroup();
    } else if (!strcmp(token, "Sphere")) {
        answer = (Object3D *) parseSphere();
    } else if (!strcmp(token, "Plane")) {
        answer = (Object3D *) parsePlane();
    } else if (!strcmp(token, "Triangle")) {
        answer = (Object3D *) parseTriangle(Matrix4f::identity());
    } else if (!strcmp(token, "Transform")) {
        answer = (Object3D *) parseTransform(false, true);
    } else if (!strcmp(token, "TinyObj")) {
        answer = (Object3D *) parseTinyObj(Matrix4f::identity(), true);
    } else {
        printf("Unknown token in parseObject: '%s'\n", token);
        exit(0);
    }
    return answer;
}

// ====================================================================
// ====================================================================

Group *SceneParser::parseGroup() {
    //
    // each group starts with an integer that specifies
    // the number of objects in the group
    //
    // the material index sets the material of all objects which follow,
    // until the next material index (scoping for the materials is very
    // simple, and essentially ignores any tree hierarchy)
    //
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));

    // read in the number of objects
    getToken(token);
    assert (!strcmp(token, "numObjects"));
    int num_objects = readInt();

    auto *answer = new Group(num_objects);

    // read in the objects
    int count = 0;
    while (num_objects > count) {
        getToken(token);
        if (!strcmp(token, "MaterialIndex")) {
            // change the current material
            int index = readInt();
            assert (index >= 0 && index <= getNumMaterials());
            current_material = getMaterial(index);
        } else {
            Object3D *object = parseObject(token);
            assert (object != nullptr);
            answer->addObject(count, object);

            count++;
        }
    }
    getToken(token);
    assert (!strcmp(token, "}"));

    // return the group
    return answer;
}

// ====================================================================
// ====================================================================

Sphere *SceneParser::parseSphere() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "center"));
    Vector3f center = readVector3f();
    getToken(token);
    assert (!strcmp(token, "radius"));
    float radius = readFloat();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    return new Sphere(center, radius, current_material);
}


Plane *SceneParser::parsePlane() {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "normal"));
    Vector3f normal = readVector3f();
    getToken(token);
    assert (!strcmp(token, "offset"));
    float offset = readFloat();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    return new Plane(normal, offset, current_material);
}


Triangle *SceneParser::parseTriangle(const Matrix4f &M=Matrix4f::identity()) {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));
    getToken(token);
    assert (!strcmp(token, "vertex0"));
    Vector3f v0 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "vertex1"));
    Vector3f v1 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "vertex2"));
    Vector3f v2 = readVector3f();
    getToken(token);
    assert (!strcmp(token, "}"));
    assert (current_material != nullptr);
    if (M == Matrix4f::identity()) { // single triangle sure to be double-sided
        return new Triangle(v0, v1, v2, current_material, false);
    } else {                         // single triangle sure to be double-sided
        Triangle *triangle = new Triangle(v0, v1, v2, current_material, false);
        return triangle->applyTransform(M);
    }
}


Object3D *SceneParser::parseTransform(bool isAreaLight=false, bool isSingleSided=true) {
    char token[MAX_PARSER_TOKEN_LENGTH];
    Matrix4f matrix = Matrix4f::identity();
    Object3D *object = nullptr;
    getToken(token);
    assert (!strcmp(token, "{"));
    // read in transformations: 
    // apply to the LEFT side of the current matrix (so the first
    // transform in the list is the last applied to the object)
    getToken(token);

    while (true) {
        if (!strcmp(token, "Scale")) {
            Vector3f s = readVector3f();
            matrix = matrix * Matrix4f::scaling(s[0], s[1], s[2]);
        } else if (!strcmp(token, "UniformScale")) {
            float s = readFloat();
            matrix = matrix * Matrix4f::uniformScaling(s);
        } else if (!strcmp(token, "Translate")) {
            matrix = matrix * Matrix4f::translation(readVector3f());
        } else if (!strcmp(token, "XRotate")) {
            matrix = matrix * Matrix4f::rotateX(DegreesToRadians(readFloat()));
        } else if (!strcmp(token, "YRotate")) {
            matrix = matrix * Matrix4f::rotateY(DegreesToRadians(readFloat()));
        } else if (!strcmp(token, "ZRotate")) {
            matrix = matrix * Matrix4f::rotateZ(DegreesToRadians(readFloat()));
        } else if (!strcmp(token, "Rotate")) {
            getToken(token);
            assert (!strcmp(token, "{"));
            Vector3f axis = readVector3f();
            float degrees = readFloat();
            float radians = DegreesToRadians(degrees);
            matrix = matrix * Matrix4f::rotation(axis, radians);
            getToken(token);
            assert (!strcmp(token, "}"));
        } else if (!strcmp(token, "Matrix4f")) {
            Matrix4f matrix2 = Matrix4f::identity();
            getToken(token);
            assert (!strcmp(token, "{"));
            for (int j = 0; j < 4; j++) {
                for (int i = 0; i < 4; i++) {
                    float v = readFloat();
                    matrix2(i, j) = v;
                }
            }
            getToken(token);
            assert (!strcmp(token, "}"));
            matrix = matrix2 * matrix;
        } else {
            // otherwise this must be an object,
            // and there are no more transformations
            if (!isAreaLight && !!strcmp(token, "TinyObj")) {
                object = parseObject(token);

                assert(object != nullptr);
                getToken(token);
                assert (!strcmp(token, "}"));
                return new Transform(matrix, object);

            } else { // AreaLight / TinyObj need to be directly transformed
                if (!strcmp(token, "Triangle")) {
                    object = parseTriangle(matrix);
                } else if (!strcmp(token, "TinyObj")) {
                    object = parseTinyObj(matrix, isSingleSided);
                } else {
                    printf("Arealight can only be Triangle or TinyObj, unknown token in parseTransform: '%s'\n", token);
                    exit(0);
                }

                assert(object != nullptr);
                getToken(token);
                assert (!strcmp(token, "}"));
                return object;
            }
        }
        getToken(token);
    }
}


Group *SceneParser::parseTinyObj(const Matrix4f &M=Matrix4f::identity(), bool isSingleSided=true) {
    char token[MAX_PARSER_TOKEN_LENGTH];
    getToken(token);
    assert (!strcmp(token, "{"));

    bool isSmooth = false;
    getToken(token);
    if (!strcmp(token, "Smooth")) {
        isSmooth = true;
        getToken(token);
    }

    std::string inputfile = token;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "./mesh"; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(inputfile, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    assert (current_material != nullptr);
    Group *answer = new Group(attrib, shapes, materials, current_material, isSmooth, M, isSingleSided);

    getToken(token);
    assert (!strcmp(token, "}"));

    return answer;
}


// ====================================================================
// ====================================================================

int SceneParser::getToken(char token[MAX_PARSER_TOKEN_LENGTH]) {
    // for simplicity, tokens must be separated by whitespace
    assert (file != nullptr);
    int success = fscanf(file, "%s ", token);
    if (success == EOF) {
        token[0] = '\0';
        return 0;
    }
    return 1;
}


Vector3f SceneParser::readVector3f() {
    float x, y, z;
    int count = fscanf(file, "%f %f %f", &x, &y, &z);
    if (count != 3) {
        printf("Error trying to read 3 floats to make a Vector3f\n");
        assert (0);
    }
    return Vector3f(x, y, z);
}


float SceneParser::readFloat() {
    float answer;
    int count = fscanf(file, "%f", &answer);
    if (count != 1) {
        printf("Error trying to read 1 float\n");
        assert (0);
    }
    return answer;
}


int SceneParser::readInt() {
    int answer;
    int count = fscanf(file, "%d", &answer);
    if (count != 1) {
        printf("Error trying to read 1 int\n");
        assert (0);
    }
    return answer;
}
