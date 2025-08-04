
#include "mesh_buffer.h"
#include <stdexcept>


namespace CGL {


    size_t typeSize(InputType inputType) {
        switch (inputType)
        {
        case Type_Byte:
        case Type_UByte:
            return 1;
        case Type_Short:
        case Type_UShort:
        case Type_HalfFloat:
            return 2;
        case Type_Int:
        case Type_UInt:
        case Type_Float:
            return 4;
        case Type_Double:
            return 8;
        }
        return 0;
    }


    MeshBuffer::MeshBuffer(const std::vector<ShaderInput> &inputFormat, void *vertexData, size_t vertexCount, void *indexData, InputType indexFormat, size_t indexCount, BufferUsage vertexUsage, BufferUsage indexUsage) :
        vertexCount(vertexCount), indexCount(indexCount),
        indexFormat(indexFormat),
        vbo(0), vao(0), ebo(0),
        sizePerVertex(0) {

        for (auto type : inputFormat)
            sizePerVertex += type.count * typeSize(type.type);

        glGenVertexArrays(1, &vao);
        GLenum err = glGetError();
        glGenBuffers(1, &vbo);
        err = glGetError();
        glGenBuffers(1, &ebo);
        err = glGetError();

        glBindVertexArray(vao);
        err = glGetError();

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        err = glGetError();
        glBufferData(GL_ARRAY_BUFFER, vertexCount * sizePerVertex, vertexData, vertexUsage);
        err = glGetError();

        size_t curSize = 0;
        for (size_t i = 0; i < inputFormat.size(); i++) {
            auto type = inputFormat[i];
            glVertexAttribPointer(i, type.count, type.type, GL_FALSE, sizePerVertex, (void *)curSize);
            err = glGetError();
            glEnableVertexAttribArray(i);
            err = glGetError();
            curSize += type.count * typeSize(type.type);
        }

        // Bind and fill EBO with indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        err = glGetError();
        if (indexFormat != Type_Byte && indexFormat != Type_UByte && indexFormat != Type_Short && indexFormat != Type_UShort && indexFormat != Type_Int && indexFormat != Type_UInt)
            throw std::invalid_argument("The input format has to be signed or unsigned byte, short, or int");
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * typeSize(indexFormat), indexData, indexUsage);
        err = glGetError();

        glBindVertexArray(0);

    }
    MeshBuffer::~MeshBuffer() {

    }

    void MeshBuffer::updateVertex(void *vertexData, size_t startIndex, size_t numVertex) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, startIndex * sizePerVertex, numVertex * sizePerVertex, vertexData);
    }
    void MeshBuffer::updateIndex(void *indexData, size_t startIndex, size_t numIndex) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, startIndex * typeSize(indexFormat), numIndex * typeSize(indexFormat), indexData);
    }
    void MeshBuffer::draw() const {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indexCount, indexFormat, NULL);
        glBindVertexArray(0);
    }


    size_t MeshBuffer::getVertexCount() const {
        return vertexCount;
    }
    size_t MeshBuffer::getIndexCount() const {
        return indexCount;
    }
    size_t MeshBuffer::getPerVertexSize() const {
        return sizePerVertex;
    }




}