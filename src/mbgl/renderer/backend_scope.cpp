#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/renderer_backend.hpp>
#include <mbgl/util/instrumentation.hpp>
#include <mbgl/util/thread_local.hpp>

#include <cassert>

namespace {

mbgl::util::ThreadLocal<mbgl::gfx::BackendScope>& currentScope() {
    static mbgl::util::ThreadLocal<mbgl::gfx::BackendScope> backendScope;

    return backendScope;
}

} // namespace

namespace mbgl {
namespace gfx {

BackendScope::BackendScope(RendererBackend& backend_, ScopeType scopeType_)
    : priorScope(currentScope().get()),
      nextScope(nullptr),
      backend(backend_),
      scopeType(scopeType_) {
    MLN_TRACE_FUNC()

    // Cannot change scope while uploading resources
    assert(backend.isFreeThreadedUploadActive() == false);

    if (priorScope) {
        assert(priorScope->nextScope == nullptr);
        priorScope->nextScope = this;
        priorScope->deactivate();
    }

    activate();

    currentScope().set(this);
}

BackendScope::~BackendScope() {
    MLN_TRACE_FUNC()

    // Cannot change scope while uploading resources
    assert(backend.isFreeThreadedUploadActive() == false);

    assert(nextScope == nullptr);
    deactivate();

    if (priorScope) {
        priorScope->activate();
        currentScope().set(priorScope);
        assert(priorScope->nextScope == this);
        priorScope->nextScope = nullptr;
    } else {
        currentScope().set(nullptr);
    }
}

void BackendScope::activate() {
    MLN_TRACE_FUNC()

    if (scopeType == ScopeType::Explicit && !(priorScope && this->backend == priorScope->backend) &&
        !(nextScope && this->backend == nextScope->backend)) {
        // Only activate when set to Explicit and
        // only once per RenderBackend
        backend.activate();
        activated = true;
    }
}

void BackendScope::deactivate() {
    if (activated && !(nextScope && this->backend == nextScope->backend)) {
        // Only deactivate when set to Explicit and
        // only once per RenderBackend
        backend.deactivate();
        activated = false;
    }
}

bool BackendScope::exists() {
    return currentScope().get();
}

FreeThreadedUploadBackendScope::FreeThreadedUploadBackendScope(RendererBackend& backend_)
    : backend(backend_) {
    MLN_TRACE_FUNC()

    assert(backend.isFreeThreadedUploadActive() == false);
    backend.beginFreeThreadedUpload();
    assert(backend.isFreeThreadedUploadActive() == true);
}

FreeThreadedUploadBackendScope::~FreeThreadedUploadBackendScope() {
    MLN_TRACE_FUNC()

    assert(backend.isFreeThreadedUploadActive() == true);
    backend.endFreeThreadedUpload();
    assert(backend.isFreeThreadedUploadActive() == false);
}

} // namespace gfx
} // namespace mbgl
