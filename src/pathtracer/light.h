#ifndef __CGL_LIGHT_H__
#define __CGL_LIGHT_H__

#include "CGL/CGLMath.h"

namespace CGL {

    class SceneLight {
    public:
        virtual Vector3D sample_L(const Vector3D p, Vector3D *wi,
            double *distToLight, double *pdf) const = 0;
        virtual bool is_delta_light() const = 0;
    };

}  // namespace CGL

#endif  // __CGL_PATHTRACER_H__
