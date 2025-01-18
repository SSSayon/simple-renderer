#ifndef CAMERA_H
#define CAMERA_H

#include "ray.hpp"
#include <vecmath.h>
#include <cmath>


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
    virtual Ray generateRay(const Vector2f &point) = 0;
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
            const Vector3f &up, int imgW, int imgH, float angle) : Camera(center, direction, up, imgW, imgH) {
        cx = imgW / 2.0f;
        cy = imgH / 2.0f;
        fx = fy = cy / std::tan(angle / 2.0f);
        camera_to_world = Matrix3f(this->horizontal, -this->up, this->direction);
    }

    Ray generateRay(const Vector2f &point) override {
        Vector3f d_camera = Vector3f((point[0] - cx) / fx, (cy - point[1]) / fy, 1).normalized();
        return Ray(center, camera_to_world * d_camera); // norm sure to be 1
    }

protected:
    float cx;
    float cy;
    float fx;
    float fy;
    Matrix3f camera_to_world;
};

#endif //CAMERA_H
