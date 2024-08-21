#pragma once

#include <mbgl/gfx/renderable.hpp>
#include <mbgl/gl/renderer_backend.hpp>

#include <EGL/egl.h>

namespace mbgl {
namespace android {

#define ANDROID_RENDER_BACKEND_DISABLE_SHARED_EGL_CONTEXTS

#ifdef ANDROID_RENDER_BACKEND_DISABLE_SHARED_EGL_CONTEXTS
#define ANDROID_RENDER_BACKEND_SHARED_EGL_CONTEXTS false
#else
#define ANDROID_RENDER_BACKEND_SHARED_EGL_CONTEXTS true
#endif

class AndroidUploadThreadContext : public gl::UploadThreadContext {
public:
    AndroidUploadThreadContext(AndroidRendererBackend&, EGLDisplay, EGLConfig, EGLContext);
    ~AndroidUploadThreadContext() override;
    void createContext() override;
    void destroyContext() override;
    void bindContext() override;
    void unbindContext() override;

private:
    AndroidRendererBackend& backend;
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLConfig config;
    EGLContext mainContext = EGL_NO_CONTEXT;
    EGLContext sharedContext = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;
};

class AndroidRendererBackend : public gl::RendererBackend, public mbgl::gfx::Renderable {
public:
    AndroidRendererBackend(const TaggedScheduler& threadPool);
    ~AndroidRendererBackend() override;

    void updateViewPort();

    // Ensures the current context is not cleaned up when destroyed
    void markContextLost();

    void resizeFramebuffer(int width, int height);
    PremultipliedImage readFramebuffer();

    void setSwapBehavior(SwapBehaviour swapBehaviour);
    void swap();

    bool supportFreeThreadedUpload() const override { return ANDROID_RENDER_BACKEND_SHARED_EGL_CONTEXTS; }
    std::shared_ptr<UploadThreadContext> createUploadThreadContext() override;
    void initFreeThreadedUpload() override;

private:
    SwapBehaviour swapBehaviour = SwapBehaviour::NoFlush;

    EGLContext eglMainCtx = EGL_NO_CONTEXT;
    EGLDisplay eglDsply = EGL_NO_DISPLAY;
    EGLSurface eglSurf = EGL_NO_SURFACE;
    EGLConfig eglConfig;

    // mbgl::gfx::RendererBackend implementation
public:
    mbgl::gfx::Renderable& getDefaultRenderable() override { return *this; }

protected:
    void activate() override {
        // no-op
    }
    void deactivate() override {
        // no-op
    }

    // mbgl::gl::RendererBackend implementation
protected:
    mbgl::gl::ProcAddress getExtensionFunctionPointer(const char*) override;
    void updateAssumedState() override;
};

} // namespace android
} // namespace mbgl
