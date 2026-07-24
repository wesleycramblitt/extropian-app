#include <exd/app/event_state.hpp>

namespace exd::app {

void EventState::begin_frame() {
    key_up_.fill(false);
    mouse_buttons_.fill(false);
    // Public fields are intentionally NOT reset here —
    // they're set explicitly by Window::poll_events().
}

} // namespace exd::app
