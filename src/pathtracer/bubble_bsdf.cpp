
#include "bubble_bsdf.h"
#include "random_util.h"
#include <cmath>
#include <complex>
#include <algorithm>
#include "corecrt_math_defines.h"

namespace CGL {

    BubbleBSDF::BubbleBSDF() {
        refractive_index = 1.33f;
        film_thickness = 500e-9f;

    }
    BubbleBSDF::~BubbleBSDF() {

    }

    Vector3D BubbleBSDF::f(const Vector3D wo, const Vector3D wi) {
        // TODO (part 1) (done
        // probably just use sample_f instead of this function
		//dont have one for delta distributions, use the sample_f function
        return Vector3D();
    }
    static void thin_film_RT(double cos_i, double n0, double nf, double n3,
        double d, double lambda, double &R, double &T) {
        cos_i = std::fabs(cos_i);

        // Snell into film
        double sin2_i = std::max(0.0, 1.0 - cos_i * cos_i);
        double sin2_t = (n0 * n0) / (nf * nf) * sin2_i;
        if (sin2_t >= 1.0) {
            // TIR at first interface (unlikely for n0<nf, but safe):
            R = 1.0; T = 0.0; return;
        }
        double cos_t = std::sqrt(std::max(0.0, 1.0 - sin2_t));

        // For air–film–air, theta_3 = theta_1, but keep symbols general:
        double cos_3 = cos_i;

        using cd = std::complex<double>;
        // Fresnel amplitudes at 0|f (incident n0 -> film nf)
        double r01_s = (n0 * cos_i - nf * cos_t) / (n0 * cos_i + nf * cos_t);
        double r01_p = (nf * cos_i - n0 * cos_t) / (nf * cos_i + n0 * cos_t);
        double t01_s = (2.0 * n0 * cos_i) / (n0 * cos_i + nf * cos_t);
        double t01_p = (2.0 * n0 * cos_i) / (nf * cos_i + n0 * cos_t);

        // Fresnel amplitudes at f|3 (film nf -> outer n3)
        // Incident angle inside film is cos_t, transmitted is cos_3
        double r12_s = (nf * cos_t - n3 * cos_3) / (nf * cos_t + n3 * cos_3);
        double r12_p = (n3 * cos_t - nf * cos_3) / (n3 * cos_t + nf * cos_3);
        double t12_s = (2.0 * nf * cos_t) / (nf * cos_t + n3 * cos_3);
        double t12_p = (2.0 * nf * cos_t) / (n3 * cos_t + nf * cos_3);

        // Phase inside the film
        double delta = 2.0 * M_PI * nf * d * cos_t / lambda;
        cd e_i2d = std::polar(1.0, 2.0 * delta); // e^{i 2 delta}

        // Airy closed-form

        cd denom_s = (cd)1.0 + r01_s * r12_s * e_i2d;
        cd denom_p = (cd)1.0 + r01_p * r12_p * e_i2d;

        cd denom_s_inv = 1.0 / denom_s;
        cd denom_p_inv = 1.0 / denom_p;

        cd r_s = (r01_s + r12_s * e_i2d) * denom_s_inv;
        cd r_p = (r01_p + r12_p * e_i2d) * denom_p_inv;

        // Important: transmission amplitude includes one pass through the film
        // (phase delta) and both interfaces' t's:
        cd e_id = std::polar(1.0, delta);
        cd t_s = (t01_s * t12_s * e_id) * denom_s_inv;
        cd t_p = (t01_p * t12_p * e_id) * denom_p_inv;

        double Rs = std::norm(r_s);
        double Rp = std::norm(r_p);
        double Ts = (n3 * cos_3) / (n0 * cos_i) * std::norm(t_s);
        double Tp = (n3 * cos_3) / (n0 * cos_i) * std::norm(t_p);

        R = 0.5 * (Rs + Rp);
        T = 0.5 * (Ts + Tp);

        // Clamp for safety
        R = std::min(1.0, std::max(0.0, R));
        T = std::min(1.0, std::max(0.0, T));
    }

    Vector3D BubbleBSDF::sample_f(const Vector3D wo, Vector3D *wi, double *pdf) {
        // TODO (part 1)
        // This should incorporate the bubble surface bsdf.
        // I think maybe choose 50% 50% to reflect or refract.
        // Maybe the probability isn't 50% 50%, we'll have to look.
        double cos_i = wo.z;
        bool entering = cos_i > 0.0;
        double n_air = 1.0, n_film = refractive_index;
        double d = film_thickness;

        const double lam[3] = { 700e-9, 546.1e-9, 435.8e-9 };
        Vector3D R_rgb, T_rgb;
        for (int c = 0; c < 3; ++c) {
            double Rc, Tc;
            thin_film_RT(cos_i, /*n0=*/1.0, /*nf=*/n_film, /*n3=*/1.0, d, lam[c], Rc, Tc);
            R_rgb[c] = Rc;
            T_rgb[c] = Tc;
        }

        auto lum = [](const Vector3D& c) { return 0.2126 * c.x + 0.7152 * c.y + 0.0722 * c.z; };
        double wR = lum(R_rgb), wT = lum(T_rgb);
        double pR = wR / std::max(1e-12, (wR + wT));
        pR = std::clamp(pR, 1e-4, 1.0 - 1e-4);

        if (coin_flip(pR)) {
            *wi = Vector3D(-wo.x, -wo.y, wo.z);
            *pdf = pR;

            double cos_wi = std::max(1e-8, std::fabs((*wi).z));
            return R_rgb / cos_wi;
        }
        else {
            double eta_i = entering ? n_air : n_film;
            double eta_t = entering ? n_film : n_air;
            double eta = eta_i / eta_t;

            double sin2_i = std::max(0.0, 1.0 - cos_i * cos_i);
            double sin2_t = eta * eta * sin2_i;
            if (sin2_t > 1.0) {
                *wi = Vector3D(-wo.x, -wo.y, wo.z);
                *pdf = 1.0;
                return R_rgb;
            }
            double cos_t = std::sqrt(std::max(0.0, 1.0 - sin2_t));
            if (entering) cos_t = -cos_t;

            *wi = -wo;// Vector3D(wo.x, wo.y, -wo.z);
            *pdf = 1.0 - pR;
            double cos_wi = std::max(1e-8, std::fabs((*wi).z));
            return T_rgb / cos_wi;
        }
    }

    Vector3D BubbleBSDF::get_emission() const {
        return Vector3D(0, 0, 0);
    }

    bool BubbleBSDF::is_delta() const {
        return true;
    }

}