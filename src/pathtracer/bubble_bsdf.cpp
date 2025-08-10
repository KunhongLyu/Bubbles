
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

    static double thin_film_reflectance(double cos_i, double n0, double nf, double d, double lambda) {
        cos_i = std::fabs(cos_i);
        double sin2_i = std::max(0.0, 1.0 - cos_i * cos_i);
        double sin2_t = (n0 * n0) / (nf * nf) * sin2_i;
        if (sin2_t >= 1.0) {
            return 1.0;
        }
        double cos_t = std::sqrt(std::max(0.0, 1.0 - sin2_t));

        std::complex<double> r01_s = (n0 * cos_i - nf * cos_t) / (n0 * cos_i + nf * cos_t);
        std::complex<double> r01_p = (nf * cos_i - n0 * cos_t) / (nf * cos_i + n0 * cos_t);

        std::complex<double> r12_s = (nf * cos_t - n0 * cos_i) / (nf * cos_t + n0 * cos_i);
        std::complex<double> r12_p = (n0 * cos_t - nf * cos_i) / (n0 * cos_t + nf * cos_i);

        double delta = 2.0 * M_PI * nf * d * cos_t / lambda;
        std::complex<double> exp2i = std::polar(1.0, 2.0 * delta);

        std::complex<double> num_s = r01_s + r12_s * exp2i;
        std::complex<double> den_s = 1.0 + r01_s * r12_s * exp2i;
        std::complex<double> rtot_s = num_s / den_s;

        std::complex<double> num_p = r01_p + r12_p * exp2i;
        std::complex<double> den_p = 1.0 + r01_p * r12_p * exp2i;
        std::complex<double> rtot_p = num_p / den_p;

        double Rs = std::norm(rtot_s);
        double Rp = std::norm(rtot_p);
        double R = 0.5 * (Rs + Rp);

        return std::min(1.0, std::max(0.0, R));
    }

    Vector3D BubbleBSDF::sample_f(const Vector3D wo, Vector3D *wi, double *pdf) {
        // TODO (part 1)
        // This should incorporate the bubble surface bsdf.
        // I think maybe choose 50% 50% to reflect or refract.
        // Maybe the probability isn¡¯t 50% 50%, we¡¯ll have to look.
            // 1) 基本几何
        double cos_i = wo.z;
        bool entering = cos_i > 0.0;
        double n_air = 1.0, n_film = refractive_index;
        double d = film_thickness;

        // 三个代表波长
        const double lam[3] = { 700e-9, 546.1e-9, 435.8e-9 };
        Vector3D R_rgb;
        for (int c = 0; c < 3; ++c) R_rgb[c] = thin_film_reflectance(cos_i, n_air, n_film, d, lam[c]);
        Vector3D T_rgb(1.0 - R_rgb.x, 1.0 - R_rgb.y, 1.0 - R_rgb.z);

        auto lum = [](const Vector3D& c) { return 0.2126 * c.x + 0.7152 * c.y + 0.0722 * c.z; };
        double pR = std::clamp(lum(R_rgb), 1e-4, 1.0 - 1e-4); // 防止 pdf 过小

        // 反射 or 折射
        if (coin_flip(pR)) {
            *wi = Vector3D(-wo.x, -wo.y, wo.z); // 镜面反射
            *pdf = pR;

            double cos_wi = std::max(1e-8, std::fabs((*wi).z));
            return (R_rgb * pR) / cos_wi; // 纯 f，交给 integrator 乘 abs_cos/pdf
        }
        else {
            double eta_i = entering ? n_air : n_film;
            double eta_t = entering ? n_film : n_air;
            double eta = eta_i / eta_t;

            double sin2_i = std::max(0.0, 1.0 - cos_i * cos_i);
            double sin2_t = eta * eta * sin2_i;
            if (sin2_t > 1.0) { // 全反 → 当作反射
                *wi = Vector3D(-wo.x, -wo.y, wo.z);
                *pdf = 1.0;
                return R_rgb;    // 与上面的反射分支匹配
            }
            double cos_t = std::sqrt(std::max(0.0, 1.0 - sin2_t));
            if (entering) cos_t = -cos_t; // 约定：法线在 +z

            *wi = -wo;
            *pdf = 1.0 - pR;
            double cos_wi = std::max(1e-8, std::fabs((*wi).z));
            return (T_rgb * (1.0 - pR)) / cos_wi;
        }
    }

    Vector3D BubbleBSDF::get_emission() const {
        return Vector3D(0, 0, 0);
    }

    bool BubbleBSDF::is_delta() const {
        return true;
    }

}