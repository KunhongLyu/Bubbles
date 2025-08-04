
#include "bubble_bsdf.h"


namespace CGL {

    BubbleBSDF::BubbleBSDF() {

    }
    BubbleBSDF::~BubbleBSDF() {

    }

    Vector3D BubbleBSDF::f(const Vector3D wo, const Vector3D wi) {
        return Vector3D(0, 0, 0);
    }

    Vector3D BubbleBSDF::sample_f(const Vector3D wo, Vector3D *wi, double *pdf) {
        return Vector3D(0, 0, 0);
    }

    Vector3D BubbleBSDF::get_emission() const {
        return Vector3D(0, 0, 0);
    }

    bool BubbleBSDF::is_delta() const {
        return false;
    }

}