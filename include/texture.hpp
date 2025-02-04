#ifndef TEXTURE_H
#define TEXTURE_H

#include <cmath>
#include <iostream>

#include "Vector2f.h"
#include "Vector3f.h"
#include <stb_image.h>


class Texture {
public:
    Texture() {
        isDefault = true;
    }
    Texture(std::string filename, bool isLinear = false) {
        data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
        if (data == nullptr) {
            std::cerr << "Failed to load texture: " << filename << std::endl;
            exit(1);
        }
        if (nrChannels != 3 && nrChannels != 4) {
            std::cerr << "Texture: " << filename << " has " << nrChannels << " channels, only 3 or 4 channels are supported." << std::endl;
            exit(1);
        }
        if (!isLinear) _sRGBToLinear();
    }

    ~Texture() {
        stbi_image_free(data);
    }

    Vector3f getColor(float u, float v) const {
        return _getColorBilinear(u, v);
    }
    Vector3f getColor(const Vector2f &uv) const {
        return getColor(uv.x(), uv.y());
    }

private:
    bool isDefault = false;
    unsigned char *data;
    int width, height, nrChannels;

    Vector3f _getColor(int x, int y) const {
        if (isDefault) return Vector3f(1.0f);

        int idx = (y * width + x) * nrChannels;

        float r = data[idx] / 255.0f;
        float g = data[idx + 1] / 255.0f;
        float b = data[idx + 2] / 255.0f;

        return Vector3f(r, g, b);
    }

    Vector3f _getColorNearestNeighbor(float u, float v) const {
        if (isDefault) return Vector3f(1.0f);

        u = std::max(0.0f, std::min(0.9999f, u));
        v = std::max(0.0f, std::min(0.9999f, 1-v));
        int x = static_cast<int>(u * width);
        int y = static_cast<int>(v * height);
        
        return _getColor(x, y);
    }
    Vector3f _getColorNearestNeighbor(const Vector2f &uv) const {
        return _getColorNearestNeighbor(uv.x(), uv.y());
    }

    Vector3f _getColorBilinear(float u, float v) const {
        if (isDefault) return Vector3f(1.0f);

        u = std::max(0.0f, std::min(0.9999f, u));
        v = std::max(0.0f, std::min(0.9999f, 1-v));

        float x = u * width;
        float y = v * height;
        int x0 = static_cast<int>(x);
        int y0 = static_cast<int>(y);
        int x1 = (x0 < width - 1) ? x0 + 1 : x0;
        int y1 = (y0 < height - 1) ? y0 + 1 : y0;

        float dx = x - x0;
        float dy = y - y0;

        Vector3f c00 = _getColor(x0, y0);
        Vector3f c10 = _getColor(x1, y0);
        Vector3f c01 = _getColor(x0, y1);
        Vector3f c11 = _getColor(x1, y1);

        Vector3f c0 = Vector3f::lerp(c00, c10, dx);
        Vector3f c1 = Vector3f::lerp(c01, c11, dx);
        return Vector3f::lerp(c0, c1, dy);
    }
    Vector3f _getColorBilinear(const Vector2f &uv) const {
        return _getColorBilinear(uv.x(), uv.y());
    }

    void _sRGBToLinear() { // convert back to LinearRGB
        for (int i = 0; i < width * height * nrChannels; i += nrChannels) {
            float r = data[i] / 255.0f;
            float g = data[i + 1] / 255.0f;
            float b = data[i + 2] / 255.0f;

            // see https://en.wikipedia.org/wiki/SRGB#Transformation
            r = (r <= 0.04045f) ? r / 12.92f : powf((r + 0.055f) / 1.055f, 2.4f);
            g = (g <= 0.04045f) ? g / 12.92f : powf((g + 0.055f) / 1.055f, 2.4f);
            b = (b <= 0.04045f) ? b / 12.92f : powf((b + 0.055f) / 1.055f, 2.4f);

            data[i] = static_cast<unsigned char>(r * 255);
            data[i + 1] = static_cast<unsigned char>(g * 255);
            data[i + 2] = static_cast<unsigned char>(b * 255);
        }
    }

};

#endif // TEXTURE_H
