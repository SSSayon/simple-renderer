#ifndef RAY_H
#define RAY_H

#include <iostream>
#include <Vector3f.h>


// Ray class mostly copied from Peter Shirley and Keith Morley
class Ray {
public:

    Ray() = delete;
    Ray(const Vector3f &orig, const Vector3f &dir) {
        origin = orig;
        direction = dir; // should NOT be normalized, for uses in Transform object
        invDirection = Vector3f(1.0f / direction.x(), 1.0f / direction.y(), 1.0f / direction.z());
    }

    Ray(const Ray &r) {
        origin = r.origin;
        direction = r.direction;
        invDirection = r.invDirection;
    }

    const Vector3f &getOrigin() const {
        return origin;
    }

    const Vector3f &getDirection() const {
        return direction;
    }

    const Vector3f &getInvDirection() const {
        return invDirection;
    }

    Vector3f pointAtParameter(float t) const {
        return origin + direction * t;
    }

    void originOffset() {
        origin += direction * offset;
    }

private:

    Vector3f origin;
    Vector3f direction, invDirection;

    static constexpr float offset = 6e-4f;

};

inline std::ostream &operator<<(std::ostream &os, const Ray &r) {
    os << "Ray <" << r.getOrigin() << ", " << r.getDirection() << ">";
    return os;
}

#endif // RAY_H
