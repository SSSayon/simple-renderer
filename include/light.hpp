#ifndef LIGHT_H
#define LIGHT_H

#include <Vector3f.h>

// ONLY FOR Whitted-style ray casting
class Light {
public:
    Light() = default;
    virtual ~Light() = default;

    /// check if the ray from p to the light source is blocked by any object
    /// @param p the origin of the ray
    /// @param isec the intersection point of the ray
    virtual bool isBlocked(const Vector3f &p, const Vector3f isec) const = 0;

    virtual void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const = 0;
};


class DirectionalLight : public Light {
public:
    DirectionalLight() = delete;

    DirectionalLight(const Vector3f &d, const Vector3f &c) {
        direction = d.normalized();
        color = c;
    }

    ~DirectionalLight() override = default;

    bool isBlocked(const Vector3f &p, const Vector3f isec) const override {
        return true;
    }

    ///@param p unused in this function
    ///@param distanceToLight not well defined because it's not a point light
    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override {
        // the direction to the light is the opposite of the
        // direction of the directional light source
        dir = -direction;
        col = color;
    }

private:

    Vector3f direction;
    Vector3f color;

};

class PointLight : public Light {
public:
    PointLight() = delete;

    PointLight(const Vector3f &p, const Vector3f &c) {
        position = p;
        color = c;
    }

    ~PointLight() override = default;

    bool isBlocked(const Vector3f &p, const Vector3f isec) const override {
        return Vector3f::dot(isec - position, p - position) > 1e-4f;
    }

    void getIllumination(const Vector3f &p, Vector3f &dir, Vector3f &col) const override {
        dir = (position - p).normalized();
        col = color;
    }

private:

    Vector3f position;
    Vector3f color;

};

#endif // LIGHT_H
