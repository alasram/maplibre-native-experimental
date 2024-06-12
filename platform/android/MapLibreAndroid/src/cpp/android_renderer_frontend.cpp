#include "android_renderer_frontend.hpp"

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/renderer/renderer.hpp>
#include <mbgl/renderer/renderer_observer.hpp>
#include <mbgl/util/async_task.hpp>
#include <mbgl/util/thread.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/geojson.hpp>
#include <mbgl/util/instrumentation.hpp>

#include "android_renderer_backend.hpp"

namespace mbgl {
namespace android {

// Forwards RendererObserver signals to the given
// Delegate RendererObserver on the given RunLoop
class ForwardingRendererObserver : public RendererObserver {
public:
    ForwardingRendererObserver(util::RunLoop& mapRunLoop, RendererObserver& delegate_)
        : mailbox(std::make_shared<Mailbox>(mapRunLoop)),
          delegate(delegate_, mailbox) {}

    ~ForwardingRendererObserver() { mailbox->close(); }

    void onInvalidate() override {
        MLN_TRACE_FUNC();
        delegate.invoke(&RendererObserver::onInvalidate);
    }

    void onResourceError(std::exception_ptr err) override {
        MLN_TRACE_FUNC();
        delegate.invoke(&RendererObserver::onResourceError, err);
    }

    void onWillStartRenderingMap() override {
        MLN_TRACE_FUNC();
        delegate.invoke(&RendererObserver::onWillStartRenderingMap);
    }

    void onWillStartRenderingFrame() override {
        MLN_TRACE_FUNC();
        delegate.invoke(&RendererObserver::onWillStartRenderingFrame);
    }

    void onDidFinishRenderingFrame(RenderMode mode,
                                   bool repaintNeeded,
                                   bool placementChanged,
                                   double frameEncodingTime,
                                   double frameRenderingTime) override {
        MLN_TRACE_FUNC();
        void (RendererObserver::*f)(
            RenderMode, bool, bool, double, double) = &RendererObserver::onDidFinishRenderingFrame;
        delegate.invoke(f, mode, repaintNeeded, placementChanged, frameEncodingTime, frameRenderingTime);
    }

    void onDidFinishRenderingMap() override {
        MLN_TRACE_FUNC();
        delegate.invoke(&RendererObserver::onDidFinishRenderingMap);
    }

    void onStyleImageMissing(const std::string& id, const StyleImageMissingCallback& done) override {
        MLN_TRACE_FUNC();
        delegate.invoke(&RendererObserver::onStyleImageMissing, id, done);
    }

    void onRemoveUnusedStyleImages(const std::vector<std::string>& ids) override {
        MLN_TRACE_FUNC();
        delegate.invoke(&RendererObserver::onRemoveUnusedStyleImages, ids);
    }

private:
    std::shared_ptr<Mailbox> mailbox;
    ActorRef<RendererObserver> delegate;
};

AndroidRendererFrontend::AndroidRendererFrontend(MapRenderer& mapRenderer_)
    : mapRenderer(mapRenderer_),
      mapRunLoop(util::RunLoop::Get()),
      updateAsyncTask(std::make_unique<util::AsyncTask>([this]() {
          mapRenderer.update(std::move(updateParams));
          mapRenderer.requestRender();
      })) {}

AndroidRendererFrontend::~AndroidRendererFrontend() = default;

void AndroidRendererFrontend::reset() {
    MLN_TRACE_FUNC();
    mapRenderer.reset();
}

void AndroidRendererFrontend::setObserver(RendererObserver& observer) {
    MLN_TRACE_FUNC();
    assert(util::RunLoop::Get());
    // Don't call the Renderer directly, but use MapRenderer#setObserver to make sure
    // the Renderer may be re-initialised without losing the RendererObserver reference.
    mapRenderer.setObserver(std::make_unique<ForwardingRendererObserver>(*mapRunLoop, observer));
}

void AndroidRendererFrontend::update(std::shared_ptr<UpdateParameters> params) {
    MLN_TRACE_FUNC();
    updateParams = std::move(params);
    updateAsyncTask->send();
}

void AndroidRendererFrontend::reduceMemoryUse() {
    MLN_TRACE_FUNC();
    mapRenderer.actor().invoke(&Renderer::reduceMemoryUse);
}

std::vector<Feature> AndroidRendererFrontend::querySourceFeatures(const std::string& sourceID,
                                                                  const SourceQueryOptions& options) const {
    MLN_TRACE_FUNC();
    // Waits for the result from the orchestration thread and returns
    return mapRenderer.actor().ask(&Renderer::querySourceFeatures, sourceID, options).get();
}

std::vector<Feature> AndroidRendererFrontend::queryRenderedFeatures(const ScreenBox& box,
                                                                    const RenderedQueryOptions& options) const {
    MLN_TRACE_FUNC();
    // Select the right overloaded method
    std::vector<Feature> (Renderer::*fn)(const ScreenBox&, const RenderedQueryOptions&)
        const = &Renderer::queryRenderedFeatures;

    // Waits for the result from the orchestration thread and returns
    return mapRenderer.actor().ask(fn, box, options).get();
}

std::vector<Feature> AndroidRendererFrontend::queryRenderedFeatures(const ScreenCoordinate& point,
                                                                    const RenderedQueryOptions& options) const {
    MLN_TRACE_FUNC();
    // Select the right overloaded method
    std::vector<Feature> (Renderer::*fn)(const ScreenCoordinate&, const RenderedQueryOptions&)
        const = &Renderer::queryRenderedFeatures;

    // Waits for the result from the orchestration thread and returns
    return mapRenderer.actor().ask(fn, point, options).get();
}

AnnotationIDs AndroidRendererFrontend::queryPointAnnotations(const ScreenBox& box) const {
    MLN_TRACE_FUNC();
    // Waits for the result from the orchestration thread and returns
    return mapRenderer.actor().ask(&Renderer::queryPointAnnotations, box).get();
}

AnnotationIDs AndroidRendererFrontend::queryShapeAnnotations(const ScreenBox& box) const {
    MLN_TRACE_FUNC();
    // Waits for the result from the orchestration thread and returns
    return mapRenderer.actor().ask(&Renderer::queryShapeAnnotations, box).get();
}

FeatureExtensionValue AndroidRendererFrontend::queryFeatureExtensions(
    const std::string& sourceID,
    const Feature& feature,
    const std::string& extension,
    const std::string& extensionField,
    const std::optional<std::map<std::string, mbgl::Value>>& args) const {
    MLN_TRACE_FUNC();
    return mapRenderer.actor()
        .ask(&Renderer::queryFeatureExtensions, sourceID, feature, extension, extensionField, args)
        .get();
}

} // namespace android
} // namespace mbgl
