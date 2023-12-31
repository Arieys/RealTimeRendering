#pragma once

#include <iostream>
#include <string>

#include "../spdlogMgr.h"

#if defined(__EMSCRIPTEN__)
    #include <webgl/webgl2.h>
#elif defined(USE_GLES)
    #include <glad/gles2.h>
#else
    #include <glad/gl.h>
#endif

static struct debugInfoControl
{
    bool showWarningInfo;
    bool showNotificationInfo;
} debugCtr;

inline GLenum implCheckGLErrors(const char* file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM: error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE: error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION: error = "INVALID_OPERATION"; break;
#ifndef __EMSCRIPTEN__
        case GL_STACK_OVERFLOW: error = "STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW: error = "STACK_UNDERFLOW"; break;
#endif
        case GL_OUT_OF_MEMORY: error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        default: error = "UNKNOWN_ERROR"; break;
        }

        std::cerr << error << " | " << file << " (" << line << ")" << std::endl;
    }

    return errorCode;
}

#define checkGLErrors() implCheckGLErrors(__FILE__, __LINE__)

inline void OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* data)
{
    const char* source_str;
    const char* type_str;
    const char* severity_str;

    auto console_logger = spdlogManagement::getConsoleLogHandle();

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        source_str = "SOURCE_API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        source_str = "SOURCE_WINDOW_SYSTEM";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        source_str = "SOURCE_SHADER_COMPILER";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        source_str = "SOURCE_THIRD_PARTY";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        source_str = "SOURCE_APPLICATION";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        source_str = "SOURCE_OTHER";
        break;
    default:
        source_str = "?";
        break;
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        severity_str = "SEVERITY_HIGH";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        severity_str = "SEVERITY_MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        severity_str = "SEVERITY_LOW";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        severity_str = "SEVERITY_NOTIFICATION";
        break;
    default:
        severity_str = "?";
        break;
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        type_str = "TYPE_ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        type_str = "TYPE_DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        type_str = "TYPE_UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        type_str = "TYPE_PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        type_str = "TYPE_PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_OTHER:
        type_str = "TYPE_OTHER";
        break;
    case GL_DEBUG_TYPE_MARKER:
        type_str = "TYPE_MARKER";
        break;
    default:
        type_str = "?";
        break;
    }

    std::string log = "[" + std::string(source_str) + "][" + type_str + "] " + msg;

    if(debugCtr.showWarningInfo && debugCtr.showNotificationInfo) console_logger->set_level(spdlog::level::info);
    else if(debugCtr.showWarningInfo) console_logger->set_level(spdlog::level::warn);
    else console_logger->set_level(spdlog::level::info);
    

    if (type == GL_DEBUG_TYPE_ERROR)
    {
        log = _COLOR_RED + log + _COLOR_DEFAULT;
        console_logger->error(log);
    }
    else
    {
        if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
        {
            if (debugCtr.showWarningInfo) {
                log = _COLOR_YELLOW + log + _COLOR_DEFAULT;
                console_logger->warn(log);
            }
        }
        else
        {
            if (debugCtr.showNotificationInfo) {
                log = _COLOR_GREEN + log + _COLOR_DEFAULT;
                console_logger->info(log);
            }

        }
    }
    
}