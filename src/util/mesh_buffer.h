
#ifndef __MESH_BUFFER_H__
#define __MESH_BUFFER_H__

#include <vector>

#include "GL/glew.h"

#include "CGL/CGL.h"
#include "CGL/CGLMath.h"


namespace CGL {

    enum InputType {
        Type_Byte = GL_BYTE,
        Type_UByte = GL_UNSIGNED_BYTE,
        Type_Short = GL_SHORT,
        Type_UShort = GL_UNSIGNED_SHORT,
        Type_Int = GL_INT,
        Type_UInt = GL_UNSIGNED_INT,
        Type_HalfFloat = GL_HALF_FLOAT,
        Type_Float = GL_FLOAT,
        Type_Double = GL_DOUBLE,
    };
    struct ShaderInput {
        InputType type;
        size_t count;
    };
    size_t typeSize(InputType inputType);

    enum BufferUsage {
        Usage_StreamDraw =  GL_STREAM_DRAW,
        Usage_StreamRead =  GL_STREAM_READ,
        Usage_StreamCopy =  GL_STREAM_COPY,
        Usage_StaticDraw =  GL_STATIC_DRAW,
        Usage_StaticRead =  GL_STATIC_READ,
        Usage_StaticCopy =  GL_STATIC_COPY,
        Usage_DynamicDraw = GL_DYNAMIC_DRAW,
        Usage_DynamicRead = GL_DYNAMIC_READ,
        Usage_DynamicCopy = GL_DYNAMIC_COPY
    };

    class MeshBuffer {
    public:
        MeshBuffer(const std::vector<ShaderInput> &inputFormat, void *vertexData, size_t vertexCount, void *indexData, InputType indexFormat, size_t indexCount, BufferUsage vertexUsage, BufferUsage indexUsage);
        virtual ~MeshBuffer();

        void updateVertex(void *vertexData, size_t startIndex, size_t numVertex);
        void updateIndex(void *indexData, size_t startIndex, size_t numIndex);
        void draw() const;

        size_t getVertexCount() const;
        size_t getIndexCount() const;
        size_t getPerVertexSize() const;

    private:
        InputType indexFormat;
        size_t vertexCount, indexCount;
        size_t sizePerVertex;

        GLuint vbo, vao, ebo;
    };

    
}




#endif //__MESH_BUFFER_H__