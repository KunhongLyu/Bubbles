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


    class DirectionalLight : public SceneLight {
    public:
        DirectionalLight(const Vector3D rad, const Vector3D lightDir);
        Vector3D sample_L(const Vector3D p, Vector3D *wi, double *distToLight,
            double *pdf) const;
        bool is_delta_light() const { return true; }

    private:
        Vector3D radiance;
        Vector3D dirToLight;

    }; // class Directional Light

    // Infinite Hemisphere Light //

    class InfiniteHemisphereLight : public SceneLight {
    public:
        InfiniteHemisphereLight(const Vector3D rad);
        Vector3D sample_L(const Vector3D p, Vector3D *wi, double *distToLight,
            double *pdf) const;
        bool is_delta_light() const { return false; }

        Vector3D radiance;
        Matrix3x3 sampleToWorld;
        //UniformHemisphereSampler3D sampler;

    }; // class InfiniteHemisphereLight


    // Point Light //

    class PointLight : public SceneLight {
    public:
        PointLight(const Vector3D rad, const Vector3D pos);
        Vector3D sample_L(const Vector3D p, Vector3D *wi, double *distToLight,
            double *pdf) const;
        bool is_delta_light() const { return true; }

        Vector3D radiance;
        Vector3D position;

    }; // class PointLight

    // Spot Light //

    class SpotLight : public SceneLight {
    public:
        SpotLight(const Vector3D rad, const Vector3D pos,
            const Vector3D dir, double angle);
        Vector3D sample_L(const Vector3D p, Vector3D *wi, double *distToLight,
            double *pdf) const;
        bool is_delta_light() const { return true; }

        Vector3D radiance;
        Vector3D position;
        Vector3D direction;
        double angle;

    }; // class SpotLight

    // Area Light //

    class AreaLight : public SceneLight {
    public:
        AreaLight(const Vector3D rad,
            const Vector3D pos, const Vector3D dir,
            const Vector3D dim_x, const Vector3D dim_y);
        Vector3D sample_L(const Vector3D p, Vector3D *wi, double *distToLight,
            double *pdf) const;
        bool is_delta_light() const { return false; }

        Vector3D radiance;
        Vector3D position;
        Vector3D direction;
        Vector3D dim_x;
        Vector3D dim_y;
        //UniformGridSampler2D sampler;
        double area;

    }; // class AreaLight


}  // namespace CGL

#endif  // __CGL_PATHTRACER_H__
