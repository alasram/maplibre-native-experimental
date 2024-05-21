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

constexpr const char* tracyTextureMemoryLabel = "Texture Memory";
#define MLB_TRACE_ALLOC_TEXTURE(id, size) TracyAllocN(reinterpret_cast<const void*>(id), size, tracyTextureMemoryLabel)
#define MLB_TRACE_FREE_TEXTURE(id) TracyFreeN(reinterpret_cast<const void*>(id), tracyTextureMemoryLabel)

constexpr const char* tracyRenderTargetMemoryLabel = "Render Target Memory";
#define MLB_TRACE_ALLOC_RT(id, size) TracyAllocN(reinterpret_cast<const void*>(id), size, tracyRenderTargetMemoryLabel)
#define MLB_TRACE_FREE_RT(id) TracyFreeN(reinterpret_cast<const void*>(id), tracyRenderTargetMemoryLabel)

constexpr const char* tracyVertexMemoryLabel = "Vertex Buffer Memory";
#define MLB_TRACE_ALLOC_VERTEX_BUFFER(id, size) \
    TracyAllocN(reinterpret_cast<const void*>(id), size, tracyVertexMemoryLabel)
#define MLB_TRACE_FREE_VERTEX_BUFFER(id) TracyFreeN(reinterpret_cast<const void*>(id), tracyVertexMemoryLabel)

constexpr const char* tracyIndexMemoryLabel = "Index Buffer Memory";
#define MLB_TRACE_ALLOC_INDEX_BUFFER(id, size) \
    TracyAllocN(reinterpret_cast<const void*>(id), size, tracyIndexMemoryLabel)
#define MLB_TRACE_FREE_INDEX_BUFFER(id) TracyFreeN(reinterpret_cast<const void*>(id), tracyIndexMemoryLabel)

constexpr const char* tracyConstMemoryLabel = "Constant Buffer Memory";
#define MLB_TRACE_ALLOC_CONST_BUFFER(id, size) \
    TracyAllocN(reinterpret_cast<const void*>(id), size, tracyConstMemoryLabel)
#define MLB_TRACE_FREE_CONST_BUFFER(id) TracyFreeN(reinterpret_cast<const void*>(id), tracyConstMemoryLabel)

#else // MLN_RENDER_BACKEND_OPENGL

#define MLB_TRACE_GL_CONTEXT()
#define MLB_TRACE_GL_ZONE(label)
#define MLB_TRACE_FUNC_GL()
#define MLB_END_FRAME() FrameMark
#define MLB_TRACE_FUNC_WITH_GL() ZoneScoped
#define MLB_TRACE_ALLOC_TEXTURE(id, size)
#define MLB_TRACE_FREE_TEXTURE(id)
#define MLB_TRACE_ALLOC_RT(id, size)
#define MLB_TRACE_FREE_RT(id)
#define MLB_TRACE_ALLOC_VERTEX_BUFFER(id, size)
#define MLB_TRACE_FREE_VERTEX_BUFFER(id)
#define MLB_TRACE_ALLOC_INDEX_BUFFER(id, size)
#define MLB_TRACE_FREE_INDEX_BUFFER(id)
#define MLB_TRACE_ALLOC_CONST_BUFFER(id, size)
#define MLB_TRACE_FREE_CONST_BUFFER(id)

#endif // MLN_RENDER_BACKEND_OPENGL

#define MLB_TRACE_FUNC() ZoneScoped
#define MLB_TRACE_ZONE(label) ZoneScopedN(#label)
