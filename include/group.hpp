#ifndef GROUP_H
#define GROUP_H


#include "object3d.hpp"
#include "ray.hpp"
#include "hit.hpp"
#include <cassert>
#include <vector>


// TODO: Implement Group - add data structure to store a list of Object*
class Group : public Object3D {

public:

    Group() {}

    explicit Group (int num_objects) {
        this->num_objects = num_objects; 
        objects = std::vector<Object3D*>(num_objects); 
    }

    ~Group() override {
        for (Object3D* obj : objects) {
            delete obj;
        }
    }

    bool intersect(const Ray &r, Hit &h, float tmin) override {
        // naive algorithm
        bool isHit = false;
        for(int i = 0; i < objects.size(); ++i) {
            isHit |= objects[i]->intersect(r, h, tmin);
        }
        return isHit;
    }

    void addObject(int index, Object3D *obj) {
        assert(index >= -1 && index < num_objects);
        if (index == -1) {
            objects.push_back(obj); 
            num_objects++;
        } else {
            objects[index] = obj;
        }
    }

    int getGroupSize() {
        return num_objects;
    }

private:
    int num_objects;
    std::vector<Object3D*> objects;
};

#endif // GROUP_H
