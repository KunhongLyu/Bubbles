#include "camera.h"

#include <iostream>
#include <sstream>
#include <fstream>

#include "CGL/misc.h"
#include "CGL/vector2D.h"
#include "CGL/vector3D.h"

using std::cout;
using std::endl;
using std::max;
using std::min;
using std::ifstream;
using std::ofstream;
using std::string;

namespace CGL {


    void Camera::configure(double hFov, double vFov, double near, double far, size_t screenW, size_t screenH) {
        this->screenW = screenW;
        this->screenH = screenH;
        nClip = near;
        fClip = far;
        this->hFov = hFov;
        this->vFov = vFov;

        double ar1 = tan(radians(hFov) / 2) / tan(radians(vFov) / 2);
        ar = static_cast<double>(screenW) / screenH;
        if (ar1 < ar) {
            // hFov is too small
            hFov = 2 * degrees(atan(tan(radians(vFov) / 2) * ar));
        } else if (ar1 > ar) {
            // vFov is too small
            vFov = 2 * degrees(atan(tan(radians(hFov) / 2) / ar));
        }
        screenDist = ((double)screenH) / (2.0 * tan(radians(vFov) / 2));
    }

    void Camera::place(const Vector3D targetPos, const double phi,
        const double theta, const double r, const double minR,
        const double maxR) {
        double r_ = min(max(r, minR), maxR);
        double phi_ = (sin(phi) == 0) ? (phi + EPS_F) : phi;
        this->targetPos = targetPos;
        this->phi = phi_;
        this->theta = theta;
        this->r = r_;
        this->minR = minR;
        this->maxR = maxR;
        compute_position();
    }

    /**
     * This function copies the camera placement state.
     */
    void Camera::copy_placement(const Camera &other) {
        pos = other.pos;
        targetPos = other.targetPos;
        phi = other.phi;
        theta = other.theta;
        minR = other.minR;
        maxR = other.maxR;
        c2w = other.c2w;
    }

    /**
     * This sets the screen size & compute the new FOV.
     */
    void Camera::set_screen_size(const size_t screenW, const size_t screenH) {
        this->screenW = screenW;
        this->screenH = screenH;
        ar = 1.0 * screenW / screenH;
        hFov = 2 * degrees(atan(((double)screenW) / (2 * screenDist)));
        vFov = 2 * degrees(atan(((double)screenH) / (2 * screenDist)));
    }

    void Camera::move_by(const double dx, const double dy, const double d) {
        const double scaleFactor = d / screenDist;
        const Vector3D displacement =
            c2w[0] * (dx * scaleFactor) + c2w[1] * (dy * scaleFactor);
        pos += displacement;
        targetPos += displacement;
    }

    /**
     * This function translates the camera position (in forward direction)
     */
    void Camera::move_forward(const double dist) {
        double newR = min(max(r - dist, minR), maxR);
        pos = targetPos + ((pos - targetPos) * (newR / r));
        r = newR;
    }

    /**
     * This function rotates the camera position
     */
    void Camera::rotate_by(const double dPhi, const double dTheta) {
        phi = clamp(phi + dPhi, 0.0, (double)PI);
        theta += dTheta;
        compute_position();
    }

    /**
     * This function computes the camera position, basis vectors, and the view matrix
     */
    void Camera::compute_position() {
        double sinPhi = sin(phi);
        if (sinPhi == 0) {
            phi += EPS_F;
            sinPhi = sin(phi);
        }
        const Vector3D dirToCamera(r * sinPhi * sin(theta),
            r * cos(phi),
            r * sinPhi * cos(theta));
        pos = targetPos + dirToCamera;
        Vector3D upVec(0, sinPhi > 0 ? 1 : -1, 0);
        Vector3D screenXDir = cross(upVec, dirToCamera);
        screenXDir.normalize();
        Vector3D screenYDir = cross(dirToCamera, screenXDir);
        screenYDir.normalize();

        c2w[0] = screenXDir;
        c2w[1] = screenYDir;
        c2w[2] = dirToCamera.unit();   // camera's view direction is the
        // opposite of of dirToCamera, so
        // directly using dirToCamera as
        // column 2 of the matrix takes [0 0 -1]
        // to the world space view direction
    }

    /**
     * This function stores the camera settings into a file
     */
    void Camera::dump_settings(string filename) {
        ofstream file(filename);
        file << hFov << " " << vFov << " " << ar << " " << nClip << " " << fClip << endl;
        for (int i = 0; i < 3; ++i)
            file << pos[i] << " ";
        for (int i = 0; i < 3; ++i)
            file << targetPos[i] << " ";
        file << endl;
        file << phi << " " << theta << " " << r << " " << minR << " " << maxR << endl;
        for (int i = 0; i < 9; ++i)
            file << c2w(i / 3, i % 3) << " ";
        file << endl;
        file << screenW << " " << screenH << " " << screenDist << endl;
        file << focalDistance << " " << lensRadius << endl;
        cout << "[Camera] Dumped settings to " << filename << endl;
    }

    /**
     * This function loads the camera settings from a file
     */
    void Camera::load_settings(string filename) {
        ifstream file(filename);

        file >> hFov >> vFov >> ar >> nClip >> fClip;
        for (int i = 0; i < 3; ++i)
            file >> pos[i];
        for (int i = 0; i < 3; ++i)
            file >> targetPos[i];
        file >> phi >> theta >> r >> minR >> maxR;
        for (int i = 0; i < 9; ++i)
            file >> c2w(i / 3, i % 3);
        file >> screenW >> screenH >> screenDist;
        file >> focalDistance >> lensRadius;
        cout << "[Camera] Loaded settings from " << filename << endl;
    }

    /**
     * This function generates a ray from camera perspective, passing through camera / sensor plane (x,y)
     */
    Ray Camera::generate_ray(double x, double y) const {

        double hfov_rad = hFov * PI / 180.0;
        double vfov_rad = vFov * PI / 180.0;
        double tan_half_h = tan(hfov_rad / 2.0);
        double tan_half_v = tan(vfov_rad / 2.0);

        // px and py in camera space (left/bottom at negative, center at 0, right/top at positive).
        double px = (2.0 * x - 1.0) * tan_half_h;
        double py = (2.0 * y - 1.0) * tan_half_v;

        Vector3D p_cam(px, py, -1);

        // Normalize direction in camera space.
        p_cam.normalize();

        // Transform direction to world space using rotation matrix.
        Vector3D dir_world = c2w * p_cam;

        // Create ray with origin at camera position, unit direction in world space.
        Ray r(pos, dir_world);
        r.min_t = nClip;
        r.max_t = fClip;

        return r;
    }

    Matrix4x4 Camera::get_view_matrix() const {
        Matrix4x4 view(Matrix4x4::identity());

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                view(i, j) = c2w[j][i];
            }
        }

        for (int i = 0; i < 3; ++i) {
            view(i, 3) = -dot(Vector3D(c2w(i, 0), c2w(i, 1), c2w(i, 2)), pos);
        }

        return view;
    }

} // namespace CGL
