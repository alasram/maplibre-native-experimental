#include <mbgl/layermanager/layer_manager.hpp>
#include <mbgl/map/map_impl.hpp>
#include <mbgl/renderer/update_parameters.hpp>
#include <mbgl/storage/file_source.hpp>
#include <mbgl/style/style_impl.hpp>
#include <mbgl/util/exception.hpp>
#include <mbgl/util/logging.hpp>
#include <mbgl/util/traits.hpp>

std::mutex& getSharedGlobalMutex();

namespace mbgl {

struct MemInfoLogger4 {
    std::string msg;

    std::string toMB(size_t x) { return std::to_string(x / 1024 / 1024); }

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

    MemInfoLogger4(std::string const& s)
        : msg(s) {
        print(true);
    }
    ~MemInfoLogger4() { print(false); }
};

#if !defined(NDEBUG)
namespace {
void logStyleDependencies(EventSeverity severity, Event event, const style::Style& style) {
    using Dependency = style::expression::Dependency;
    constexpr auto maskCount = underlying_type(Dependency::MaskCount);
    std::array<std::size_t, maskCount + 1> counts = {0};
    const auto layers = style.getLayers();
    for (const auto& layer : layers) {
        const auto deps = layer->getDependencies();
        if (deps == Dependency::None) {
            counts[0]++;
        } else {
            for (size_t i = 0; i < maskCount; ++i) {
                if (deps & Dependency{1u << i}) {
                    counts[i + 1]++;
                }
            }
        }
    }
    std::ostringstream ss;
    ss << "Style '" << style.getName() << "' has " << layers.size() << " layers:\n";
    ss << "  " << Dependency::None << ": " << counts[0] << "\n";
    for (size_t i = 0; i < maskCount; ++i) {
        if (counts[i + 1]) {
            ss << "  " << Dependency{1u << i} << ": " << counts[i + 1] << "\n";
        }
    }
    Log::Record(severity, event, ss.str());
}
} // namespace
#endif

Map::Impl::Impl(RendererFrontend& frontend_,
                MapObserver& observer_,
                std::shared_ptr<FileSource> fileSource_,
                const MapOptions& mapOptions)
    : observer(observer_),
      rendererFrontend(frontend_),
      transform(observer, mapOptions.constrainMode(), mapOptions.viewportMode()),
      mode(mapOptions.mapMode()),
      pixelRatio(mapOptions.pixelRatio()),
      crossSourceCollisions(mapOptions.crossSourceCollisions()),
      fileSource(std::move(fileSource_)),
      style(std::make_unique<style::Style>(fileSource, pixelRatio)),
      annotationManager(*style) {
    MemInfoLogger4 logger("Map::Impl::Impl");
    transform.setNorthOrientation(mapOptions.northOrientation());
    style->impl->setObserver(this);
    rendererFrontend.setObserver(*this);
    transform.resize(mapOptions.size());
}

Map::Impl::~Impl() {
    MemInfoLogger4 logger("Map::Impl::~Impl");
    // Explicitly reset the RendererFrontend first to ensure it releases
    // All shared resources (AnnotationManager)
    rendererFrontend.reset();
};

// MARK: - Map::Impl StyleObserver

void Map::Impl::onSourceChanged(style::Source& source) {
    MemInfoLogger4 logger("Map::Impl::onSourceChanged");
    observer.onSourceChanged(source);
}

void Map::Impl::onUpdate() {
    MemInfoLogger4 logger("Map::Impl::onUpdate");
    // Don't load/render anything in still mode until explicitly requested.
    if (mode != MapMode::Continuous && !stillImageRequest) {
        return;
    }

    TimePoint timePoint = mode == MapMode::Continuous ? Clock::now() : Clock::time_point::max();

    transform.updateTransitions(timePoint);

    UpdateParameters params = {style->impl->isLoaded(),
                               mode,
                               pixelRatio,
                               debugOptions,
                               timePoint,
                               transform.getState(),
                               style->impl->getGlyphURL(),
                               style->impl->areSpritesLoaded(),
                               style->impl->getTransitionOptions(),
                               style->impl->getLight()->impl,
                               style->impl->getImageImpls(),
                               style->impl->getSourceImpls(),
                               style->impl->getLayerImpls(),
                               annotationManager.makeWeakPtr(),
                               fileSource,
                               prefetchZoomDelta,
                               bool(stillImageRequest),
                               crossSourceCollisions};

    rendererFrontend.update(std::make_shared<UpdateParameters>(std::move(params)));
}

void Map::Impl::onStyleLoading() {
    MemInfoLogger4 logger("Map::Impl::onStyleLoading");
    loading = true;
    rendererFullyLoaded = false;
    observer.onWillStartLoadingMap();
}

void Map::Impl::onStyleLoaded() {
    MemInfoLogger4 logger("Map::Impl::onStyleLoaded");
    if (!cameraMutated) {
        jumpTo(style->getDefaultCamera());
    }
    if (LayerManager::annotationsEnabled) {
        annotationManager.onStyleLoaded();
    }
    observer.onDidFinishLoadingStyle();

#if !defined(NDEBUG)
    logStyleDependencies(EventSeverity::Info, Event::Style, *style);
#endif
}

void Map::Impl::onStyleError(std::exception_ptr error) {
    MemInfoLogger4 logger("Map::Impl::onStyleError");
    MapLoadError type;
    std::string description;

    try {
        std::rethrow_exception(error);
    } catch (const mbgl::util::StyleParseException& e) {
        type = MapLoadError::StyleParseError;
        description = e.what();
    } catch (const mbgl::util::StyleLoadException& e) {
        type = MapLoadError::StyleLoadError;
        description = e.what();
    } catch (const mbgl::util::NotFoundException& e) {
        type = MapLoadError::NotFoundError;
        description = e.what();
    } catch (const std::exception& e) {
        type = MapLoadError::UnknownError;
        description = e.what();
    }

    observer.onDidFailLoadingMap(type, description);
}

// MARK: - Map::Impl RendererObserver

void Map::Impl::onInvalidate() {
    onUpdate();
}

void Map::Impl::onResourceError(std::exception_ptr error) {
    if (mode != MapMode::Continuous && stillImageRequest) {
        auto request = std::move(stillImageRequest);
        request->callback(error);
    }
}

void Map::Impl::onWillStartRenderingFrame() {
    MemInfoLogger4 logger("Map::Impl::onWillStartRenderingFrame");
    if (mode == MapMode::Continuous) {
        observer.onWillStartRenderingFrame();
    }
}

void Map::Impl::onDidFinishRenderingFrame(RenderMode renderMode,
                                          bool needsRepaint,
                                          bool placemenChanged,
                                          double frameEncodingTime,
                                          double frameRenderingTime) {
    MemInfoLogger4 logger("Map::Impl::onDidFinishRenderingFrame");
    rendererFullyLoaded = renderMode == RenderMode::Full;

    if (mode == MapMode::Continuous) {
        observer.onDidFinishRenderingFrame({MapObserver::RenderMode(renderMode),
                                            needsRepaint,
                                            placemenChanged,
                                            frameEncodingTime,
                                            frameRenderingTime});

        if (needsRepaint || transform.inTransition()) {
            onUpdate();
        } else if (rendererFullyLoaded) {
            observer.onDidBecomeIdle();
        }
    } else if (stillImageRequest && rendererFullyLoaded) {
        auto request = std::move(stillImageRequest);
        request->callback(nullptr);
    }
}

void Map::Impl::onWillStartRenderingMap() {
    MemInfoLogger4 logger("Map::Impl::onWillStartRenderingMap");
    if (mode == MapMode::Continuous) {
        observer.onWillStartRenderingMap();
    }
}

void Map::Impl::onDidFinishRenderingMap() {
    MemInfoLogger4 logger("Map::Impl::onDidFinishRenderingMap");
    if (mode == MapMode::Continuous && loading) {
        observer.onDidFinishRenderingMap(MapObserver::RenderMode::Full);
        if (loading) {
            loading = false;
            observer.onDidFinishLoadingMap();
        }
    }
};

void Map::Impl::jumpTo(const CameraOptions& camera) {
    MemInfoLogger4 logger("Map::Impl::jumpTo");
    cameraMutated = true;
    transform.jumpTo(camera);
    onUpdate();
}

void Map::Impl::onStyleImageMissing(const std::string& id, const std::function<void()>& done) {
    MemInfoLogger4 logger("Map::Impl::onStyleImageMissing");
    if (!style->getImage(id)) observer.onStyleImageMissing(id);

    done();
    onUpdate();
}

void Map::Impl::onRemoveUnusedStyleImages(const std::vector<std::string>& unusedImageIDs) {
    MemInfoLogger4 logger("Map::Impl::onRemoveUnusedStyleImages");
    for (const auto& unusedImageID : unusedImageIDs) {
        if (observer.onCanRemoveUnusedStyleImage(unusedImageID)) {
            style->removeImage(unusedImageID);
        }
    }
}

void Map::Impl::onRegisterShaders(gfx::ShaderRegistry& registry) {
    MemInfoLogger4 logger("Map::Impl::onRegisterShaders");
    observer.onRegisterShaders(registry);
}

} // namespace mbgl
