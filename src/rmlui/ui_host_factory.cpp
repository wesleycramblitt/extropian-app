// IUIHost factory implementation — creates the appropriate backend.

#include <exd/app/ui_host.hpp>
#include "rmlui/host.hpp"
#include <memory>
#include <cstdio>

namespace exd::app {

std::unique_ptr<IUIHost> IUIHost::create(Backend backend, void* window_handle) {
    switch (backend) {
        case Backend::RmlUi:
            return std::make_unique<rmlui::RmlUiHost>(window_handle);

        case Backend::Null:
            // Null backend — no UI, headless mode
            return nullptr;

        case Backend::ImGui:
        case Backend::Qt6:
            std::fprintf(stderr, "[IUIHost] Backend not yet implemented, falling back to RmlUi\n");
            return std::make_unique<rmlui::RmlUiHost>(window_handle);
    }
    return nullptr;
}

} // namespace exd::app
