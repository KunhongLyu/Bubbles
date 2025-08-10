
#include "light.h"


namespace CGL {
    DirectionalLight::DirectionalLight(const Vector3D rad,
        const Vector3D lightDir)
        : radiance(rad) {
        dirToLight = -lightDir.unit();
    }

    Vector3D DirectionalLight::sample_L(const Vector3D p, Vector3D *wi,
        double *distToLight, double *pdf) const {
        *wi = dirToLight;
        *distToLight = INF_D;
        *pdf = 1.0;
        return radiance;
    }

    // Infinite Hemisphere Light //

    InfiniteHemisphereLight::InfiniteHemisphereLight(const Vector3D rad)
        : radiance(rad) {
        sampleToWorld[0] = Vector3D(1, 0, 0);
        sampleToWorld[1] = Vector3D(0, 0, -1);
        sampleToWorld[2] = Vector3D(0, 1, 0);
    }

    Vector3D InfiniteHemisphereLight::sample_L(const Vector3D p, Vector3D *wi,
        double *distToLight,
        double *pdf) const {
        Vector3D dir;// = sampler.get_sample();
        *wi = sampleToWorld * dir;
        *distToLight = INF_D;
        *pdf = 1.0 / (2.0 * PI);
        return radiance;
    }

    // Point Light //

    PointLight::PointLight(const Vector3D rad, const Vector3D pos) :
        radiance(rad), position(pos) {}

    Vector3D PointLight::sample_L(const Vector3D p, Vector3D *wi,
        double *distToLight,
        double *pdf) const {
        Vector3D d = position - p;
        *wi = d.unit();
        *distToLight = d.norm();
        *pdf = 1.0;
        return radiance;
    }


    // Spot Light //

    SpotLight::SpotLight(const Vector3D rad, const Vector3D pos,
        const Vector3D dir, double angle) {

    }

    Vector3D SpotLight::sample_L(const Vector3D p, Vector3D *wi,
        double *distToLight, double *pdf) const {
        return Vector3D();
    }


    // Area Light //

    AreaLight::AreaLight(const Vector3D rad,
        const Vector3D pos, const Vector3D dir,
        const Vector3D dim_x, const Vector3D dim_y)
        : radiance(rad), position(pos), direction(dir.unit()),
        dim_x(dim_x), dim_y(dim_y), area(dim_x.norm() *dim_y.norm()) {}

    Vector3D AreaLight::sample_L(const Vector3D p, Vector3D *wi,
        double *distToLight, double *pdf) const {

        Vector2D sample = /* sampler.get_sample() */ Vector2D(0.5, 0.5) - Vector2D(0.5, 0.5);
        Vector3D x_l = position + sample.x * dim_x + sample.y * dim_y;
        Vector3D d = x_l - p;
        double sqDist = d.norm2();
        double dist = sqrt(sqDist);
        Vector3D d_unit = d / dist;
        double cosTheta = dot(direction, d_unit);  // direction check
        if (cosTheta <= 0.0) { *pdf = 0.0; return Vector3D(); }
        *pdf = sqDist / (area * cosTheta);
        return radiance;
    };


}