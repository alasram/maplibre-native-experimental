#include <mbgl/gfx/attribute.hpp>
#include <mbgl/gfx/index_vector.hpp>
#include <mbgl/gfx/vertex_vector.hpp>
#include <mbgl/programs/segment.hpp>
#include <mbgl/util/color.hpp>

#include <cstdint>

namespace mbgl {
namespace gfx {

struct DrawableBuilder::Impl {
    using VT = gfx::detail::VertexType<gfx::AttributeType<std::int16_t,2>>;
    gfx::VertexVector<VT> vertices;
    gfx::IndexVector<gfx::Triangles> indexes;
    SegmentVector<TypeList<void>> segments;
    Color currentColor = Color::white();
    std::vector<Color> colors;
};

} // namespace gfx
} // namespace mbgl
