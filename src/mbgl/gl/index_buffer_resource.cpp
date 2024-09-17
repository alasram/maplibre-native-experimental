#include <mbgl/gl/context.hpp>
#include <mbgl/gl/index_buffer_resource.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace gl {

IndexBufferResource::IndexBufferResource(UniqueBuffer&& buffer_, int byteSize_)
    : buffer(std::move(buffer_)),
      byteSize(byteSize_) {
    MLN_TRACE_ALLOC_INDEX_BUFFER(buffer.get(), byteSize);
}

IndexBufferResource::~IndexBufferResource() noexcept {
    MLN_TRACE_FREE_INDEX_BUFFER(buffer.get());
    auto& stats = buffer.get_deleter().context.renderingStats();
    stats.memIndexBuffers -= byteSize;
    assert(stats.memIndexBuffers >= 0);

    static int numDestroyedBuffers = 0;
    static int numTouchedBuffers = 0;
    ++numDestroyedBuffers;

    if (!drawTouched) {
        Log::Error(
            Event::OpenGL,
            std::string(
                "################### Calling ~IndexBufferResource on a resource that is never touched by a draw. ") +
                std::to_string(numDestroyedBuffers) + " total destroyed index buffers, " +
                std::to_string(numTouchedBuffers) + " total destroyed and touched by a draw.");
    } else {
        ++numTouchedBuffers;
    }
}

} // namespace gl
} // namespace mbgl