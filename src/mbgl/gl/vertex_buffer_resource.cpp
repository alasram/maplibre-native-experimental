#include <mbgl/gl/context.hpp>
#include <mbgl/gl/vertex_buffer_resource.hpp>
#include <mbgl/util/instrumentation.hpp>

#include <cassert>
#include <cstring>

namespace mbgl {
namespace gl {

using namespace platform;

VertexBufferResource::VertexBufferResource(UniqueBuffer&& buffer_, int byteSize_)
    : buffer(std::make_unique<UniqueBuffer>(std::move(buffer_))),
      byteSize(byteSize_) {
    assert(buffer);
    MLN_TRACE_ALLOC_VERTEX_BUFFER(buffer.get(), byteSize)
}

VertexBufferResource::VertexBufferResource(AsyncAllocCallback asyncAllocCallback_,
                                           AsyncUpdateCallback asyncUpdateCallback_)
    : asyncAllocCallback(std::move(asyncAllocCallback_)),
      asyncUpdateCallback(std::move(asyncUpdateCallback_)) {
    assert(asyncAllocCallback);
}

VertexBufferResource::VertexBufferResource(UniqueBuffer&& buffer_,
                                           int byteSize_,
                                           AsyncUpdateCallback asyncUpdateCallback_)
    : buffer(std::make_unique<UniqueBuffer>(std::move(buffer_))),
      byteSize(byteSize_),
      asyncUpdateCallback(std::move(asyncUpdateCallback_)) {
    assert(buffer);
    assert(asyncUpdateCallback);
    MLN_TRACE_ALLOC_VERTEX_BUFFER(buffer.get(), byteSize)
}

VertexBufferResource::~VertexBufferResource() noexcept {
    // We expect the resource to own a buffer at destruction time
    assert(buffer);
    if (asyncUploadRequested) {
        // Resource created but never used
        wait();
    }
    // No pending async upload
    assert(asyncUploadRequested == false);
    assert(asyncUploadCommands.data.empty());

    if (!buffer) {
        return; // No buffer owned
    }

    auto& underlyingBuffer = *buffer;
    MLN_TRACE_FREE_VERTEX_BUFFER(underlyingBuffer.get())
    auto& stats = underlyingBuffer.get_deleter().context.renderingStats();
    stats.memVertexBuffers -= byteSize;
    assert(stats.memVertexBuffers >= 0);
}

const UniqueBuffer& VertexBufferResource::pickBuffer() const {
    // wait must be called first
    assert(asyncUploadRequested == false);
    assert(asyncUploadIssued == false);
    // VertexBufferResource must own a buffer
    assert(buffer);
    return *buffer;
}

const UniqueBuffer& VertexBufferResource::waitAndGetBuffer() {
    wait();
    return pickBuffer();
}

void VertexBufferResource::asyncAlloc(ResourceUploadThreadPool& threadPool,
                                      int size,
                                      gfx::BufferUsageType usage,
                                      const void* initialData) {
    assert(asyncAllocCallback);
    // VertexBufferResource must not own a buffer yet since we are allocating one
    assert(!buffer);
    // VertexBufferResource must not have a pending async upload
    assert(asyncUploadIssued == false);
    assert(asyncUploadRequested == false);
    asyncUploadRequested = true;

    auto& cmd = asyncUploadCommands;
    assert(cmd.data.empty());
    assert(cmd.type == BufferAsyncUploadCommandType::None);
    assert(byteSize == 0);
    byteSize = size;
    cmd.type = BufferAsyncUploadCommandType::Alloc;
    cmd.dataSize = size;
    cmd.usage = usage;
    cmd.data.resize((size + sizeof(uint64_t) - 1) / sizeof(uint64_t));
    std::memcpy(cmd.data.data(), initialData, size);

    threadPool.schedule([this] { issueAsyncUpload(); });
}

void VertexBufferResource::asyncUpdate(ResourceUploadThreadPool& threadPool, int size, const void* data) {
    assert(asyncUpdateCallback);
    // VertexBufferResource must own a buffer to update it
    assert(buffer);
    // VertexBufferResource must not have a pending async upload
    assert(asyncUploadIssued == false);
    assert(asyncUploadRequested == false);
    asyncUploadRequested = true;

    auto& cmd = asyncUploadCommands;
    assert(cmd.data.empty());
    assert(cmd.type == BufferAsyncUploadCommandType::None);
    assert(size <= byteSize);
    byteSize = size;
    cmd.type = BufferAsyncUploadCommandType::Update;
    cmd.dataSize = size;
    cmd.data.resize((size + sizeof(uint64_t) - 1) / sizeof(uint64_t));
    std::memcpy(cmd.data.data(), data, size);

    threadPool.schedule([this] { issueAsyncUpload(); });
}

void VertexBufferResource::wait() {
    std::unique_lock<std::mutex> lk(asyncUploadMutex);
    if (!asyncUploadRequested) {
        return;
    }
    if (!asyncUploadIssued) {
        asyncUploadConditionVar.wait(lk, [&] { return asyncUploadIssued; });
    }
    assert(asyncUploadCommands.data.empty());
    assert(asyncUploadCommands.type == BufferAsyncUploadCommandType::None);
#ifdef MLN_RENDER_BACKEND_USE_UPLOAD_GL_FENCE
    gpuFence.gpuWait();
    gpuFence.reset();
#endif
    asyncUploadIssued = false;
    asyncUploadRequested = false;
}

void VertexBufferResource::issueAsyncUpload() {
    const std::lock_guard<std::mutex> lk(asyncUploadMutex);

    assert(asyncUploadRequested == true);
    assert(asyncUploadIssued == false);

    auto& cmd = asyncUploadCommands;
    switch (cmd.type) {
        case BufferAsyncUploadCommandType::Alloc:
            buffer = std::make_unique<UniqueBuffer>(asyncAllocCallback(cmd.dataSize, cmd.usage, cmd.data.data()));
            break;
        case BufferAsyncUploadCommandType::Update:
            asyncUpdateCallback(*buffer, cmd.dataSize, cmd.data.data());
            break;
        default:
            assert(false);
            break;
    }
#ifdef MLN_RENDER_BACKEND_USE_UPLOAD_GL_FENCE
    gpuFence.insert();
#else
    MBGL_CHECK_ERROR(glFlush());
#endif

    cmd.type = BufferAsyncUploadCommandType::None;
    cmd.dataSize = 0;
    cmd.data.clear();
    asyncUploadIssued = true;

    asyncUploadConditionVar.notify_all();
}

} // namespace gl
} // namespace mbgl