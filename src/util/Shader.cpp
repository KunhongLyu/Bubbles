
#include "Shader.h"
#include <fstream>

#include "CGL/CGL.h"
#include "CGL/vector2d.h"
#include "CGL/vector3d.h"
#include "CGL/vector4d.h"
#include "CGL/matrix3x3.h"
#include "CGL/matrix4x4.h"

namespace CGL {

    size_t getFileLength(const std::string &filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return 0;
        }
        return static_cast<size_t>(file.tellg()) + 1;
    }
    void fillBufferWithFileContent(const std::string &filename, char *buffer) {
        std::ifstream file(filename, std::ios::binary);
        if (file.is_open()) {
            size_t length = getFileLength(filename);
            file.read(buffer, length);
            buffer[length - 1] = 0;
        }
    }

    void compileShaderFromFile(const std::string &fileName, GLint shaderID) {
        size_t fileSz = getFileLength(fileName);
        char *source = new char[fileSz];
        fillBufferWithFileContent(fileName, source);
        glShaderSource(shaderID, 1, &source, NULL);
        glCompileShader(shaderID);
        delete[] source;

        GLint success;
        char infoLog[512];
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
            std::cout << "Error creating shader!" << std::endl << "Error message: " << infoLog << std::endl;
        }
    }

    Shader::Shader(const std::string &vsFileName, const std::string &fsFileName) {
        vs = glCreateShader(GL_VERTEX_SHADER);
        compileShaderFromFile(vsFileName, vs);

        fs = glCreateShader(GL_FRAGMENT_SHADER);
        compileShaderFromFile(fsFileName, fs);

        program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);

        glDetachShader(program, vs);
        glDetachShader(program, fs);
        glDeleteShader(vs);
        glDeleteShader(fs);
    }
    
    Shader::~Shader() {
        glDeleteProgram(program);
    }

    void Shader::useProgram() const {
        glUseProgram(program);
    }


    GLint Shader::uniformLocation(const std::string &varName) const {
        return glGetUniformLocation(program, varName.c_str());
    }


    void Shader::setVec1(GLint loc, float f) {
        glUseProgram(program);
        glUniform1f(loc, f);
    }
    void Shader::setVec2(GLint loc, Vector2D v) {
        setVec2(loc, static_cast<float>(v.x), static_cast<float>(v.y));
    }
    void Shader::setVec2(GLint loc, float f0, float f1) {
        glUseProgram(program);
        glUniform2f(loc, f0, f1);
    }
    void Shader::setVec3(GLint loc, Vector3D v) {
        setVec3(loc, static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z));
    }
    void Shader::setVec3(GLint loc, float f0, float f1, float f2) {
        glUseProgram(program);
        glUniform3f(loc, f0, f1, f2);
    }
    void Shader::setVec4(GLint loc, Vector4D v) {
        setVec4(loc, static_cast<float>(v.x), static_cast<float>(v.y), static_cast<float>(v.z), static_cast<float>(v.w));
    }
    void Shader::setVec4(GLint loc, float f0, float f1, float f2, float f3) {
        glUseProgram(program);
        glUniform4f(loc, f0, f1, f2, f3);
    }

    void Shader::setMat3x3(GLint loc, Matrix3x3 m) {
        glUseProgram(program);
        float entries[] = {
            static_cast<float>(m[0].x), static_cast<float>(m[0].y), static_cast<float>(m[0].z),
            static_cast<float>(m[1].x), static_cast<float>(m[1].y), static_cast<float>(m[1].z),
            static_cast<float>(m[2].x), static_cast<float>(m[2].y), static_cast<float>(m[2].z),
        };
        glUniformMatrix3fv(loc, 1, GL_FALSE, entries);

    }
    void Shader::setMat4x4(GLint loc, Matrix4x4 m) {
        glUseProgram(program);
        float entries[] = {
            static_cast<float>(m[0].x), static_cast<float>(m[0].y), static_cast<float>(m[0].z), static_cast<float>(m[0].w),
            static_cast<float>(m[1].x), static_cast<float>(m[1].y), static_cast<float>(m[1].z), static_cast<float>(m[1].w),
            static_cast<float>(m[2].x), static_cast<float>(m[2].y), static_cast<float>(m[2].z), static_cast<float>(m[2].w),
            static_cast<float>(m[3].x), static_cast<float>(m[3].y), static_cast<float>(m[3].z), static_cast<float>(m[3].w),
        };
        glUniformMatrix4fv(loc, 1, GL_FALSE, entries);
    }
}