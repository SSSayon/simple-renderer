#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "Matrix3f.h"
#include "Matrix4f.h"
#include "Vector2f.h"
#include "Vector3f.h"
#include "object3d.hpp"
#include <cassert>
#include <vecmath.h>
#include <cmath>

static Vector3f transVertex(const Matrix4f &mat, const Vector3f &point) {
    return (mat * Vector4f(point, 1)).xyz();
}

static Vector3f transNormal(const Matrix4f &mat, const Vector3f &dir) {
    return (mat * Vector4f(dir, 0)).xyz().normalized();
}

class Triangle: public Object3D {

public:
    Triangle() = delete;

    // a b c are three vertex positions of the triangle
    Triangle(const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m,
             bool isSingleSided = true)
    : Object3D(m) {

        v1 = a; v2 = b; v3 = c;
        vn1 = Vector3f::ZERO; vn2 = Vector3f::ZERO; vn3 = Vector3f::ZERO;
        uv1 = Vector2f::MINUS_ONE; uv2 = Vector2f::MINUS_ONE; uv3 = Vector2f::MINUS_ONE;

        normal = Vector3f::cross(v2 - v1, v3 - v1);
        area = 0.5f * normal.length();
        normal = normal / (2.0f * area);

        this->isSingleSided = isSingleSided;
    }

    Triangle(const Vector3f& a, const Vector3f& b, const Vector3f& c, Material* m,
             const Vector3f& na, const Vector3f& nb, const Vector3f& nc,
             const Vector2f& ta, const Vector2f& tb, const Vector2f& tc,
             bool isSingleSided = true)
    : Triangle(a, b, c, m, isSingleSided) {
        vn1 = na; vn2 = nb; vn3 = nc;
        uv1 = ta; uv2 = tb; uv3 = tc;

        // normal mapping, precompute TBN matrix
        // reference: https://learnopengl.com/Advanced-Lighting/Normal-Mapping
        if (!m->getNormalTextureFilename().empty()) {
            assert(uv1 != Vector2f::MINUS_ONE);
            Vector3f e1 = v2 - v1;
            Vector3f e2 = v3 - v1;
            Vector2f deltaUV1 = uv2 - uv1;
            Vector2f deltaUV2 = uv3 - uv1;

            Vector3f tangent, bitangent;
            float det = deltaUV1.x() * deltaUV2.y() - deltaUV1.y() * deltaUV2.x();
            if (fabs(det) < EPStriangle) { // degenerate triangle
                tangent = Vector3f(1.0f, 0.0f, 0.0f);
                bitangent = Vector3f(0.0f, 1.0f, 0.0f);
            } else {
                float invDet = 1.0f / det;
                /*
                    e1 = deltaUV1.x() * tangent + deltaUV1.y() * bitangent
                    e2 = deltaUV2.x() * tangent + deltaUV2.y() * bitangent

                    solve the above equations to get tangent and bitangent
                */
                tangent = (deltaUV2.y() * e1 - deltaUV1.y() * e2) * invDet;
                // bitangent = (deltaUV1.x() * e2 - deltaUV2.x() * e1) * invDet;

                // Gram-Schmidt orthogonalize
                tangent = (tangent - Vector3f::dot(tangent, normal) * normal).normalized();
                bitangent = Vector3f::cross(normal, tangent);
            }
            TBN = Matrix3f(tangent, bitangent, normal);
        }
    }

    Triangle *applyTransform(const Matrix4f &M) {
        if (vn1 != Vector3f::ZERO || vn2 != Vector3f::ZERO || vn3 != Vector3f::ZERO) {
            Matrix4f M_inv_T = M.inverse().transposed();
            return new Triangle(transVertex(M, v1), transVertex(M, v2), transVertex(M, v3), material,
                                transNormal(M_inv_T, vn1), transNormal(M_inv_T, vn2), transNormal(M_inv_T, vn3),
                                uv1, uv2, uv3, isSingleSided);
        } else {
            return new Triangle(transVertex(M, v1), transVertex(M, v2), transVertex(M, v3), material,
                                Vector3f::ZERO, Vector3f::ZERO, Vector3f::ZERO,
                                uv1, uv2, uv3, isSingleSided);
        }
    }

