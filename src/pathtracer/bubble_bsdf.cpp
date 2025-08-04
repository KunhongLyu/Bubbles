
#include "bubble_bsdf.h"
#include "random_util.h"


namespace CGL {

    BubbleBSDF::BubbleBSDF() {

    }
    BubbleBSDF::~BubbleBSDF() {

    }

    Vector3D BubbleBSDF::f(const Vector3D wo, const Vector3D wi) {
        // TODO (part 1)
        // probably just use sample_f instead of this function
        return Vector3D(0, 0, 0);
    }

    Vector3D BubbleBSDF::sample_f(const Vector3D wo, Vector3D *wi, double *pdf) {
        // TODO (part 1)
        // This should incorporate the bubble surface bsdf.
        // I think maybe choose 50% 50% to reflect or refract.
        // Maybe the probability isn¡¯t 50% 50%, we¡¯ll have to look.


        if (coin_flip(0.5)) {
            // reflect here

            return Vector3D(0, 0, 0);
        } else {
            // refract here

            return Vector3D(0, 0, 0);
        }
    }

    Vector3D BubbleBSDF::get_emission() const {
        return Vector3D(0, 0, 0);
    }

    bool BubbleBSDF::is_delta() const {
        return false;
    }

}