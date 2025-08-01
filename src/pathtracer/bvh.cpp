#include "bvh.h"

#include "CGL/CGL.h"
#include "triangle.h"

#include <iostream>
#include <stack>
#include <algorithm>

using namespace std;
namespace CGL {
    namespace SceneObjects {

        BVHAccel::BVHAccel(const std::vector<Primitive *> &_primitives,
            size_t max_leaf_size) {

            primitives = std::vector<Primitive *>(_primitives);
            root = construct_bvh(primitives.begin(), primitives.end(), max_leaf_size);
        }

        BVHAccel::~BVHAccel() {
            if (root)
                delete root;
            primitives.clear();
        }

        BBox BVHAccel::get_bbox() const { return root->bb; }

        void BVHAccel::draw(BVHNode *node, const Color &c, float alpha) const {
            if (node->isLeaf()) {
                for (auto p = node->start; p != node->end; p++) {
                    (*p)->draw(c, alpha);
                }
            } else {
                draw(node->l, c, alpha);
                draw(node->r, c, alpha);
            }
        }

        void BVHAccel::drawOutline(BVHNode *node, const Color &c, float alpha) const {
            if (node->isLeaf()) {
                for (auto p = node->start; p != node->end; p++) {
                    (*p)->drawOutline(c, alpha);
                }
            } else {
                drawOutline(node->l, c, alpha);
                drawOutline(node->r, c, alpha);
            }
        }

        BVHNode *BVHAccel::construct_bvh(std::vector<Primitive *>::iterator start,
            std::vector<Primitive *>::iterator end,
            size_t max_leaf_size) {

            // TODO (Part 2.1):
            // Construct a BVH from the given vector of primitives and maximum leaf
            // size configuration. The starter code build a BVH aggregate with a
            // single leaf node (which is also the root) that encloses all the
            // primitives.

            size_t num_primitives = std::distance(start, end);

            BBox bbox;

            for (auto p = start; p != end; p++) {
                BBox bb = (*p)->get_bbox();
                bbox.expand(bb);
            }

            BVHNode *node = new BVHNode(bbox);
            node->start = start;
            node->end = end;

            if (num_primitives <= max_leaf_size) {
                node->l = NULL;
                node->r = NULL;
                return node;
            }

            Vector3D centroid = bbox.centroid();
            Vector3D extent = bbox.extent;
            int axis = 0;
            if (extent.y > extent.x) axis = 1;
            if (extent.z > extent[axis]) axis = 2;

            double split = centroid[axis];
            auto is_left = [axis, split](Primitive *p) {
                const BBox &b = p->get_bbox();
                return b.centroid()[axis] < split;
                };


            std::vector<Primitive *>::iterator mid;
            mid = std::stable_partition(start, end, is_left);

            if (std::distance(start, mid) == 0 || std::distance(mid, end) == 0) {
                mid = start + num_primitives / 2;
            }

            node->l = construct_bvh(start, mid, max_leaf_size);
            node->r = construct_bvh(mid, end, max_leaf_size);

            return node;
        }

        bool BVHAccel::has_intersection(const Ray &ray, BVHNode *node) const {
            // TODO (Part 2.3):
            // Fill in the intersect function.
            // Take note that this function has a short-circuit that the
            // Intersection version cannot, since it returns as soon as it finds
            // a hit, it doesn't actually have to find the closest hit.

            if (node == NULL)
                return false;

            double t1, t2;
            if (!node->bb.intersect(ray, t1, t2))
                return false;
            if (t1 > ray.max_t || t2 < ray.min_t)
                return false;

            if (node->isLeaf()) {
                for (auto p = node->start; p != node->end; p++) {
                    total_isects++;
                    if ((*p)->has_intersection(ray))
                        return true;
                }
            }

            if (has_intersection(ray, node->l))
                return true;
            if (has_intersection(ray, node->r))
                return true;

            return false;
        }

        bool BVHAccel::intersect(const Ray &ray, Intersection *i, BVHNode *node) const {
            // TODO (Part 2.3):
            // Fill in the intersect function.
            if (node == NULL)
                return false;

            double t1, t2;
            if (!node->bb.intersect(ray, t1, t2))
                return false;
            if (t1 > ray.max_t || t2 < ray.min_t)
                return false;

            if (node->isLeaf()) {
                Intersection temp;
                bool hit = false;
                for (auto p = node->start; p != node->end; p++) {
                    total_isects++;
                    if ((*p)->intersect(ray, &temp) && temp.t < i->t) {
                        *i = temp;
                        hit = true;
                    }
                }
                return hit;
            }

            bool hit = false;
            Intersection temp;
            if (intersect(ray, &temp, node->l)) {
                *i = temp;
                hit = true;
            }
            if (intersect(ray, &temp, node->r) && temp.t < i->t) {
                *i = temp;
                hit = true;
            }
            return hit;
        }

    } // namespace SceneObjects
} // namespace CGL
