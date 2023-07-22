#pragma once

#include "gl_utility.h"

class VertexArray
{
public:
    VertexArray()
    {
        glGenVertexArrays(1, &_handle);

        checkGLErrors();
    }

    VertexArray(VertexArray&& other) noexcept : _handle(other._handle)
    {
        other._handle = static_cast<GLuint>(0);
    }

    ~VertexArray()
    {
        if (_handle)
        {
            glDeleteVertexArrays(1, &_handle);
            _handle = static_cast<GLuint>(0);
        }
    }

    void enableVertexAttribute(
        GLuint      index,
        GLint       size,
        GLenum      type,
        GLboolean   normalized,
        GLsizei     stride,
        const void* pointer)
    {
        glVertexAttribPointer(index, size, type, normalized, stride, pointer);
        glEnableVertexAttribArray(index);
    }

    void bind() const
    {
        glBindVertexArray(_handle);
    }

    void unbind() const
    {
        glBindVertexArray(static_cast<GLuint>(0));
    }

private:
    GLuint _handle;
};