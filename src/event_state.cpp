#include <exd/app/event_state.hpp>

namespace exd::app {

EventState::EventState(SDL_Event* events, int num_events, const bool* keyboard_state,
                        float mouse_rel_x, float mouse_rel_y)
    : keyboard_state(keyboard_state), num_events(num_events), events(events),
      mouse_rel_x(mouse_rel_x), mouse_rel_y(mouse_rel_y) {}

void EventState::set_state(SDL_Event* _events, int _num_events,
                            const bool* _keyboard_state,
                            float _mouse_rel_x, float _mouse_rel_y) {
    events = _events;
    num_events = _num_events;
    keyboard_state = _keyboard_state;
    mouse_rel_x = _mouse_rel_x;
    mouse_rel_y = _mouse_rel_y;
}

} // namespace exd::app
