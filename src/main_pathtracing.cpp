#include <iostream>
#include <random>
#include <chrono>
#include <string>
#include <functional>

#include "Vector2f.h"
#include "Vector3f.h"
#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "objects/group.hpp"
#include "shader/shader_naive.cpp"
#include "shader/shader_nee.cpp"
#include "shader/shader_mis.cpp"
#include "utils.hpp"

#include <omp.h>

using ShaderFunc = std::function<Vector3f(Ray, Group*, Group*, const Vector3f&, std::mt19937&)>;

ShaderFunc selectShader(const std::string &description) {
    if (description.find("mis") != std::string::npos) {
        return ShaderMIS;
    } else if (description.find("nee") != std::string::npos) {
        return ShaderNEE;
    } else {
        return ShaderNaive;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 5) {
        std::cout << "Usage: ./build/pathtracing <input scene file> <output bmp file> <description> <SPP>(optional)" << std::endl;
        return 1;
    }
    std::string inputFile = argv[1];
    std::string outputFile = argv[2];  // only bmp is allowed.
    std::string description = argv[3];
    int SPP = 16;
    if (argc == 5) SPP = std::stoi(argv[4]);

    SceneParser parser(inputFile.c_str());
    std::cout << "Built scene from " << inputFile << std::endl;

    Camera *camera = parser.getCamera();
    Group *baseGroup = parser.getGroup();
    Group *lightGroup = parser.getLightGroup();
    Vector3f backgroundColor = parser.getBackgroundColor();
    int width = camera->getWidth();
    int height = camera->getHeight();
    Image image(width, height);

    ShaderFunc shaderFunc = selectShader(description);

    std::cout << "Rendering with " << omp_get_max_threads() << " threads..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    int cnt = 0;
    #pragma omp parallel for schedule(dynamic)
    for (int s = 0; s < SPP; ++s) {
        std::mt19937 rng(s);

        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {

                Ray ray = camera->generateRay(x, y, rng);
                Vector3f color = shaderFunc(ray, baseGroup, lightGroup, backgroundColor, rng);
                image.AddPixel(x, y, color);
            }

            #pragma omp atomic
            cnt++;
            #pragma omp critical
            showProgress((float)cnt / (SPP * width));
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    showProgress(1.0f);
    std::cout << std::endl << "Rendering time: " << duration / 1000.0 << "s" << std::endl;

    image.divideAllPixels(SPP);
    image.LinearToSRGB();
    std::string filename = "output/" + outputFile 
                         + "_" + std::to_string(SPP) + "spp"
                         + "_" + std::to_string((int)duration / 1000) + "s"
                         + "_" + description + ".bmp";
    image.SaveBMP(filename.c_str());
    std::cout << "Image saved to " << filename << std::endl;

    return 0;
}
