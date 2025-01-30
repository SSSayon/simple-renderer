#ifndef CAMERA_H
#define CAMERA_H

#include <cmath>
#include <random>

#include "Vector2f.h"
#include "Vector3f.h"
#include "Matrix3f.h"
#include "ray.hpp"

class Camera {
public:
    Camera(const Vector3f &center, const Vector3f &direction, const Vector3f &up, int imgW, int imgH) {
        this->center = center;
        this->direction = direction.normalized();
        this->horizontal = Vector3f::cross(this->direction, up).normalized();
        this->up = Vector3f::cross(this->horizontal, this->direction);
        this->width = imgW;
        this->height = imgH;
    }

    // Generate rays for each screen-space coordinate
    virtual Ray generateRay(int x, int y, std::mt19937 &rng) = 0;
    virtual ~Camera() = default;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

protected:
    // Extrinsic parameters
    Vector3f center;
    Vector3f direction;
    Vector3f up;
    Vector3f horizontal;
    // Intrinsic parameters
    int width;
    int height;
};

class PerspectiveCamera : public Camera {

public:
    PerspectiveCamera(const Vector3f &center, const Vector3f &direction,
            const Vector3f &up, int imgW, int imgH, float angle,
            float aperture = 0.0f, float focalLength = 1.0f)
    : Camera(center, direction, up, imgW, imgH) {
        cx = imgW / 2.0f;
        cy = imgH / 2.0f;
        fx = fy = cy / std::tan(angle / 2.0f);
        cameraToWorld = Matrix3f(this->horizontal, -this->up, this->direction);

        this->aperture = aperture;
        this->focalLength = focalLength;
    }

    Ray generateRay(int x, int y, std::mt19937 &rng) override {

        // anti-aliasing
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        // float dx = dist(rng), dy = dist(rng);
        float dx = 0.5f, dy = 0.5f;

        Vector3f cameraDir = Vector3f((x + dx - cx) / fx, (cy - y - dy) / fy, 1).normalized();
        Vector3f rayDir = cameraToWorld * cameraDir;

        if (aperture == 0.0f) {
            return Ray(center, rayDir);

        } else { // depth of field, see https://stackoverflow.com/questions/13532947/references-for-depth-of-field-implementation-in-a-raytracer
            float r = sqrtf(dist(rng)) * aperture;
            float theta = dist(rng) * 2 * M_PI;
            Vector3f origin = center + r * (cos(theta) * horizontal + sin(theta) * -up); // i.e, cameraToWorld * (r cos, r sin, 0)^T

            Vector3f focalPoint = center + focalLength * rayDir; // on the focal "plane" (a spherical shell)

            rayDir = (focalPoint - origin).normalized();
            return Ray(origin, rayDir);
        }
    }

protected:
    float cx;
    float cy;
    float fx;
    float fy;
    Matrix3f cameraToWorld;

    // for DOF
    float aperture;
    float focalLength;
};

#endif //CAMERA_H
