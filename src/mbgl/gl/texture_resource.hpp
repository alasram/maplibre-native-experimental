#pragma once

#include <mbgl/gfx/texture.hpp>
#include <mbgl/gl/object.hpp>

namespace mbgl {
namespace gl {

class TextureResource : public gfx::TextureResource {
public:
    TextureResource(UniqueTexture&& texture_, int byteSize_);
    ~TextureResource() noexcept override;

    static int getStorageSize(const Size& size, gfx::TexturePixelType format, gfx::TextureChannelDataType type);

    UniqueTexture texture;
    gfx::TextureFilterType filter = gfx::TextureFilterType::Nearest;
    gfx::TextureMipMapType mipmap = gfx::TextureMipMapType::No;
    gfx::TextureWrapType wrapX = gfx::TextureWrapType::Clamp;
    gfx::TextureWrapType wrapY = gfx::TextureWrapType::Clamp;
    int byteSize;
};

} // namespace gl
} // namespace mbgl
