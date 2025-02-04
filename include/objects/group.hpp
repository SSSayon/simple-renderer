#ifndef GROUP_H
#define GROUP_H

#include <cassert>
#include <random>
#include <vector>

#include <tiny_obj_loader.h>

#include "Vector2f.h"
#include "Vector3f.h"
#include "hit.hpp"
#include "object3d.hpp"
#include "objects/mesh.hpp"
#include "material/material_utils.hpp"
#include "accel/bvh.hpp"

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
          Material *m, bool isSmooth=false, const Matrix4f &M=Matrix4f::identity(), bool isSingleSided=true) {

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
            else material = createTinyMaterial(m->getTypeName(), 
                                               m->getTextureFilename(), 
                                               m->getNormalTextureFilename(), 
                                               materials[shape.mesh.material_ids[0]]);
                // actually the 0 above is idx/3 (w.r.t. each triangle)
                // but it will be space-consuming, and it is often the case that
                // the whole mesh has the *same* material, which we'll assume here.
            Mesh *mesh = new Mesh(vertices, vertexNormals, uvs, shape, material, isSmooth, M, isSingleSided);
            addObject(-1, mesh);
        }
    }

    ~Group() override {
        for (Object3D* obj : objects) {
            delete obj;
        }
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

    bool intersect(const Ray &r, Hit &h, float tmin) const override {
        if (bvh) {
            return bvh->intersect(r, h, tmin);
        }
        bool result = false;
        for(auto obj : objects) {
            result |= obj->intersect(r, h, tmin);
        }
        return result;
    }
    bool intersect(const Ray &r, Hit &h, float tmin, float &pdf) override {
        bool result = false;
        for(auto obj : objects) {
            result |= obj->intersect(r, h, tmin, pdf);
        }
        if (result) {
            pdf /= objects.size();
        }
        return result;
    }

    AABB *getAABB() const override {
        std::cerr << "Group::getAABB method should NOT be called!" << std::endl;
        exit(1);
    }

    std::vector<const Object3D*> getObjects() const override {
        std::vector<const Object3D*> constObjects;
        for (const auto obj : objects) {
            for (auto o : obj->getObjects()) {
                constObjects.push_back(o);
            }
        }
        return constObjects;
    }

    Object3D *sampleObject(std::mt19937 &rng, float &pdf) {
        pdf = 1.0f / objects.size();
        std::uniform_int_distribution<int> dist(0, objects.size() - 1);
        return objects[dist(rng)];
    }

    Vector3f samplePoint(std::mt19937 &rng, float &pdf) override {
        std::uniform_int_distribution<int> dist(0, objects.size() - 1);
        Vector3f p = objects[dist(rng)]->samplePoint(rng, pdf);
        pdf /= objects.size();
        return p;
    }

    void buildBVH() {
        std::vector<BVHNode*> nodes;
        bvh = new BVH(getObjects());
    }

private:
    std::vector<Object3D*> objects;
    BVH *bvh = nullptr;
};

#endif // GROUP_H