    float getArea() const {
        return area;
    }

    // Moller–Trumbore intersection algorithm, see https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
    bool intersect(const Ray& r,  Hit& h , float tmin) const override {
        Vector3f o = r.getOrigin();
        Vector3f d = r.getDirection();

        Vector3f e1 = v2 - v1;
        Vector3f e2 = v3 - v1;

        Vector3f d_cross_e2 = Vector3f::cross(d, e2);
        float det = Vector3f::dot(e1, d_cross_e2);

        if (fabs(det) < EPStriangle) return false;

        float inv_det = 1 / det;

        Vector3f o_minus_v1 = o - v1;
        float u = inv_det * Vector3f::dot(o_minus_v1, d_cross_e2);
        if (u < 0 || u > 1) return false;

        Vector3f o_minus_v1_cross_e1 = Vector3f::cross(o_minus_v1, e1);
        float v = inv_det * Vector3f::dot(d, o_minus_v1_cross_e1);
        if (v < 0 || u + v > 1) return false;

        float t = inv_det * Vector3f::dot(e2, o_minus_v1_cross_e1);
        if (t > h.getT() || t < tmin) return false;

        Vector2f uv = Vector2f::MINUS_ONE;
        if (uv1 != Vector2f::MINUS_ONE) {
            uv = (1 - u - v) * uv1 + u * uv2 + v * uv3;
        }

        Vector3f n = normal;
        if (!material->getNormalTextureFilename().empty()) {                                // normal mapping
            Vector3f tangentNormal = material->getNormalTexture()->getColor(uv);
            tangentNormal = 2.0f * tangentNormal - Vector3f(1.0f);
            n = TBN * tangentNormal;
            n.normalize();
        }
        else if (vn1 != Vector3f::ZERO || vn2 != Vector3f::ZERO || vn3 != Vector3f::ZERO) { // normal interpolation
            n = (1 - u - v) * vn1 + u * vn2 + v * vn3;
            n.normalize();
        }

        if(isSingleSided) {
            h.set(t, material, n, n, uv);
        } else {
            if(Vector3f::dot(-d, n) > 0) {
                h.set(t, material, n, n, uv);
            } else {
                h.set(t, material, -n, n, uv);
            }
        }
        return true;
    }
    bool intersect(const Ray& r,  Hit& h , float tmin, float &pdf) override {
        bool result = intersect(r, h, tmin);
        if (result) {
            pdf = 1.0f / area;
        }
        return result;
    }

    Vector3f samplePoint(std::mt19937 &rng, float &pdf) override {
        std::uniform_real_distribution<float> dist(0, 1);
        float u = dist(rng);
        float v = dist(rng);
        if (u + v > 1.0f) {
            u = 1.0f - u;
            v = 1.0f - v;
        }
        pdf = 1.0f / area;
        return (1.0f - u - v) * v1 + u * v2 + v * v3;
    }

    AABB *getAABB() const override {
        if (aabb != nullptr) return aabb;

        Vector3f pMin = Vector3f::min(v1, Vector3f::min(v2, v3));
        Vector3f pMax = Vector3f::max(v1, Vector3f::max(v2, v3));
        aabb = new AABB(pMin - Vector3f(AABB::EPSaabb), pMax + Vector3f(AABB::EPSaabb));
        return aabb;
    }

    std::vector<const Object3D*> getObjects() const override {
        return {this};
    }

protected:
    
    Vector3f v1, v2, v3;       // vertex
    Vector3f vn1, vn2, vn3;    // vertex normal
    Vector2f uv1, uv2, uv3;    // vertex texture coordinate

    Vector3f normal;           // face normal
    Matrix3f TBN;              // TBN matrix

    bool isSingleSided = true; // single-sided or double-sided
                               // you can image a double-sided triangle has very, very thin thickness
    float area;
    
    static constexpr float EPStriangle = 1e-8f;
};

#endif // TRIANGLE_H
