

#include "skybox.h"
#include <vector>
#include "image/lodepng.h"

using std::vector;
using std::string;


namespace CGL {

    SkyboxFaces SkyboxFaces::loadFromFiles(
        const string &leftFaceFile,
        const string &rightFaceFile,
        const string &topFaceFile,
        const string &bottomFaceFile,
        const string &frontFaceFile,
        const string &backFaceFile) {
        
        SkyboxFaces faces;


        RawFace *facePtrs[] = {
            &faces.leftFace,
            &faces.rightFace,
            &faces.topFace,
            &faces.bottomFace,
            &faces.frontFace,
            &faces.backFace
        };
        const string filenames[] = {
            leftFaceFile,
            rightFaceFile,
            topFaceFile,
            bottomFaceFile,
            frontFaceFile,
            backFaceFile
        };

        for (int i = 0; i < 6; i++) {
            unsigned error = lodepng::decode(
                facePtrs[i]->face,
                facePtrs[i]->width,
                facePtrs[i]->height,
                filenames[i]);

            if (error != 0) {
                std::cout << "error " << error << ": " << lodepng_error_text(error) << std::endl;
                return {};
            }
        }

        return faces;
    }

    namespace CubeData {

        constexpr float s = 1.0f;
        constexpr float cubeBackVerts[] = {
            -s,  s,  s,  1.0f, 0.0f,
             s,  s,  s,  0.0f, 0.0f,
             s, -s,  s,  0.0f, 1.0f,
            -s, -s,  s,  1.0f, 1.0f
        };
        constexpr float cubeFrontVerts[] = {
            -s,  s, -s,  0.0f, 0.0f,
            -s, -s, -s,  0.0f, 1.0f,
             s, -s, -s,  1.0f, 1.0f,
             s,  s, -s,  1.0f, 0.0f,
        };
        constexpr float cubeLeftVerts[] = {
            -s,  s, -s,  1.0f, 0.0f,
            -s, -s, -s,  1.0f, 1.0f,
            -s, -s,  s,  0.0f, 1.0f,
            -s,  s,  s,  0.0f, 0.0f,
        };
        constexpr float cubeRightVerts[] = {
             s,  s,  s,  1.0f, 0.0f,
             s, -s,  s,  1.0f, 1.0f,
             s, -s, -s,  0.0f, 1.0f,
             s,  s, -s,  0.0f, 0.0f,
        };
        constexpr float cubeBottomVerts[] = {
            -s, -s,  s,  0.0f, 1.0f,
            -s, -s, -s,  0.0f, 0.0f,
             s, -s, -s,  1.0f, 0.0f,
             s, -s,  s,  1.0f, 1.0f,
        };
        constexpr float cubeTopVerts[] = {
            -s,  s, -s,  0.0f, 1.0f,
            -s,  s,  s,  0.0f, 0.0f,
             s,  s,  s,  1.0f, 0.0f,
             s,  s, -s,  1.0f, 1.0f
        };
        constexpr uint32_t cubeIndices[] = { 0, 1, 2,   0, 2, 3, };
    }


    Skybox::Skybox(const SkyboxFaces &faces) {
    
        using namespace CubeData;

        vector<ShaderInput> cubeInputFormat;
        cubeInputFormat.push_back({ Type_Float , 3 });
        cubeInputFormat.push_back({ Type_Float , 2 });

        cubeBack = new MeshBuffer(cubeInputFormat, cubeBackVerts, 4, cubeIndices, Type_UInt, 6, Usage_StaticDraw, Usage_StaticDraw);
        cubeFront = new MeshBuffer(cubeInputFormat, cubeFrontVerts, 4, cubeIndices, Type_UInt, 6, Usage_StaticDraw, Usage_StaticDraw);
        cubeLeft = new MeshBuffer(cubeInputFormat, cubeLeftVerts, 4, cubeIndices, Type_UInt, 6, Usage_StaticDraw, Usage_StaticDraw);
        cubeRight = new MeshBuffer(cubeInputFormat, cubeRightVerts, 4, cubeIndices, Type_UInt, 6, Usage_StaticDraw, Usage_StaticDraw);
        cubeBottom = new MeshBuffer(cubeInputFormat, cubeBottomVerts, 4, cubeIndices, Type_UInt, 6, Usage_StaticDraw, Usage_StaticDraw);
        cubeTop = new MeshBuffer(cubeInputFormat, cubeTopVerts, 4, cubeIndices, Type_UInt, 6, Usage_StaticDraw, Usage_StaticDraw);

        cubeShader = new Shader("Shaders/skybox.vert", "Shaders/skybox.frag");


        viewMatLoc = cubeShader->uniformLocation("view");
        projMatLoc = cubeShader->uniformLocation("projection");
        skyboxTextureLoc = cubeShader->uniformLocation("boxFace");


        glGenTextures(1, &texBack);
        glGenTextures(1, &texFront);
        glGenTextures(1, &texLeft);
        glGenTextures(1, &texRight);
        glGenTextures(1, &texTop);
        glGenTextures(1, &texBottom);

        GLuint textures[] = { texBack, texFront, texLeft, texRight, texTop, texBottom };
        const RawFace *facesArr[] = { &faces.backFace, &faces.frontFace, &faces.leftFace, &faces.rightFace, &faces.topFace, &faces.bottomFace };
        
        for (int i = 0; i < 6; i++) {
            auto &texID = textures[i];
            glBindTexture(GL_TEXTURE_2D, texID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            const RawFace *face = facesArr[i];

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, face->width, face->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, face->face.data());

            glGenerateMipmap(GL_TEXTURE_2D);
        }

        //pathtracerCapture = new SkyboxCapture(this);
    }
    Skybox::~Skybox() {

    }


    void Skybox::setViewMat(const Matrix4x4 &viewMat) {
        Matrix4x4 v = viewMat;
        v(0, 3) = 0.0f; v(1,3) = 0.0f; v(2,3) = 0.0f;
        cubeShader->setMat4x4(viewMatLoc, v);
    }
    void Skybox::setProjMat(const Matrix4x4 &projMat) {
        cubeShader->setMat4x4(projMatLoc, projMat);
    }

    void Skybox::render() {
        GLuint textures[] = { texBack, texFront, texLeft, texRight, texTop, texBottom };
        MeshBuffer *faces[] = { cubeBack, cubeFront, cubeLeft, cubeRight, cubeTop, cubeBottom };

        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);

        for (int i = 0; i < 6; i++) {
            cubeShader->setTex(skyboxTextureLoc, i);
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            cubeShader->useProgram();
            faces[i]->draw();
        }

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }



    PathtracingSkybox::PathtracingSkybox(const SkyboxFaces &faces) : faces(faces) {

    }
    PathtracingSkybox::~PathtracingSkybox() {

    }

    Vector3D PathtracingSkybox::sample(Vector3D dir) const {
        float ax = abs(dir.x), ay = abs(dir.y), az = abs(dir.z);


        if (ax >= ay && ax >= az) {
            if (dir.x > 0) {
                // +X (right)
            } else {
                // -X (left)
            }
        } else if (ay >= ax && ay >= az) {
            if (dir.y > 0) {
                // +Y (top)
            } else {
                // -Y (bottom)
            }
        } else {
            if (dir.z > 0) {
                // +Z (front)
            } else {
                // -Z (back)
            }
        }

        return out;
    }
}