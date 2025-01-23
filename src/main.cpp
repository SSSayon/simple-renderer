#include <iostream>

#include "Vector2f.h"
#include "Vector3f.h"
#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "objects/group.hpp"
#include "shader/shader_whitted.cpp"
#include "utils.hpp"

#include <omp.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Usage: ./build/final <input scene file> <output bmp file>" << std::endl;
        return 1;
    }
    std::string inputFile = argv[1];
    std::string outputFile = argv[2];  // only bmp is allowed.

    SceneParser parser(inputFile.c_str());
    std::cout << "Built scene from " << inputFile << std::endl;

    Camera *camera = parser.getCamera();
    Group *baseGroup = parser.getGroup();
    int numLights = parser.getNumLights();
    int width = camera->getWidth();
    int height = camera->getHeight();
    Image image(width, height);

    int cnt = 0;
    std::cout << "Rendering with " << omp_get_max_threads() << " threads..." << std::endl;
    #pragma omp parallel for schedule(dynamic)
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            Ray ray = camera->generateRay(Vector2f(x, y));
            Vector3f color = ShaderWhitted(ray, parser, baseGroup, 0);
            image.SetPixel(x, y, color);
        }

        #pragma omp critical
        {
            cnt++;
            showProgress((float)cnt / width);
        }
    }
    std::cout << std::endl;

    // image.LinearToSRGB();
    image.SaveBMP(("output/" + outputFile + ".bmp").c_str());
    std::cout << "Image saved to output/" << outputFile << ".bmp" << std::endl;

    return 0;
}
