// Reusable 3D Viewport component — C++ side.
// Manages viewport size and provides a render target handle for the 3D renderer.

#include <exd/app/ui_host.hpp>
#include <string>

namespace exd::app::rmlui {

struct ViewportState {
    int x = 0, y = 0;
    int width = 1280, height = 720;
    bool has_focus = false;
    bool is_hovered = false;
};

inline void update_viewport_state(Document& doc, ViewportState& state) {
    // Query viewport element position/size from RmlUi
    // and forward to the 3D renderer for camera aspect ratio.
    doc.set_attribute("main-viewport", "data-width", std::to_string(state.width));
    doc.set_attribute("main-viewport", "data-height", std::to_string(state.height));
}

} // namespace exd::app::rmlui
