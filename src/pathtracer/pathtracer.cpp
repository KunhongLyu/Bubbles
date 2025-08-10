#include "pathtracer.h"



namespace CGL {

    PathTracer::PathTracer() {
        gridSampler = new UniformGridSampler2D();
        hemisphereSampler = new UniformHemisphereSampler3D();

        tm_gamma = 2.2f;
        tm_level = 1.0f;
        tm_key = 0.18;
        tm_wht = 5.0f;
    }

    PathTracer::~PathTracer() {
        delete gridSampler;
        delete hemisphereSampler;
    }

    void PathTracer::set_frame_size(size_t width, size_t height) {
        sampleBuffer.resize(width, height);
        sampleCountBuffer.resize(width * height);
    }

    void PathTracer::clear() {
        bvh = NULL;
        camera = NULL;
        sampleBuffer.clear();
        sampleCountBuffer.clear();
        sampleBuffer.resize(0, 0);
        sampleCountBuffer.resize(0, 0);
    }

    void PathTracer::write_to_framebuffer(ImageBuffer &framebuffer, size_t x0,
        size_t y0, size_t x1, size_t y1) {
        sampleBuffer.toColor(framebuffer, x0, y0, x1, y1);
    }

    Vector3D
        PathTracer::estimate_direct_lighting_hemisphere(const Ray &r,
            const Intersection &isect) {
        // Estimate the lighting from this intersection coming directly from a light.
        // For this function, sample uniformly in a hemisphere.

        // Note: When comparing Cornel Box (CBxxx.dae) results to importance sampling, you may find the "glow" around the light source is gone.
        // This is totally fine: the area lights in importance sampling has directionality, however in hemisphere sampling we don't model this behaviour.

        // make a coordinate system for a hit point
        // with N aligned with the Z direction.
		Vector3D n = isect.n;
		if (dot(n,r.d) >  0) n = -n;
        Matrix3x3 o2w;
        make_coord_space(o2w, n);
        Matrix3x3 w2o = o2w.T();

        // w_out points towards the source of the ray (e.g.,
        // toward the camera if this is a primary ray)
        const Vector3D hit_p = r.o + r.d * isect.t;
        const Vector3D w_out = w2o * (-r.d);

        // This is the same number of total samples as
        // estimate_direct_lighting_importance (outside of delta lights). We keep the
        // same number of samples for clarity of comparison.
        int num_samples = lights.size() * ns_area_light;
        num_samples = num_samples;
        Vector3D L_out;

        for (int i = 0; i < num_samples; i++) {
            Vector3D wi_loc = hemisphereSampler->get_sample();
            Vector3D wi_world = o2w * wi_loc;
            Ray new_r(hit_p, wi_world);
            new_r.min_t = EPS_F;
            Intersection new_i;
            if (bvh->intersect(new_r, &new_i)) {
                Vector3D Li = new_i.bsdf->get_emission();
                L_out += (isect.bsdf->f(w_out, wi_loc) * Li) * (abs_cos_theta(wi_loc) * (2 * PI));
            }
        }
        return L_out / num_samples;

    }

    Vector3D
        PathTracer::estimate_direct_lighting_importance(const Ray &r,
            const Intersection &isect) {
        // Estimate the lighting from this intersection coming directly from a light.
        // To implement importance sampling, sample only from lights, not uniformly in
        // a hemisphere.

        // make a coordinate system for a hit point
        // with N aligned with the Z direction.
        Vector3D n = isect.n;
        if (dot(n, r.d) > 0) n = -n;
        Matrix3x3 o2w;
        make_coord_space(o2w, n);
        Matrix3x3 w2o = o2w.T();

        // w_out points towards the source of the ray (e.g.,
        // toward the camera if this is a primary ray)
        const Vector3D hit_p = r.o + r.d * isect.t;
        const Vector3D w_out = w2o * (-r.d);
        Vector3D L_out;

        for (const auto &light : lights) {
            int n_samples = light->is_delta_light() ? 1 : ns_area_light;
            Vector3D contrib(0.0);
            for (int i = 0; i < n_samples; i++) {
                Vector3D wi_world;
                double distToLight, pdf;
                Vector3D Li = light->sample_L(hit_p, &wi_world, &distToLight, &pdf);
                Vector3D wi_local = w2o * wi_world;
                if (wi_local.z <= 0) continue;
                Ray shadow_ray(hit_p, wi_world);
                shadow_ray.min_t = EPS_F;
                Intersection dummy;
                if (bvh->intersect(shadow_ray, &dummy) && dummy.t + EPS_F < distToLight)
                    continue;
                contrib += (isect.bsdf->f(w_out, wi_local) * Li) * (abs_cos_theta(wi_local) / pdf);
            }
            L_out += contrib / (double)n_samples;
        }

        return L_out;
    }

    Vector3D PathTracer::zero_bounce_radiance(const Ray &r,
        const Intersection &isect) {

        return isect.bsdf->get_emission();

    }

    Vector3D PathTracer::one_bounce_radiance(const Ray &r,
        const Intersection &isect) {

        //if (direct_hemisphere_sample)
        //    return estimate_direct_lighting_hemisphere(r, isect);
        if (isect.bsdf->is_delta()) {
            return Vector3D();
        }
        return estimate_direct_lighting_importance(r, isect);
    }

