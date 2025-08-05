#include "bsdf.h"


namespace CGL {


    class BubbleBSDF : public BSDF {
    public:

        BubbleBSDF();
        ~BubbleBSDF();

        Vector3D f(const Vector3D wo, const Vector3D wi);

        Vector3D sample_f(const Vector3D wo, Vector3D *wi, double *pdf);

        Vector3D get_emission() const;

        bool is_delta() const;

        double refractive_index; // Refractive index of the bubble
        double film_thickness; // Thickness of the bubble film in meters
    };
}