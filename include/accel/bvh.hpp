#ifndef BVH_H
#define BVH_H

#include <algorithm>
#include <iostream>
#include <vector>
#include <chrono>

#include "aabb.hpp"
#include "objects/object3d.hpp"
#include "ray.hpp"
#include "hit.hpp"

class BVHNode {
public:
    AABB *aabb;
    const Object3D *object;      // only leaf nodes have object
    BVHNode *left, *right;       // leaf nodes: left = right = nullptr

    BVHNode() : aabb(nullptr), object(nullptr), left(nullptr), right(nullptr) {}
    BVHNode(AABB *_aabb, BVHNode *_left, BVHNode *_right) : aabb(_aabb), object(nullptr), left(_left), right(_right) {}
    BVHNode(const Object3D *_object) : aabb(_object->getAABB()), object(_object), left(nullptr), right(nullptr) {}

    ~BVHNode() { delete aabb; delete left; delete right; }

    bool intersect(const Ray &r, Hit &h, float tmin) const {
        if (!aabb->intersectAABB(r, tmin, h.getT())) return false;

        // leaf node
        if (object) return object->intersect(r, h, tmin);

        // internal node
        bool result;
        if (left) result = left->intersect(r, h, tmin);
        if (right) result |= right->intersect(r, h, tmin);
        return result;
    }

};

class BVH {
public:

    BVH() = delete;
    BVH(const std::vector<const Object3D*> &objects) {
        std::cout << "Building BVH with " << objects.size() << " objects..." << std::endl;
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<const Object3D*> nonPlanes;
        for (auto obj : objects) {
            if (obj->isPlane()) {
                planes.push_back(obj);
            } else {
                nonPlanes.push_back(obj);
            }
        }
        root = buildBVH(nonPlanes);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "BVH built in " << duration << "ms" << std::endl;
    }

    ~BVH() { delete root; }

    bool intersect(const Ray &r, Hit &h, float tmin) const {
        bool result = root->intersect(r, h, tmin);
        for (auto plane : planes) {
            result |= plane->intersect(r, h, tmin);
        }
        return result;
    }

private:
    BVHNode *root;
    std::vector<const Object3D*> planes; // handle Planes separately
    static constexpr int B = 16;         // num of buckets of SAH

    struct BucketInfo {
        int count = 0;
        AABB aabb;
        BucketInfo() : count(0) {}
    };

