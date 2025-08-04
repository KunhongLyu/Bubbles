#ifndef __CGL_PATHTRACER_H__
#define __CGL_PATHTRACER_H__

#include "CGL/timer.h"
#include "CGL/CGLMath.h"

#include "bvh.h"
#include "sampler.h"
#include "intersection.h"
#include "light.h"

#include "../util/image.h"
#include "camera.h"


using CGL::BVHNode;
using CGL::BVHAccel;

namespace CGL {

    class PathTracer {
    public:
        PathTracer();
        ~PathTracer();

        /**
         * Sets the pathtracer's frame size. If in a running state (VISUALIZE,
         * RENDERING, or DONE), transitions to READY b/c a changing window size
         * would invalidate the output. If in INIT and configuration is done,
         * transitions to READY.
         * \param width width of the frame
         * \param height height of the frame
         */
        void set_frame_size(size_t width, size_t height);

        void write_to_framebuffer(ImageBuffer &framebuffer, size_t x0, size_t y0, size_t x1, size_t y1);

        /**
         * If the pathtracer is in READY, delete all internal data, transition to INIT.
         */
        void clear();

        void autofocus(Vector2D loc);

        /**
         * Trace an ray in the scene.
         */
        Vector3D estimate_direct_lighting_hemisphere(const Ray &r, const Intersection &isect);
        Vector3D estimate_direct_lighting_importance(const Ray &r, const Intersection &isect);

        Vector3D est_radiance_global_illumination(const Ray &r);
        Vector3D zero_bounce_radiance(const Ray &r, const Intersection &isect);
        Vector3D one_bounce_radiance(const Ray &r, const Intersection &isect);
        Vector3D at_least_one_bounce_radiance(const Ray &r, const Intersection &isect);
        template<bool use_roulette>
        Vector3D at_least_one_bounce_radiance_internal(const Ray &r, const Intersection &isect);

        Vector3D debug_shading(const Vector3D d) {
            return Vector3D(abs(d.r), abs(d.g), .0).unit();
        }

        Vector3D normal_shading(const Vector3D n) {
            return n * .5 + Vector3D(.5);
        }

        void set_roulette_prob(double new_prob) {
            roulette_prob = new_prob;
            roulette_prob_inv = 1 / new_prob;
        }

        /**
         * Trace a camera ray given by the pixel coordinate.
         */
        void raytrace_pixel(size_t x, size_t y);

        // Integrator sampling settings //

        size_t max_ray_depth; ///< maximum allowed ray depth (applies to all rays)
        size_t isAccumBounces; ///< number of bounces to accumulate
        size_t ns_aa;         ///< number of camera rays in one pixel (along one axis)
        size_t ns_area_light; ///< number samples per area light source
        size_t ns_diff;       ///< number of samples - diffuse surfaces
        size_t ns_glsy;       ///< number of samples - glossy surfaces
        size_t ns_refr;       ///< number of samples - refractive surfaces

        size_t samplesPerBatch;
        double maxTolerance;
        bool direct_hemisphere_sample; ///< true if sampling uniformly from hemisphere for direct lighting. Otherwise, light sample
        bool use_roulette_stopping;

        double roulette_prob;
        double roulette_prob_inv;

        // Components //

        BVHAccel *bvh;                 ///< BVH accelerator aggregate
        Sampler2D *gridSampler;        ///< samples unit grid
        Sampler3D *hemisphereSampler;  ///< samples unit hemisphere
        HDRImageBuffer sampleBuffer;   ///< sample buffer
        Timer timer;                   ///< performance test timer

        std::vector<int> sampleCountBuffer;   ///< sample count buffer

        std::vector<SceneLight *> lights; ///< scene lights

        Camera *camera;       ///< current camera

        // Tonemapping Controls //

        double tm_gamma;                           ///< gamma
        double tm_level;                           ///< exposure level
        double tm_key;                             ///< key value
        double tm_wht;                             ///< white point
    };

}  // namespace CGL

#endif  // __CGL_PATHTRACER_H__
