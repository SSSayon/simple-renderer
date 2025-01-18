#include <iostream>

#include "Vector2f.h"
#include "Vector3f.h"
#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"
#include "light.hpp"

using namespace std;

int main(int argc, char *argv[]) {
    for (int argNum = 1; argNum < argc; ++argNum) {
        std::cout << "Argument " << argNum << " is: " << argv[argNum] << std::endl;
    }

    if (argc != 3) {
        cout << "Usage: ./bin/PA1 <input scene file> <output bmp file>" << endl;
        return 1;
    }
    string inputFile = argv[1];
    string outputFile = argv[2];  // only bmp is allowed.

    SceneParser parser(inputFile.c_str());
    Camera *camera = parser.getCamera();
    Group *baseGroup = parser.getGroup();
    int numLights = parser.getNumLights();
    int width = camera->getWidth();
    int height = camera->getHeight();
    Image image(width, height);

    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            Ray ray = camera->generateRay(Vector2f(x, y));
            Hit hit;
            bool isIntersect = baseGroup->intersect(ray, hit, 0);
            if (isIntersect) {
                Vector3f finalColor = Vector3f::ZERO;
                for (int li = 0; li < numLights; ++li) {
                    Light *light = parser.getLight(li);
                    Vector3f L, lightColor;
                    light->getIllumination(ray.pointAtParameter(hit.getT()), L, lightColor);
                    finalColor += hit.getMaterial()->Shade(ray, hit, L, lightColor);
                }
                image.SetPixel(x, y, finalColor);
            } else {
                image.SetPixel(x, y, parser.getBackgroundColor());
            }
        }
    }
    
    image.SaveBMP(outputFile.c_str());
    return 0;
}

