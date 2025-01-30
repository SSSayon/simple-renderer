#ifndef MESH_H
#define MESH_H

#include <map>
#include <vector>
#include <random>

#include <tiny_obj_loader.h>

#include "Vector2f.h"
#include "Vector3f.h"
#include "Matrix4f.h"
#include "object3d.hpp"
#include "objects/triangle.hpp"

// NO LONGER SUPPORT direct creation (only through TinyObj's Group)
class Mesh : public Object3D {

public:
    Mesh();

    // assume the mesh is triangulated
    Mesh(const std::vector<Vector3f> &vertices, const std::vector<Vector3f> &vertexNormals, const std::vector<Vector2f> &uvs, 
         const tinyobj::shape_t &shape, Material *m, bool isSmooth=false, const Matrix4f &M=Matrix4f::identity(), bool isSingleSided=true)
    : Object3D(m) {

        const auto &indices = shape.mesh.indices;

        bool hasVertexNormals = vertexNormals.size() > 0;
        bool hasUVs = uvs.size() > 0;

        // prepare smooth normals
        std::map<int, Vector3f> smoothNormals;
        if (isSmooth && !hasVertexNormals) {
            for (int idx = 0; idx < indices.size(); idx += 3) {
                Vector3f v0 = vertices[indices[idx].vertex_index];
                Vector3f v1 = vertices[indices[idx + 1].vertex_index];
                Vector3f v2 = vertices[indices[idx + 2].vertex_index];

                Vector3f normal = Vector3f::cross(v1 - v0, v2 - v0).normalized();
                float area = 0.5f * Vector3f::cross(v1 - v0, v2 - v0).length();

                for (int i = 0; i < 3; i++) {
                    smoothNormals[indices[idx + i].vertex_index] += normal * area;
                }
            }
            for (auto &pair: smoothNormals) {
                pair.second.normalize();
            }
        }

        // write into triangles
        for (int idx = 0; idx < indices.size(); idx += 3) {
            Vector3f v[3], vn[3];
            Vector2f vt[3];
            for (int i = 0; i < 3; i++) {
                v[i] = vertices[indices[idx + i].vertex_index];
                if (hasVertexNormals) {
                    vn[i] = vertexNormals[indices[idx + i].normal_index];
                } else if (isSmooth) {
                    vn[i] = smoothNormals[indices[idx + i].vertex_index];
                } else {
                    vn[i] = Vector3f::ZERO;
                }
                if (hasUVs) {
                    vt[i] = uvs[indices[idx + i].texcoord_index];
                } else {
                    vt[i] = Vector2f::ZERO;
                }
            }
            Triangle *triangle = new Triangle(v[0], v[1], v[2], m,
                                             vn[0], vn[1], vn[2],
                                             vt[0], vt[1], vt[2],
                                             isSingleSided);
            if (M != Matrix4f::identity()) {
                triangle = triangle->applyTransform(M);
            }
            triangles.push_back(triangle);
        }
    }

    ~Mesh() {
        for (auto triangle: triangles) { delete triangle; }
    }

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        bool result = false;
        for (auto triangle: triangles) {
            result |= triangle->intersect(r, h, tmin);
        }
        return result;
    };
    bool intersect(const Ray &r, Hit &h, float tmin, float &pdf) override {
        bool result = false;
        for (auto triangle: triangles) {
            result |= triangle->intersect(r, h, tmin, pdf);
        }
        if (result) {
            pdf /= triangles.size();
        }
        return result;
    }

    Vector3f samplePoint(std::mt19937 &rng, float &pdf) override {
        std::uniform_int_distribution<int> dist(0, triangles.size() - 1);
        Vector3f p = triangles[dist(rng)]->samplePoint(rng, pdf);
        pdf /= triangles.size();
        return p;
    }

private:
    std::vector<Triangle*> triangles;
};

#endif
