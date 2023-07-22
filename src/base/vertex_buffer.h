#pragma once

#include "gl_utility.h"

class VertexBuffer
{
public:
    VertexBuffer()
    {
        glGenBuffers(1, &_handle);

        checkGLErrors();
    }

    VertexBuffer(VertexBuffer&& other) noexcept : _handle(other._handle)
    {
        other._handle = static_cast<GLuint>(0);
    }

    ~VertexBuffer()
    {
        if (_handle)
        {
            glDeleteBuffers(1, &_handle);
            _handle = static_cast<GLuint>(0);
        }
    }

    void bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, _handle);
    }

    void unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(0));
    }

    void upload(GLsizeiptr size, const void* data, GLenum usage)
    {
        glBufferData(GL_ARRAY_BUFFER, size, data, usage);
    }

private:
    GLuint _handle{};
};