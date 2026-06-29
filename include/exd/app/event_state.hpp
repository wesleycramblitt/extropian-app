#pragma once

#include <SDL3/SDL.h>
#include <array>

namespace exd::app {

class EventState {
public:
    EventState() = default;
    EventState(SDL_Event* events, int num_events, const bool* keyboard_state,
               float mouse_rel_x, float mouse_rel_y);
    void set_state(SDL_Event* events, int num_events, const bool* keyboard_state,
                    float mouse_rel_x, float mouse_rel_y);

    /// True only on the frame that `sc` was released.
    bool was_key_released(SDL_Scancode sc) const {
        return sc < SDL_SCANCODE_COUNT && key_up_[sc];
    }

    const bool* keyboard_state = nullptr;
    SDL_Event* events = nullptr;
    int num_events = 0;
    float mouse_rel_x = 0;
    float mouse_rel_y = 0;

private:
    friend class Window;
    std::array<bool, SDL_SCANCODE_COUNT> key_up_ = {};
};

} // namespace exd::app
