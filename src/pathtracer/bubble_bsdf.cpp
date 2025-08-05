
#include "bubble_bsdf.h"
#include "random_util.h"
#include <cmath>

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
        return Vector3D(0, 0, 0);
    }

    Vector3D BubbleBSDF::sample_f(const Vector3D wo, Vector3D *wi, double *pdf) {
        // TODO (part 1)
        // This should incorporate the bubble surface bsdf.
        // I think maybe choose 50% 50% to reflect or refract.
        // Maybe the probability isn¡¯t 50% 50%, we¡¯ll have to look.
        double cos_theta = wo.z;
        bool entering = cos_theta > 0.0;
        double eta_i = 1.0;
        double eta_t = refractive_index;

        if (!entering) std::swap(eta_i, eta_t);

        //  Schlick's approximation for Fresnel
        double r0 = pow((eta_i - eta_t) / (eta_i + eta_t), 2.0);
        double fresnel_R = r0 + (1 - r0) * pow(1 - fabs(cos_theta), 5.0);

        if (coin_flip(fresnel_R)) {
            // Reflect
            *wi = Vector3D(-wo.x, -wo.y, wo.z);
            *pdf = fresnel_R;
            return fresnel_R / fabs((*wi).z) * Vector3D(1, 1, 1); // Scale by fresnel and cosine
        }
        else {
            // Refract
            double eta = eta_i / eta_t;
            double sin2_theta_i = fmax(0.0, 1.0 - cos_theta * cos_theta);
            double sin2_theta_t = eta * eta * sin2_theta_i;

            if (sin2_theta_t > 1.0) {
                *wi = Vector3D(-wo.x, -wo.y, wo.z);
                *pdf = 1.0;
                return Vector3D(1, 1, 1); 
            }

            double cos_theta_t = sqrt(1.0 - sin2_theta_t);
            if (entering) cos_theta_t = -cos_theta_t;

            *wi = Vector3D(-eta * wo.x, -eta * wo.y, cos_theta_t);
            *pdf = 1.0 - fresnel_R;

            double factor = eta * eta;
            return factor / fabs((*wi).z) * Vector3D(1, 1, 1);
        }
    }

    Vector3D BubbleBSDF::get_emission() const {
        return Vector3D(0, 0, 0);
    }

    bool BubbleBSDF::is_delta() const {
        return true;
    }

}