#pragma once

#ifdef MLN_TRACY_ENABLE

#include <tracy/Tracy.hpp>

#ifndef MLN_RENDER_BACKEND_OPENGL
#error \
    "MLN_RENDER_BACKEND_OPENGL is not defined. MLN_RENDER_BACKEND_OPENGL is expected to be defined in CMake and Bazel"
#endif

#define MLN_TRACE_FUNC() ZoneScoped
#define MLN_TRACE_ZONE(label) ZoneScopedN(#label)

constexpr const char* tracyTextureMemoryLabel = "Texture Memory";
#define MLN_TRACE_ALLOC_TEXTURE(id, size) TracyAllocN(reinterpret_cast<const void*>(id), size, tracyTextureMemoryLabel)
#define MLN_TRACE_FREE_TEXTURE(id) TracyFreeN(reinterpret_cast<const void*>(id), tracyTextureMemoryLabel)

constexpr const char* tracyRenderTargetMemoryLabel = "Render Target Memory";
#define MLN_TRACE_ALLOC_RT(id, size) TracyAllocN(reinterpret_cast<const void*>(id), size, tracyRenderTargetMemoryLabel)
#define MLN_TRACE_FREE_RT(id) TracyFreeN(reinterpret_cast<const void*>(id), tracyRenderTargetMemoryLabel)

constexpr const char* tracyVertexMemoryLabel = "Vertex Buffer Memory";
#define MLN_TRACE_ALLOC_VERTEX_BUFFER(id, size) \
    TracyAllocN(reinterpret_cast<const void*>(id), size, tracyVertexMemoryLabel)
#define MLN_TRACE_FREE_VERTEX_BUFFER(id) TracyFreeN(reinterpret_cast<const void*>(id), tracyVertexMemoryLabel)

constexpr const char* tracyIndexMemoryLabel = "Index Buffer Memory";
#define MLN_TRACE_ALLOC_INDEX_BUFFER(id, size) \
    TracyAllocN(reinterpret_cast<const void*>(id), size, tracyIndexMemoryLabel)
#define MLN_TRACE_FREE_INDEX_BUFFER(id) TracyFreeN(reinterpret_cast<const void*>(id), tracyIndexMemoryLabel)

constexpr const char* tracyConstMemoryLabel = "Constant Buffer Memory";
#define MLN_TRACE_ALLOC_CONST_BUFFER(id, size) \
    TracyAllocN(reinterpret_cast<const void*>(id), size, tracyConstMemoryLabel)
#define MLN_TRACE_FREE_CONST_BUFFER(id) TracyFreeN(reinterpret_cast<const void*>(id), tracyConstMemoryLabel)

// Only OpenGL is currently considered for GPU profiling
// Metal and other APIs need to be handled separately
#if MLN_RENDER_BACKEND_OPENGL

#include <mbgl/gl/timestamp_query_extension.hpp>

// TracyOpenGL.hpp assumes OpenGL functions are in the global namespace
// Temporarily expose the functions to TracyOpenGL.hpp then undef
#define glGenQueries mbgl::gl::extension::glGenQueries
#define glGetQueryiv mbgl::gl::extension::glGetQueryiv
#define glGetQueryObjectiv mbgl::gl::extension::glGetQueryObjectiv
#define glGetInteger64v mbgl::gl::extension::glGetInteger64v
#define glQueryCounter mbgl::gl::extension::glQueryCounter
#define glGetQueryObjectui64v mbgl::gl::extension::glGetQueryObjectui64v
#define GLint mbgl::platform::GLint

#include "tracy/TracyOpenGL.hpp"

#define MLN_TRACE_GL_CONTEXT() TracyGpuContext
#define MLN_TRACE_GL_ZONE(label) TracyGpuZone(#label)
#define MLN_TRACE_FUNC_GL() TracyGpuZone(__FUNCTION__)

#define MLN_END_FRAME()  \
    do {                 \
        FrameMark;       \
        TracyGpuCollect; \
    } while (0)

#undef glGenQueries
#undef glGetQueryiv
#undef glGetQueryObjectiv
#undef glGetInteger64v
#undef glQueryCounter
#undef glGetQueryObjectui64v
#undef GLint

#else // MLN_RENDER_BACKEND_OPENGL

#define MLN_TRACE_GL_CONTEXT()
#define MLN_TRACE_GL_ZONE(label)
#define MLN_TRACE_FUNC_GL()
#define MLN_END_FRAME() FrameMark

#endif // MLN_RENDER_BACKEND_OPENGL

#else // MLN_TRACY_ENABLE

#define MLN_TRACE_GL_CONTEXT()
#define MLN_TRACE_GL_ZONE(label)
#define MLN_TRACE_FUNC_GL()
#define MLN_END_FRAME()
#define MLN_TRACE_ALLOC_TEXTURE(id, size)
#define MLN_TRACE_FREE_TEXTURE(id)
#define MLN_TRACE_ALLOC_RT(id, size)
#define MLN_TRACE_FREE_RT(id)
#define MLN_TRACE_ALLOC_VERTEX_BUFFER(id, size)
#define MLN_TRACE_FREE_VERTEX_BUFFER(id)
#define MLN_TRACE_ALLOC_INDEX_BUFFER(id, size)
#define MLN_TRACE_FREE_INDEX_BUFFER(id)
#define MLN_TRACE_ALLOC_CONST_BUFFER(id, size)
#define MLN_TRACE_FREE_CONST_BUFFER(id)
#define MLN_TRACE_FUNC()
#define MLN_TRACE_ZONE(label)

#endif // MLN_TRACY_ENABLE