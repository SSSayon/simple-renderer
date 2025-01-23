#ifndef GROUP_H
#define GROUP_H

#include <cassert>
#include <vector>

#include <tiny_obj_loader.h>

#include "Vector2f.h"
#include "Vector3f.h"
#include "hit.hpp"
#include "object3d.hpp"
#include "objects/mesh.hpp"

// TODO: Implement Group - add data structure to store a list of Object*
class Group : public Object3D {

public:

    Group() {}

    explicit Group(int num_objects) {
        objects = std::vector<Object3D*>(num_objects); 
    }

    Group(const tinyobj::attrib_t &attrib, 
          const std::vector<tinyobj::shape_t> &shapes, 
          const std::vector<tinyobj::material_t> &materials,
          Material *m, bool isSmooth = false) {
        
        // prepare vertices, vertex normals, and uvs
        std::vector<Vector3f> vertices, vertexNormals;
        std::vector<Vector2f> uvs;
        for (int i = 0; i < attrib.vertices.size(); i += 3) {
            vertices.emplace_back(Vector3f(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]));
        }
        for (int i = 0; i < attrib.normals.size(); i += 3) {
            vertexNormals.emplace_back(Vector3f(attrib.normals[i], attrib.normals[i + 1], attrib.normals[i + 2]));
        }
        for (int i = 0; i < attrib.texcoords.size(); i += 2) {
            uvs.emplace_back(Vector2f(attrib.texcoords[i], attrib.texcoords[i + 1]));
        }

        for (const auto &shape: shapes) {
            Material *material;
            if (materials.size() == 0) material = m;
            else material = m->createTinyMaterial(materials[shape.mesh.material_ids[0]]);
                // actually the 0 above is idx/3 (w.r.t. each triangle)
                // but it will be space-consuming, and it is often the case that
                // the whole mesh has the *same* material, which we'll assume here.
            Mesh *mesh = new Mesh(vertices, vertexNormals, uvs, shape, material, isSmooth);
            addObject(-1, mesh);
        }
    }

    ~Group() override {
        for (Object3D* obj : objects) {
            delete obj;
        }
    }

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        bool isHit = false;
        for(unsigned long i = 0; i < objects.size(); ++i) {
            isHit |= objects[i]->intersect(r, h, tmin);
        }
        return isHit;
    }

    void addObject(int index, Object3D *obj) {
        assert(index >= -1 && index < getGroupSize());
        if (index == -1) {
            objects.push_back(obj); 
        } else {
            objects[index] = obj;
        }
    }

    int getGroupSize() {
        return objects.size();
    }

private:
    std::vector<Object3D*> objects;
};

#endif // GROUP_H
