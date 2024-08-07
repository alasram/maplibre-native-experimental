#pragma once

#include <mbgl/gfx/uniform_block.hpp>

namespace mbgl {
namespace vulkan {

class UniformBlock final : public gfx::UniformBlock {
public:
    UniformBlock(int index_, std::size_t size_)
        : gfx::UniformBlock(index_, size_) {}
    UniformBlock(const UniformBlock& other)
        : gfx::UniformBlock(other) {}
    UniformBlock(UniformBlock&& other)
        : gfx::UniformBlock(std::move(other)) {}
    ~UniformBlock() override = default;

    void bindBuffer(const gfx::UniformBuffer&) override {}
    void unbindBuffer() override {}

    bool getBindVertex() const { return bindVertex; }
    void setBindVertex(bool value) { bindVertex = value; }

    bool getBindFragment() const { return bindFragment; }
    void setBindFragment(bool value) { bindFragment = value; }

protected:
    bool bindVertex = false;
    bool bindFragment = false;
};

/// Stores a collection of uniform blocks by name
class UniformBlockArray final : public gfx::UniformBlockArray {
public:
    UniformBlockArray() = default;
    UniformBlockArray(UniformBlockArray&& other)
        : gfx::UniformBlockArray(std::move(other)) {}
    UniformBlockArray(const UniformBlockArray&) = delete;

    UniformBlockArray& operator=(UniformBlockArray&& other) {
        gfx::UniformBlockArray::operator=(std::move(other));
        return *this;
    }
    UniformBlockArray& operator=(const UniformBlockArray& other) {
        gfx::UniformBlockArray::operator=(other);
        return *this;
    }

private:
    std::unique_ptr<gfx::UniformBlock> create(int index, std::size_t size) override {
        return std::make_unique<UniformBlock>(index, size);
    }
    std::unique_ptr<gfx::UniformBlock> copy(const gfx::UniformBlock& uniformBlocks) override {
        return std::make_unique<UniformBlock>(static_cast<const UniformBlock&>(uniformBlocks));
    }
};

} // namespace vulkan
} // namespace mbgl
