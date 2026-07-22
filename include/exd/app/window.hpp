#pragma once

#include <SDL3/SDL.h>
#include <exd/app/window_state.hpp>
#include <exd/app/event_state.hpp>
#include <exd/app/input_mode.hpp>
#include <vector>

namespace exd::app {

/// SDL3 window wrapper with OpenGL context and event polling.
///
/// Inherits from WindowState to expose dimension / input / toggle
/// values to the renderer.
class Window : public WindowState {
public:
    Window();
    ~Window();

    void die(const char* msg);
    void swap_buffers();
    void get_dimensions(int& width, int& height, float& aspect) const override;
    bool was_key_released(int scancode) const override;
    void poll_events();

    void set_input_mode(InputMode mode);

    bool should_close = false;
    SDL_Window* sdl_window = nullptr;
    SDL_GLContext gl_context = nullptr;
    EventState event_state;
    std::vector<SDL_Event> event_buffer;

    // grid_visible, wireframe, input_mode, keyboard_state,
    // mouse_rel_x/y are inherited from WindowState.
};

} // namespace exd::app
