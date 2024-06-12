#include "android_renderer_backend.hpp"

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/gl/renderable_resource.hpp>

#include <mbgl/util/instrumentation.hpp>

#include <EGL/egl.h>

#include <cassert>

namespace mbgl {
namespace android {

class AndroidGLRenderableResource final : public mbgl::gl::RenderableResource {
public:
    AndroidGLRenderableResource(AndroidRendererBackend& backend_)
        : backend(backend_) {}

    void bind() override {
        MLN_TRACE_FUNC();
        assert(gfx::BackendScope::exists());
        backend.setFramebufferBinding(0);
        backend.setViewport(0, 0, backend.getSize());
    }

    void swap() override {
        MLN_TRACE_FUNC();
        backend.swap();
    }

private:
    AndroidRendererBackend& backend;
};

AndroidRendererBackend::AndroidRendererBackend()
    : gl::RendererBackend(gfx::ContextMode::Unique),
      mbgl::gfx::Renderable({64, 64}, std::make_unique<AndroidGLRenderableResource>(*this)) {}

AndroidRendererBackend::~AndroidRendererBackend() = default;

gl::ProcAddress AndroidRendererBackend::getExtensionFunctionPointer(const char* name) {
    MLN_TRACE_FUNC();
    assert(gfx::BackendScope::exists());
    return eglGetProcAddress(name);
}

void AndroidRendererBackend::updateViewPort() {
    MLN_TRACE_FUNC();
    assert(gfx::BackendScope::exists());
    setViewport(0, 0, size);
}

void AndroidRendererBackend::resizeFramebuffer(int width, int height) {
    size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

PremultipliedImage AndroidRendererBackend::readFramebuffer() {
    MLN_TRACE_FUNC();
    assert(gfx::BackendScope::exists());
    return gl::RendererBackend::readFramebuffer(size);
}

void AndroidRendererBackend::updateAssumedState() {
    MLN_TRACE_FUNC();
    assumeFramebufferBinding(0);
    assumeViewport(0, 0, size);
}

void AndroidRendererBackend::markContextLost() {
    MLN_TRACE_FUNC();
    if (context) {
        getContext<gl::Context>().setCleanupOnDestruction(false);
    }
}

void AndroidRendererBackend::setSwapBehavior(SwapBehaviour swapBehaviour_) {
    MLN_TRACE_FUNC();
    swapBehaviour = swapBehaviour_;
}

void AndroidRendererBackend::swap() {
    MLN_TRACE_FUNC();
    if (swapBehaviour == SwapBehaviour::Flush) {
        static_cast<gl::Context&>(getContext()).finish();
    }
}

} // namespace android
} // namespace mbgl
