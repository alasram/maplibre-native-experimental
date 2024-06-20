#include <mbgl/renderer/renderer.hpp>

#include <mbgl/annotation/annotation_manager.hpp>
#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/layermanager/layer_manager.hpp>
#include <mbgl/renderer/renderer_impl.hpp>
#include <mbgl/renderer/render_static_data.hpp>
#include <mbgl/renderer/render_tree.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/util/instrumentation.hpp>

std::mutex& getSharedGlobalMutex();

namespace mbgl {

struct MemInfoLoggerRenderer {
    std::string msg;

    void print(bool begin) {
        std::mutex& mux = getSharedGlobalMutex();
        const std::lock_guard<std::mutex> lock(mux);

        std::string text = "######################################################## ";
        text += begin ? "BEGIN " : "END ";
        text += msg + " ";

        std::string cmd = "echo \"" + text + "\" >> /data/testdir/mem_info.txt";
        std::system(cmd.c_str());
        std::system("dumpsys meminfo com.rivian.rivianivinavigation >> /data/testdir/mem_info.txt");

        // Log::Error(Event::JNI, cmd);
    }

    MemInfoLoggerRenderer(std::string const& s)
        : msg(s) {
        print(true);
    }
    ~MemInfoLoggerRenderer() { print(false); }
};

Renderer::Renderer(gfx::RendererBackend& backend, float pixelRatio_, const std::optional<std::string>& localFontFamily_)
    : impl(std::make_unique<Impl>(backend, pixelRatio_, localFontFamily_)) {
    MemInfoLoggerRenderer logger("Renderer::Renderer");
}

Renderer::~Renderer() {
    gfx::BackendScope guard{impl->backend};
    impl.reset();
}

void Renderer::markContextLost() {
    MemInfoLoggerRenderer logger("Renderer::markContextLost");
    impl->orchestrator.markContextLost();
}

void Renderer::setObserver(RendererObserver* observer) {
    MemInfoLoggerRenderer logger("Renderer::setObserver");
    impl->setObserver(observer);
    impl->orchestrator.setObserver(observer);
}

void Renderer::render(const std::shared_ptr<UpdateParameters>& updateParameters) {
    MemInfoLoggerRenderer logger("Renderer::render");

    MLN_TRACE_FUNC();
    assert(updateParameters);
    if (auto renderTree = impl->orchestrator.createRenderTree(updateParameters)) {
        renderTree->prepare();
        impl->render(*renderTree, updateParameters);
    }
}

std::vector<Feature> Renderer::queryRenderedFeatures(const ScreenLineString& geometry,
                                                     const RenderedQueryOptions& options) const {
    MemInfoLoggerRenderer logger("Renderer::queryRenderedFeatures");
    return impl->orchestrator.queryRenderedFeatures(geometry, options);
}

std::vector<Feature> Renderer::queryRenderedFeatures(const ScreenCoordinate& point,
                                                     const RenderedQueryOptions& options) const {
    MemInfoLoggerRenderer logger("Renderer::queryRenderedFeatures");
    return impl->orchestrator.queryRenderedFeatures({point}, options);
}

std::vector<Feature> Renderer::queryRenderedFeatures(const ScreenBox& box, const RenderedQueryOptions& options) const {
    MemInfoLoggerRenderer logger("Renderer::queryRenderedFeatures");
    return impl->orchestrator.queryRenderedFeatures(
        {box.min, {box.max.x, box.min.y}, box.max, {box.min.x, box.max.y}, box.min}, options);
}

AnnotationIDs Renderer::queryPointAnnotations(const ScreenBox& box) const {
    MemInfoLoggerRenderer logger("Renderer::queryPointAnnotations");

    if (!LayerManager::annotationsEnabled) {
        return {};
    }
    RenderedQueryOptions options;
    options.layerIDs = {{AnnotationManager::PointLayerID}};
    auto features = queryRenderedFeatures(box, options);
    return getAnnotationIDs(features);
}

AnnotationIDs Renderer::queryShapeAnnotations(const ScreenBox& box) const {
    MemInfoLoggerRenderer logger("Renderer::queryShapeAnnotations");

    if (!LayerManager::annotationsEnabled) {
        return {};
    }
    auto features = impl->orchestrator.queryShapeAnnotations(
        {box.min, {box.max.x, box.min.y}, box.max, {box.min.x, box.max.y}, box.min});
    return getAnnotationIDs(features);
}

AnnotationIDs Renderer::getAnnotationIDs(const std::vector<Feature>& features) const {
    MemInfoLoggerRenderer logger("Renderer::getAnnotationIDs");

    if (!LayerManager::annotationsEnabled) {
        return {};
    }
    std::set<AnnotationID> set;
    for (auto& feature : features) {
        assert(feature.id.is<uint64_t>());
        assert(feature.id.get<uint64_t>() <= std::numeric_limits<AnnotationID>::max());
        set.insert(static_cast<AnnotationID>(feature.id.get<uint64_t>()));
    }
    AnnotationIDs ids;
    ids.reserve(set.size());
    std::move(set.begin(), set.end(), std::back_inserter(ids));
    return ids;
}

std::vector<Feature> Renderer::querySourceFeatures(const std::string& sourceID,
                                                   const SourceQueryOptions& options) const {
    return impl->orchestrator.querySourceFeatures(sourceID, options);
}

FeatureExtensionValue Renderer::queryFeatureExtensions(const std::string& sourceID,
                                                       const Feature& feature,
                                                       const std::string& extension,
                                                       const std::string& extensionField,
                                                       const std::optional<std::map<std::string, Value>>& args) const {
    return impl->orchestrator.queryFeatureExtensions(sourceID, feature, extension, extensionField, args);
}

void Renderer::setFeatureState(const std::string& sourceID,
                               const std::optional<std::string>& sourceLayerID,
                               const std::string& featureID,
                               const FeatureState& state) {
    impl->orchestrator.setFeatureState(sourceID, sourceLayerID, featureID, state);
}

void Renderer::getFeatureState(FeatureState& state,
                               const std::string& sourceID,
                               const std::optional<std::string>& sourceLayerID,
                               const std::string& featureID) const {
    impl->orchestrator.getFeatureState(state, sourceID, sourceLayerID, featureID);
}

void Renderer::removeFeatureState(const std::string& sourceID,
                                  const std::optional<std::string>& sourceLayerID,
                                  const std::optional<std::string>& featureID,
                                  const std::optional<std::string>& stateKey) {
    impl->orchestrator.removeFeatureState(sourceID, sourceLayerID, featureID, stateKey);
}

void Renderer::dumpDebugLogs() {
    impl->orchestrator.dumpDebugLogs();
}

void Renderer::collectPlacedSymbolData(bool enable) {
    impl->orchestrator.collectPlacedSymbolData(enable);
}

const std::vector<PlacedSymbolData>& Renderer::getPlacedSymbolsData() const {
    return impl->orchestrator.getPlacedSymbolsData();
}

void Renderer::reduceMemoryUse() {
    MemInfoLoggerRenderer logger("Renderer::reduceMemoryUse");

    gfx::BackendScope guard{impl->backend};
    impl->reduceMemoryUse();
    impl->orchestrator.reduceMemoryUse();
}

void Renderer::clearData() {
    impl->orchestrator.clearData();
}

} // namespace mbgl
