#include "bbox.h"

#include <algorithm>
#include <iostream>

namespace CGL {

    bool BBox::intersect(const Ray &r, double &t0, double &t1) const {

        t0 = r.min_t;
        t1 = r.max_t;

        // Check each axis (x, y, z)
        for (int i = 0; i < 3; ++i) {
            // Compute intersection times with the slab (min[i], max[i])
            double inv_d = 1.0 / r.d[i]; // Inverse direction for efficiency
            double t_near = (min[i] - r.o[i]) * inv_d;
            double t_far = (max[i] - r.o[i]) * inv_d;

            // Swap if ray is negative direction
            if (inv_d < 0.0) {
                std::swap(t_near, t_far);
            }

            // Update t0 (latest entry) and t1 (earliest exit)
            t0 = std::max(t0, t_near);
            t1 = std::min(t1, t_far);

            // If t0 > t1, no intersection possible
            if (t0 > t1) {
                return false;
            }
        }

        // Intersection exists if t0 <= t1 and within ray's bounds
        return t0 <= r.max_t && t1 >= r.min_t;

    }

    std::ostream &operator<<(std::ostream &os, const BBox &b) {
        return os << "BBOX(" << b.min << ", " << b.max << ")";
    }

} // namespace CGL
