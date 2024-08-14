#pragma once

#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/util/thread_pool.hpp>

#include <cassert>
#include <thread>

namespace mbgl {
namespace gfx {

class ResourceUploadThreadPool : public ThreadedScheduler {
public:
    ResourceUploadThreadPool(RendererBackend& backend)
        : ThreadedScheduler(numberOfResourceUploadThreads(), false, makeThreadCallbacks(backend)) {}

private:
    static size_t numberOfResourceUploadThreads() {
        size_t hwThreads = std::thread::hardware_concurrency();
        if (hwThreads < 2) {
            return 1;
        }
        // Set the pool size to the number of threads minus one to account for the main render thread
        return std::thread::hardware_concurrency() - 1;
    }

    static std::function<ThreadCallbacks()> makeThreadCallbacks(RendererBackend& backend) {
        assert(backend.supportFreeThreadedUpload());

        // callbacks will run in a separate thread. It is assumed the backend exists during upload

        auto callbackGen = [ctx = backend.createUploadThreadContext()]() {
            ThreadCallbacks callbacks;
            callbacks.onThreadBegin = [ctx = ctx]() {
                ctx->createContext();
            };

            callbacks.onThreadEnd = [ctx = ctx]() {
                ctx->destroyContext();
            };

            callbacks.onTaskBegin = [ctx = ctx]() {
                ctx->bindContext();
            };

            callbacks.onTaskEnd = [ctx = ctx]() {
                ctx->unbindContext();
            };
            return callbacks;
        };

        return callbackGen;
    }
};

} // namespace gfx
} // namespace mbgl
