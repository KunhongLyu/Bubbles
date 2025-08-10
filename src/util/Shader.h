
#ifndef __SHADER_H__
#define __SHADER_H__

#include <string>

#include "GL/glew.h"

#include "CGL/CGL.h"


namespace CGL {

    class Shader {
    public:
        Shader(const std::string &vsFileName, const std::string &fsFileName);
        ~Shader();

        GLint uniformLocation(const std::string &varName) const;

        void setVec1(GLint loc, float f);
        void setVec2(GLint loc, Vector2D v);
        void setVec2(GLint loc, float f0, float f1);
        void setVec3(GLint loc, Vector3D v);
        void setVec3(GLint loc, float f0, float f1, float f2);
        void setVec4(GLint loc, Vector4D v);
        void setVec4(GLint loc, float f0, float f1, float f2, float f3);

        void setMat3x3(GLint loc, Matrix3x3 m);
        void setMat4x4(GLint loc, Matrix4x4 m);

        void setTex(GLint loc, GLuint texUnit);

        void useProgram() const;
        static void removeProgram();

    public:
        GLuint vs, fs;
        GLuint program;
    };
}





#endif //__SHADER_H__