#include <mbgl/style/layer_impl.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace style {

Layer::Impl::Impl(std::string layerID, std::string sourceID)
    : id(std::move(layerID)),
      source(std::move(sourceID)) {
    static size_t id_len = 0;
    if (layerID.size() >= id_len) {
        id_len = layerID.size();
        Log::Error(Event::General,
                   "################################################################   " + layerID + "  (" +
                       std::to_string(id_len) + ")");
    }
}

void Layer::Impl::populateFontStack(std::set<FontStack>&) const {}

} // namespace style
} // namespace mbgl
