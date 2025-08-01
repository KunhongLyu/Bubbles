#include "bsdf.h"

#include <algorithm>
#include <iostream>
#include <utility>

using std::max;
using std::min;
using std::swap;

namespace CGL {

// Mirror BSDF //

Vector3D MirrorBSDF::f(const Vector3D wo, const Vector3D wi) {
  return Vector3D();
}

Vector3D MirrorBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {

  // TODO:
  // Implement MirrorBSDF
    reflect(wo, wi);
    *pdf = 1.0;
    return reflectance / abs_cos_theta(*wi);
}

// Microfacet BSDF //

double MicrofacetBSDF::G(const Vector3D wo, const Vector3D wi) {
  return 1.0 / (1.0 + Lambda(wi) + Lambda(wo));
}

double MicrofacetBSDF::D(const Vector3D h) {
  // TODO: proj3-2, part 3
  // Compute Beckmann normal distribution function (NDF) here.
  // You will need the roughness alpha.
    double theta_h = getTheta(h);
    double tan_theta_h = tan(theta_h);
    double cos_theta_h = cos(theta_h);
    double denom = PI * alpha * alpha * cos_theta_h * cos_theta_h * cos_theta_h * cos_theta_h;
    double expon = exp(-tan_theta_h * tan_theta_h / (alpha * alpha));
    return expon / denom;
}

Vector3D MicrofacetBSDF::F(const Vector3D wi) {
  // TODO: proj3-2, part 3
  // Compute Fresnel term for reflection on dielectric-conductor interface.
  // You will need both eta and etaK, both of which are Vector3D.
    double cosTheta = abs_cos_theta(wi);
    Vector3D Rs = (eta * eta + k * k - Vector3D(2.0) * eta * cosTheta + Vector3D(cosTheta * cosTheta)) /
        (eta * eta + k * k + Vector3D(2.0) * eta * cosTheta + Vector3D(cosTheta * cosTheta));
    Vector3D Rp = (eta * eta + k * k - Vector3D(2.0) * eta / cosTheta + Vector3D(1.0)) /
        (eta * eta + k * k + Vector3D(2.0) * eta / cosTheta + Vector3D(1.0));
    return (Rs + Rp) / 2.0;
}

Vector3D MicrofacetBSDF::f(const Vector3D wo, const Vector3D wi) {
  // TODO: proj3-2, part 3
  // Implement microfacet model here.
    if (cos_theta(wo) <= 0 || cos_theta(wi) <= 0) return Vector3D();
    Vector3D h = (wo + wi).unit();
    return F(wi) * G(wo, wi) * D(h) / (4.0 * cos_theta(wi) * cos_theta(wo));
}

Vector3D MicrofacetBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {
  // TODO: proj3-2, part 3
  // *Importance* sample Beckmann normal distribution function (NDF) here.
  // Note: You should fill in the sampled direction *wi and the corresponding *pdf,
  //       and return the sampled BRDF value.

    Vector2D sample = sampler.get_sample();
    double theta = atan(sqrt(-alpha * alpha * log(1 - sample.x)));
    double phi = 2 * PI * sample.y;
    Vector3D h = Vector3D(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
    *wi = (Vector3D(2.0) * dot(wo, h) * h - wo).unit();
    if ((*wi).z <= 0) {
        *pdf = 0.0;
        return Vector3D();
    }
    double d = D(h);
    *pdf = d * cos_theta(h) / (4.0 * dot(wo, h));
    return MicrofacetBSDF::f(wo, *wi);
}

// Refraction BSDF //

Vector3D RefractionBSDF::f(const Vector3D wo, const Vector3D wi) {
  return Vector3D();
}

Vector3D RefractionBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {

  // TODO:
  // Implement RefractionBSDF
  
    if (!refract(wo, wi, ior)) return Vector3D();
    *pdf = 1.0;
    double eta = cos_theta(wo) > 0 ? ior : 1.0 / ior;
    return transmittance / abs_cos_theta(*wi) / (eta * eta);
}

// Glass BSDF //

Vector3D GlassBSDF::f(const Vector3D wo, const Vector3D wi) {
  return Vector3D();
}

Vector3D GlassBSDF::sample_f(const Vector3D wo, Vector3D* wi, double* pdf) {

  // TODO:
  // Compute Fresnel coefficient and either reflect or refract based on it.

  // compute Fresnel coefficient and use it as the probability of reflection
  // - Fundamentals of Computer Graphics page 305

    if (!refract(wo, wi, ior)) {
        reflect(wo, wi);
        *pdf = 1.0;
        return reflectance / abs_cos_theta(*wi);
    }

    bool entering = cos_theta(wo) > 0;
    double eta_i = entering ? 1.0 : ior;
    double eta_t = entering ? ior : 1.0;
    double cos_theta_i = abs_cos_theta(wo);
    double cos_theta_t = abs_cos_theta(*wi);
    Vector3D eta = eta_i / eta_t;

    double r_parallel = (eta_t * cos_theta_i - eta_i * cos_theta_t) / (eta_t * cos_theta_i + eta_i * cos_theta_t);
    double r_perp = (eta_i * cos_theta_i - eta_t * cos_theta_t) / (eta_i * cos_theta_i + eta_t * cos_theta_t);
    double fr = 0.5 * (r_parallel * r_parallel + r_perp * r_perp);

    if (coin_flip(fr)) {
        reflect(wo, wi);
        *pdf = fr;
        return fr * reflectance / abs_cos_theta(*wi);
    } else {
        *pdf = 1.0 - fr;
        return (1.0 - fr) * transmittance * eta * eta / abs_cos_theta(*wi);
    }
}

void BSDF::reflect(const Vector3D wo, Vector3D* wi) {

  // TODO:
  // Implement reflection of wo about normal (0,0,1) and store result in wi.
    *wi = Vector3D(-wo.x, -wo.y, wo.z);
}

bool BSDF::refract(const Vector3D wo, Vector3D* wi, double ior) {

  // TODO:
  // Use Snell's Law to refract wo surface and store result ray in wi.
  // Return false if refraction does not occur due to total internal reflection
  // and true otherwise. When dot(wo,n) is positive, then wo corresponds to a
  // ray entering the surface through vacuum.


    bool entering = cos_theta(wo) > 0;
    double eta_i = entering ? 1.0 : ior;
    double eta_t = entering ? ior : 1.0;
    double eta = eta_i / eta_t;

    double cos_theta_i = abs_cos_theta(wo);
    double sin_theta_i = sin_theta(wo);
    double sin_theta_t = eta * sin_theta_i;
    if (sin_theta_t >= 1.0) return false;
    double cos_theta_t = sqrt(1.0 - sin_theta_t * sin_theta_t);

    *wi = eta * -wo + (eta * cos_theta_i - cos_theta_t) * Vector3D(0, 0, entering ? 1.0 : -1.0);
    return true;
}

} // namespace CGL
