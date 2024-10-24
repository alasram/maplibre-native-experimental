#include <mbgl/shaders/mtl/custom_symbol_icon.hpp>
#include <mbgl/shaders/shader_defines.hpp>

namespace mbgl {
namespace shaders {

const std::array<UniformBlockInfo, 2>
    ShaderSource<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::Metal>::uniforms = {
        UniformBlockInfo{true, false, sizeof(CustomSymbolIconDrawableUBO), idCustomSymbolDrawableUBO},
        UniformBlockInfo{true, false, sizeof(CustomSymbolIconParametersUBO), idCustomSymbolParametersUBO},
};
const std::array<AttributeInfo, 2> ShaderSource<BuiltIn::CustomSymbolIconShader,
                                                gfx::Backend::Type::Metal>::attributes = {
    // always attributes
    AttributeInfo{customSymbolDrawableUBOCount + 0, gfx::AttributeDataType::Float2, idCustomSymbolPosVertexAttribute},
    AttributeInfo{customSymbolDrawableUBOCount + 1, gfx::AttributeDataType::Float2, idCustomSymbolTexVertexAttribute},
};
const std::array<TextureInfo, 1> ShaderSource<BuiltIn::CustomSymbolIconShader, gfx::Backend::Type::Metal>::textures = {
    TextureInfo{0, idCustomSymbolImageTexture},
};

} // namespace shaders
} // namespace mbgl
