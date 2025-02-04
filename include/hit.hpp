#ifndef HIT_H
#define HIT_H

#include "Vector2f.h"
#include "Vector3f.h"
#include <iostream>
#include <vecmath.h>

class Material;

class Hit {
public:

    // constructors
    Hit() {
        material = nullptr;
        t = 1e38;
    }

    Hit(float _t, Material *m, const Vector3f &n, const Vector3f &emitN=Vector3f::ZERO, const Vector2f &_uv=Vector2f::MINUS_ONE) {
        t = _t;
        material = m;
        normal = n;
        emitNormal = emitN; // ONLY used for double-sided, single-emission triangle area light
        uv = _uv;           // ONLY used for triangle texture mapping
    }

    Hit(const Hit &h) {
        t = h.t;
        material = h.material;
        normal = h.normal;
        emitNormal = h.emitNormal;
        uv = h.uv;
    }

    // destructor
    ~Hit() = default;

    float getT() const {
        return t;
    }

    Material *getMaterial() const {
        return material;
    }

    const Vector3f &getNormal() const {
        return normal;
    }

    const Vector3f &getEmitNormal() const {
        return emitNormal;
    }

    const Vector2f &getUV() const {
        return uv;
    }

    void set(float _t, Material *m, const Vector3f &n, const Vector3f &emitN=Vector3f::ZERO, const Vector2f &_uv=Vector2f::MINUS_ONE) {
        t = _t;
        material = m;
        normal = n;
        emitNormal = emitN;
        uv = _uv;
    }

private:
    float t;
    Material *material;
    Vector3f normal;
    Vector3f emitNormal;
    Vector2f uv;
};

inline std::ostream &operator<<(std::ostream &os, const Hit &h) {
    os << "Hit <" << h.getT() << ", " << h.getNormal() << ">";
    return os;
}

#endif // HIT_H