    template<bool use_roulette>
    Vector3D PathTracer::at_least_one_bounce_radiance_internal(const Ray &r,
        const Intersection &isect) {
        Vector3D n = isect.n;
        if (dot(n, r.d) > 0) n = -n;
        Matrix3x3 o2w;
        make_coord_space(o2w, n);
        Matrix3x3 w2o = o2w.T();

        Vector3D hit_p = r.o + r.d * isect.t;
        Vector3D w_out = w2o * (-r.d);

        Vector3D L_out;

        // Returns the one bounce radiance + radiance from extra bounces at this point.
        // Should be called recursively to simulate extra bounces.

        if (isAccumBounces || r.depth == 1) {
            Vector3D one_bounce = one_bounce_radiance(r, isect);
            L_out += one_bounce;
        }
        if (r.depth == 1)
            return L_out;

        //isAccumBounces = true;
        //r.depth == 0; // should just return zero_bounce_radiance(r, isect);
        //r.depth == 1; // should return zero_bounce_radiance(r, isect) + one_bounce_radiance(r, isect);
        //r.depth > 1;  // should return zero_bounce_radiance(r, isect) + one_bounce_radiance(r, isect) + higher;
        //
        //isAccumBounces = false;
        //r.depth == 0; // should just return zero_bounce_radiance(r, isect);
        //r.depth == 1; // should return one_bounce_radiance(r, isect);
        //r.depth > 1;  // should return higher;

        if (use_roulette) {
            if (!coin_flip(roulette_prob))
                return L_out;
        }


        Vector3D bounced;
        size_t bounce_count = 1;
        for (size_t i = 0; i < bounce_count; i++) {
            Vector3D wi_local;
            double pdf;
            Vector3D bsdf_val = isect.bsdf->sample_f(w_out, &wi_local, &pdf);

            if (pdf <= 0.0) {
                continue;
            }

            Vector3D wi_world = o2w * wi_local;

            bool transmitted = (wi_local.z * w_out.z < 0.0);
            Vector3D n = isect.n;
            if (dot(n, r.d) > 0) n = -n;
            Vector3D offset_dir = transmitted ? (-n) : (n);

            Ray new_r(hit_p + offset_dir * 1e-4, wi_world.unit());
            new_r.min_t = EPS_F;
            new_r.depth = r.depth - 1;
            Intersection new_isect;
            Vector3D Li;
            if (bvh->intersect(new_r, &new_isect)) {
                Li = new_isect.bsdf->get_emission();
                if (new_r.depth > 0) {
                    Li += at_least_one_bounce_radiance_internal<use_roulette>(new_r, new_isect);
                }
            }
            else {
				Li = Vector3D(1.0, 1.0, 1.0);;
            }

            if (use_roulette)
                bounced += (bsdf_val * abs_cos_theta(wi_local) * roulette_prob_inv / pdf) * Li;
            else
                bounced += (bsdf_val * abs_cos_theta(wi_local) / pdf) * Li;
        }
        L_out += bounced / bounce_count;

        return L_out;
    }


    Vector3D PathTracer::at_least_one_bounce_radiance(const Ray &r,
        const Intersection &isect) {
        if (use_roulette_stopping)
            return at_least_one_bounce_radiance_internal<true>(r, isect);
        return at_least_one_bounce_radiance_internal<false>(r, isect);
    }

    Vector3D PathTracer::est_radiance_global_illumination(const Ray &r) {
        Intersection isect;
        Vector3D L_out;

        if (!bvh->intersect(r, &isect))
            return Vector3D(1.0, 1.0, 1.0);

        if (isect.t == INF_D)
            return debug_shading(r.d);
        //L_out += normal_shading(isect.n);
        //L_out = zero_bounce_radiance(r, isect) + estimate_direct_lighting_hemisphere(r, isect);
        if (isAccumBounces || r.depth == 0)
            L_out += zero_bounce_radiance(r, isect);
        if (r.depth > 0)
            L_out += at_least_one_bounce_radiance(r, isect);

        return L_out;
    }

    void PathTracer::raytrace_pixel(size_t x, size_t y) {
        int num_samples = ns_aa;          // total samples to evaluate
        Vector2D origin = Vector2D(x, y); // bottom left corner of the pixel
        Vector3D color;
        double s1 = 0, s2 = 0;
        //cout << "Rendering pixel (" << x << "," << y << ")" << endl;

        if (x == 400 && y == 300) {
            int aoiwghaogoaiw = 0;
        }

        for (int i = 1; i <= num_samples; i++) {
            Vector2D pixel = origin + gridSampler->get_sample();
            pixel.x /= sampleBuffer.w;
            pixel.y /= sampleBuffer.h;
            Ray r = camera->generate_ray(pixel.x, pixel.y);
            r.depth = max_ray_depth;
            Vector3D cur_color = est_radiance_global_illumination(r);
            color += cur_color;

            double illuminance = cur_color.illum();
            s1 += illuminance;
            s2 += illuminance * illuminance;
            if (i % samplesPerBatch == 0) {
                double mean = s1 / i;
                double sigma2;
                if (i > 1) sigma2 = (s2 - mean * s1) / (i - 1);
                else sigma2 = 0;
                double I = 1.96 * sqrt(sigma2 / i);
                //cout << i << "th mean: " << mean << " and " << i << "th std: " << sigma2 << endl;
                //cout << "Confidence: " << I << " and maxtol*mu=" << maxTolerance * mean << endl;

                if (i > 1 && I <= maxTolerance * mean) {
                    num_samples = i;
                    break;
                }
            }
        }
        //cout << "Pixel (" << x << "," << y << ") terminated with " << num_samples << " samples." << endl;

        sampleBuffer.update_pixel(color / num_samples, x, y);
        sampleCountBuffer[x + y * sampleBuffer.w] = num_samples;
    }

    void PathTracer::autofocus(Vector2D loc) {
        Ray r = camera->generate_ray(loc.x / sampleBuffer.w, loc.y / sampleBuffer.h);
        Intersection isect;

        bvh->intersect(r, &isect);

        camera->focalDistance = isect.t;
    }

} // namespace CGL