    BVHNode *buildBVH(const std::vector<const Object3D*> &objects) {
        if (objects.empty()) return nullptr;
        if (objects.size() == 1) return new BVHNode(objects[0]);
        if (objects.size() == 2) {
            AABB *combined = union_(*objects[0]->getAABB(), *objects[1]->getAABB());
            return new BVHNode(combined, new BVHNode(objects[0]), new BVHNode(objects[1]));
        }

        // simply use median split when num of objects is small
        if (objects.size() <= 4) {

            AABB centroidAABB;
            for (auto obj : objects)
                centroidAABB.union_(obj->getAABB()->centroid());

            // choose axis with maximum extent
            Vector3f diag = centroidAABB.pMax - centroidAABB.pMin;
            int axis = 0;
            if (diag.y() > diag.x()) axis = 1;
            if (diag.z() > diag[axis]) axis = 2;

            auto sorted = objects;
            std::sort(sorted.begin(), sorted.end(), [axis](const Object3D* a, const Object3D* b) {
                return a->getAABB()->centroid()[axis] < b->getAABB()->centroid()[axis];
            });

            auto mid = sorted.begin() + sorted.size() / 2;
            std::vector<const Object3D*> left(sorted.begin(), mid);
            std::vector<const Object3D*> right(mid, sorted.end());

            BVHNode *leftChild = buildBVH(left);
            BVHNode *rightChild = buildBVH(right);

            AABB *combined = union_(*leftChild->aabb, *rightChild->aabb);
            return new BVHNode(combined, leftChild, rightChild);
        }

        // SAH implementation, reference:
        // https://pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies
        // http://15462.courses.cs.cmu.edu/fall2018/lecture/spatialdatastructures

        float minCost = std::numeric_limits<float>::max();
        int bestAxis = -1;
        float bestPos = 0;

        AABB centroidAABB;
        for (auto obj : objects)
            centroidAABB.union_(obj->getAABB()->centroid());

        // try all three axes
        for (int axis = 0; axis < 3; ++axis) {
            Vector3f extent = centroidAABB.pMax - centroidAABB.pMin;
            if (extent[axis] <= 0) continue; // skip degenerate axis

            float minVal = centroidAABB.pMin[axis];
            float maxVal = centroidAABB.pMax[axis];
            float bucketWidth = (maxVal - minVal) / B;

            // initialize buckets (B buckets in total, each bucket contains count and AABB)
            BucketInfo buckets[B];
            for (auto obj : objects) {
                float centroidPos = obj->getAABB()->centroid()[axis];
                int b = std::min(B-1, (int)((centroidPos - minVal) / bucketWidth));
                buckets[b].count++;
                buckets[b].aabb.union_(*obj->getAABB());
            }

            // compute prefix AABBs and counts
            AABB prefixAABBs[B];
            int prefixCounts[B];
            prefixAABBs[0] = buckets[0].aabb;
            prefixCounts[0] = buckets[0].count;
            for (int i = 1; i < B; ++i) {
                prefixAABBs[i] = prefixAABBs[i-1];
                prefixAABBs[i].union_(buckets[i].aabb);
                prefixCounts[i] = prefixCounts[i-1] + buckets[i].count;
            }

            // compute suffix AABBs and counts
            AABB suffixAABBs[B];
            int suffixCounts[B];
            suffixAABBs[B-1] = buckets[B-1].aabb;
            suffixCounts[B-1] = buckets[B-1].count;
            for (int i = B-2; i >= 0; --i) {
                suffixAABBs[i] = suffixAABBs[i+1];
                suffixAABBs[i].union_(buckets[i].aabb);
                suffixCounts[i] = suffixCounts[i+1] + buckets[i].count;
            }

            // evaluate SAH for possible splits
            for (int k = 1; k < B; ++k) {
                int leftCount = prefixCounts[k-1];
                int rightCount = suffixCounts[k];
                if (leftCount == 0 || rightCount == 0) continue;

                float cost = prefixAABBs[k-1].surfaceArea() * leftCount 
                           + suffixAABBs[k].surfaceArea() * rightCount;
                
                if (cost < minCost) {
                    minCost = cost;
                    bestAxis = axis;
                    bestPos = minVal + k * bucketWidth;
                }
            }
        }

        // fallback to median split if SAH failed, can only happen when all objects' centroids are the same
        if (bestAxis == -1) {
            std::vector<const Object3D*> left, right;
            int mid = objects.size() / 2;
            left.assign(objects.begin(), objects.begin() + mid);
            right.assign(objects.begin() + mid, objects.end());
            BVHNode *leftChild = buildBVH(left);
            BVHNode *rightChild = buildBVH(right);
            AABB *combined = union_(*leftChild->aabb, *rightChild->aabb);
            return new BVHNode(combined, leftChild, rightChild);
        }

        // partition objects based on best split
        std::vector<const Object3D*> leftObjs, rightObjs;
        for (auto obj : objects) {
            float val = obj->getAABB()->centroid()[bestAxis];
            if (val < bestPos)
                leftObjs.push_back(obj);
            else
                rightObjs.push_back(obj);
        }

        // just in case, normally this should not happen
        if (leftObjs.empty() || rightObjs.empty()) {
            int mid = objects.size() / 2;
            leftObjs.assign(objects.begin(), objects.begin() + mid);
            rightObjs.assign(objects.begin() + mid, objects.end());
        }

        // build subtrees
        BVHNode *leftChild = buildBVH(leftObjs);
        BVHNode *rightChild = buildBVH(rightObjs);
        AABB *combined = union_(*leftChild->aabb, *rightChild->aabb);
        return new BVHNode(combined, leftChild, rightChild);
    }

};

#endif // BVH_H
