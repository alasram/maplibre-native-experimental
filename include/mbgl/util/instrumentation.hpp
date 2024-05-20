#pragma once

#include <tracy/Tracy.hpp>

#ifndef MLN_RENDER_BACKEND_OPENGL
#define MLN_RENDER_BACKEND_OPENGL 0
#endif

#if MLN_RENDER_BACKEND_OPENGL
#include <mbgl/gl/timestamp_query_extension.hpp>

#define glGenQueries mbgl::gl::extension::glGenQueries
#define glGetQueryiv mbgl::gl::extension::glGetQueryiv
#define glGetQueryObjectiv mbgl::gl::extension::glGetQueryObjectiv
#define glGetInteger64v mbgl::gl::extension::glGetInteger64v
#define glQueryCounter mbgl::gl::extension::glQueryCounter
#define glGetQueryObjectui64v mbgl::gl::extension::glGetQueryObjectui64v
#define GLint mbgl::platform::GLint

#include "tracy/TracyOpenGL.hpp"

#define MLB_TRACE_GL_CONTEXT() TracyGpuContext
#define MLB_TRACE_GL_ZONE(label) TracyGpuZone(#label)
#define MLB_TRACE_FUNC_GL() TracyGpuZone(__FUNCTION__)

#define MLB_END_FRAME()  \
    do {                 \
        FrameMark;       \
        TracyGpuCollect; \
    } while (0)

#define MLB_TRACE_FUNC_WITH_GL()    \
    do {                            \
        ZoneScoped;                 \
        TracyGpuZone(__FUNCTION__); \
    } while (0)

#undef glGenQueries
#undef glGetQueryiv
#undef glGetQueryObjectiv
#undef glGetInteger64v
#undef glQueryCounter
#undef glGetQueryObjectui64v
#undef GLint

#else // MLN_RENDER_BACKEND_OPENGL

#define MLB_TRACE_GL_CONTEXT()
#define MLB_TRACE_GL_ZONE(label)
#define MLB_TRACE_FUNC_GL()
#define MLB_END_FRAME() FrameMark
#define MLB_TRACE_FUNC_WITH_GL() ZoneScoped

#endif // MLN_RENDER_BACKEND_OPENGL

#define MLB_TRACE_FUNC() ZoneScoped
#define MLB_TRACE_ZONE(label) ZoneScopedN(#label)
