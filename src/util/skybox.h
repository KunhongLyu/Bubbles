
#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include <string>
#include <vector>
#include "mesh_buffer.h"
#include "shader.h"
#include "../pathtracer/bvh.h"

using std::string;
using std::vector;


namespace CGL {
    class Skybox {
    public:
        Skybox(string leftFaceFile, 
               string rightFaceFile,
               string topFaceFile,
               string bottomFaceFile,
               string frontFaceFile,
               string backFaceFile);
        ~Skybox();

        void setViewMat(const Matrix4x4 &viewMat);
        void setProjMat(const Matrix4x4 &projMat);

        void render();

        MeshPathtracerCapture *getPathtracerCapture() const {
            return pathtracerCapture;
        };


    private:

        class SkyboxCapture : public MeshPathtracerCapture {
        public:
            SkyboxCapture(Skybox *par) : parentSkybox(par) {};

        protected:
            virtual void create_ptr(MeshablePathtracer **) const;
            virtual void update_cur_ptr(MeshablePathtracer **) const;
            virtual void release_ptr(MeshablePathtracer *) const;

        private:
            Skybox *parentSkybox;

        };

        SkyboxCapture *pathtracerCapture;

        MeshBuffer *cubeBack, *cubeFront, *cubeLeft, *cubeRight, *cubeTop, *cubeBottom;

        GLuint texBack, texFront, texLeft, texRight, texTop, texBottom;

        Shader *cubeShader;

        GLint viewMatLoc;
        GLint projMatLoc;

        GLint skyboxTextureLoc;

        vector<unsigned char> imgBack, imgFront, imgLeft, imgRight, imgTop, imgBottom;


        
    };
}





#endif // __SKYBOX_GL_H__