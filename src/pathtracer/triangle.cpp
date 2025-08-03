
#include "triangle.h"

#include "CGL/CGL.h"
#include "GL/glew.h"

namespace CGL {

    Triangle::Triangle(Vector3D p1, Vector3D p2, Vector3D p3, Vector3D n1, Vector3D n2, Vector3D n3, BSDF *bsdf) {
        this->p1 = p1;
        this->p2 = p2;
        this->p3 = p3;
        this->n1 = n1;
        this->n2 = n2;
        this->n3 = n3;
        bbox = BBox(p1);
        bbox.expand(p2);
        bbox.expand(p3);

        this->bsdf = bsdf;
    }

    BBox Triangle::get_bbox() const { return bbox; }

    bool Triangle::has_intersection(const Ray &r) const {
        // Part 1, Task 3: implement ray-triangle intersection
        // The difference between this function and the next function is that the next
        // function records the "intersection" while this function only tests whether
        // there is a intersection.
        constexpr double eps = std::numeric_limits<double>::epsilon();

        Vector3D e1 = p2 - p1;
        Vector3D e2 = p3 - p1;
        Vector3D pvec = cross(r.d, e2);
        double det = dot(e1, pvec);

        if (det < eps && det > -eps) return false;

        double inv_det = 1.0 / det;
        Vector3D tvec = r.o - p1;
        double u = dot(tvec, pvec) * inv_det;

        if (u < 0.0 || u > 1.0) return false;

        Vector3D qvec = cross(tvec, e1);
        double v = dot(r.d, qvec) * inv_det;

        if (v < 0.0 || u + v > 1.0) return false;

        double t = dot(e2, qvec) * inv_det;

        if (t < r.min_t || t > r.max_t) return false;

        return true;
    }

    bool Triangle::intersect(const Ray &r, Intersection *isect) const {
        // Part 1, Task 3:
        // implement ray-triangle intersection. When an intersection takes
        // place, the Intersection data should be updated accordingly

        constexpr double eps = std::numeric_limits<double>::epsilon();

        Vector3D e1 = p2 - p1;
        Vector3D e2 = p3 - p1;
        Vector3D s1 = cross(r.d, e2);
        double det = dot(s1, e1);
        // if the ray is parallel to the plane
        if (det < eps && det > -eps) return false;

        double inv_det = 1.0 / det;

        Vector3D s = r.o - p1;
        double u = inv_det * dot(s1, s);
        if (u < 0.0 || u > 1.0) return false;

        Vector3D s2 = cross(s, e1);
        double v = inv_det * dot(s2, r.d);
        if (v < 0.0 || u + v > 1.0) return false;

        double t = inv_det * dot(s2, e2);
        if (t < r.min_t || t > r.max_t) return false;

        r.max_t = t;
        isect->t = t;
        isect->primitive = this;
        isect->bsdf = get_bsdf();
        isect->n = (1.0 - u - v) * n1 + u * n2 + v * n3;
        isect->n.normalize();

        return true;
    }
} // namespace CGL
