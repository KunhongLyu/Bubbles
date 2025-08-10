
#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include <string>
#include <vector>
#include "mesh_buffer.h"
#include "shader.h"
#include "../pathtracer/bvh.h"
#include "CGL/CGLMath.h"

using std::string;
using std::vector;


namespace CGL {

    struct RawFace {
        vector<unsigned char> face;
        unsigned int width, height;
    };

    struct SkyboxFaces {
        RawFace leftFace;
        RawFace rightFace;
        RawFace topFace;
        RawFace bottomFace;
        RawFace frontFace;
        RawFace backFace;

        static SkyboxFaces loadFromFiles(
            const string &leftFaceFile,
            const string &rightFaceFile,
            const string &topFaceFile,
            const string &bottomFaceFile,
            const string &frontFaceFile,
            const string &backFaceFile);
    };

    class Skybox {
    public:
        Skybox(const SkyboxFaces &faces);
        ~Skybox();

        void setViewMat(const Matrix4x4 &viewMat);
        void setProjMat(const Matrix4x4 &projMat);

        void render();

    private:

        MeshBuffer *cubeBack, *cubeFront, *cubeLeft, *cubeRight, *cubeTop, *cubeBottom;

        GLuint texBack, texFront, texLeft, texRight, texTop, texBottom;

        Shader *cubeShader;

        GLint viewMatLoc;
        GLint projMatLoc;

        GLint skyboxTextureLoc;
    };

    class PathtracingSkybox {
    public:
        PathtracingSkybox(const SkyboxFaces &faces);
        ~PathtracingSkybox();

        Vector3D sample(Vector3D dir) const;

    private:
        SkyboxFaces faces;

    };
}





#endif // __SKYBOX_GL_H__