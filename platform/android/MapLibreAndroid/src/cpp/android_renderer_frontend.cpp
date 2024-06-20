#include "android_renderer_frontend.hpp"

#include <mbgl/actor/scheduler.hpp>
#include <mbgl/renderer/renderer.hpp>
#include <mbgl/renderer/renderer_observer.hpp>
#include <mbgl/util/async_task.hpp>
#include <mbgl/util/thread.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/geojson.hpp>

#include "android_renderer_backend.hpp"

#include <mutex>
#include <thread>

std::mutex& getSharedGlobalMutex() {
    static std::mutex mux;
    return mux;
}

std::mutex& getSharedGlobalMutex();

namespace mbgl {
namespace android {

struct MemInfoLogger3 {
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

    MemInfoLogger3(std::string const& s)
        : msg(s) {
        print(true);
    }
    ~MemInfoLogger3() { print(false); }
};

// Forwards RendererObserver signals to the given
// Delegate RendererObserver on the given RunLoop
class ForwardingRendererObserver : public RendererObserver {
public:
    ForwardingRendererObserver(util::RunLoop& mapRunLoop, RendererObserver& delegate_)
        : mailbox(std::make_shared<Mailbox>(mapRunLoop)),
          delegate(delegate_, mailbox) {}

    ~ForwardingRendererObserver() { mailbox->close(); }

    void onInvalidate() override { delegate.invoke(&RendererObserver::onInvalidate); }

    void onResourceError(std::exception_ptr err) override { delegate.invoke(&RendererObserver::onResourceError, err); }

    void onWillStartRenderingMap() override { delegate.invoke(&RendererObserver::onWillStartRenderingMap); }

    void onWillStartRenderingFrame() override { delegate.invoke(&RendererObserver::onWillStartRenderingFrame); }

    void onDidFinishRenderingFrame(RenderMode mode,
                                   bool repaintNeeded,
                                   bool placementChanged,
                                   double frameEncodingTime,
                                   double frameRenderingTime) override {
        void (RendererObserver::*f)(
            RenderMode, bool, bool, double, double) = &RendererObserver::onDidFinishRenderingFrame;
        delegate.invoke(f, mode, repaintNeeded, placementChanged, frameEncodingTime, frameRenderingTime);
    }

    void onDidFinishRenderingMap() override { delegate.invoke(&RendererObserver::onDidFinishRenderingMap); }

    void onStyleImageMissing(const std::string& id, const StyleImageMissingCallback& done) override {
        delegate.invoke(&RendererObserver::onStyleImageMissing, id, done);
    }

    void onRemoveUnusedStyleImages(const std::vector<std::string>& ids) override {
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
          mapRenderer.updateX(std::move(updateParams));
          mapRenderer.requestRender();
      })) {
    MemInfoLogger3 logger("AndroidRendererFrontend::AndroidRendererFrontend");
}

AndroidRendererFrontend::~AndroidRendererFrontend() = default;

void AndroidRendererFrontend::reset() {
    MemInfoLogger3 logger("AndroidRendererFrontend::reset");
    mapRenderer.reset();
}

void AndroidRendererFrontend::setObserver(RendererObserver& observer) {
    MemInfoLogger3 logger("AndroidRendererFrontend::setObserver");
    assert(util::RunLoop::Get());
    // Don't call the Renderer directly, but use MapRenderer#setObserver to make sure
    // the Renderer may be re-initialised without losing the RendererObserver reference.
    mapRenderer.setObserver(std::make_unique<ForwardingRendererObserver>(*mapRunLoop, observer));
}

void AndroidRendererFrontend::update(std::shared_ptr<UpdateParameters> params) {
    MemInfoLogger3 logger("AndroidRendererFrontend::updateY");
    updateParams = std::move(params);
    updateAsyncTask->send();
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void AndroidRendererFrontend::reduceMemoryUse() {
    MemInfoLogger3 logger("AndroidRendererFrontend::reduceMemoryUse");
    mapRenderer.actor().invoke(&Renderer::reduceMemoryUse);
}

std::vector<Feature> AndroidRendererFrontend::querySourceFeatures(const std::string& sourceID,
                                                                  const SourceQueryOptions& options) const {
    MemInfoLogger3 logger("AndroidRendererFrontend::querySourceFeatures");
    // Waits for the result from the orchestration thread and returns
    return mapRenderer.actor().ask(&Renderer::querySourceFeatures, sourceID, options).get();
}

std::vector<Feature> AndroidRendererFrontend::queryRenderedFeatures(const ScreenBox& box,
                                                                    const RenderedQueryOptions& options) const {
    MemInfoLogger3 logger("AndroidRendererFrontend::queryRenderedFeatures");
    // Select the right overloaded method
    std::vector<Feature> (Renderer::*fn)(const ScreenBox&, const RenderedQueryOptions&)
        const = &Renderer::queryRenderedFeatures;

    // Waits for the result from the orchestration thread and returns
    return mapRenderer.actor().ask(fn, box, options).get();
}

std::vector<Feature> AndroidRendererFrontend::queryRenderedFeatures(const ScreenCoordinate& point,
                                                                    const RenderedQueryOptions& options) const {
    MemInfoLogger3 logger("AndroidRendererFrontend::queryRenderedFeatures");
    // Select the right overloaded method
    std::vector<Feature> (Renderer::*fn)(const ScreenCoordinate&, const RenderedQueryOptions&)
        const = &Renderer::queryRenderedFeatures;

    // Waits for the result from the orchestration thread and returns
    return mapRenderer.actor().ask(fn, point, options).get();
}

AnnotationIDs AndroidRendererFrontend::queryPointAnnotations(const ScreenBox& box) const {
    MemInfoLogger3 logger("AndroidRendererFrontend::queryPointAnnotations");
    // Waits for the result from the orchestration thread and returns
    return mapRenderer.actor().ask(&Renderer::queryPointAnnotations, box).get();
}

AnnotationIDs AndroidRendererFrontend::queryShapeAnnotations(const ScreenBox& box) const {
    MemInfoLogger3 logger("AndroidRendererFrontend::queryShapeAnnotations");
    // Waits for the result from the orchestration thread and returns
    return mapRenderer.actor().ask(&Renderer::queryShapeAnnotations, box).get();
}

FeatureExtensionValue AndroidRendererFrontend::queryFeatureExtensions(
    const std::string& sourceID,
    const Feature& feature,
    const std::string& extension,
    const std::string& extensionField,
    const std::optional<std::map<std::string, mbgl::Value>>& args) const {
    MemInfoLogger3 logger("AndroidRendererFrontend::queryFeatureExtensions");
    return mapRenderer.actor()
        .ask(&Renderer::queryFeatureExtensions, sourceID, feature, extension, extensionField, args)
        .get();
}

} // namespace android
} // namespace mbgl
