#include <mbgl/gl/context.hpp>
#include <mbgl/gl/index_buffer_resource.hpp>
#include <mbgl/util/instrumentation.hpp>

#include <cassert>

namespace mbgl {
namespace gl {

IndexBufferResource::IndexBufferResource(UniqueBuffer&& buffer_, int byteSize_)
    : BufferResource(std::move(buffer_), byteSize_){MLN_TRACE_ALLOC_INDEX_BUFFER(buffer.get(), byteSize)}

      IndexBufferResource::IndexBufferResource(AsyncAllocCallback alloc, AsyncUpdateCallback update)
    : BufferResource(std::move(alloc), std::move(update)) {}

IndexBufferResource::IndexBufferResource(UniqueBuffer&& buffer_, int byteSize_, AsyncUpdateCallback update)
    : BufferResource(
          std::move(buffer_), byteSize_, std::move(update)){MLN_TRACE_ALLOC_INDEX_BUFFER(buffer.get(), byteSize)}

      IndexBufferResource::~IndexBufferResource() noexcept {
    auto& underlyingBuffer = *buffer;
    MLN_TRACE_FREE_INDEX_BUFFER(underlyingBuffer.get())
    auto& stats = underlyingBuffer.get_deleter().context.renderingStats();
    stats.memIndexBuffers -= byteSize;
    assert(stats.memIndexBuffers >= 0);
}

} // namespace gl
} // namespace mbgl