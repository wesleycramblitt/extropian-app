#pragma once

#include <SDL3/SDL.h>
#include <exd/app/event_state.hpp>
#include <exd/app/input_mode.hpp>
#include <vector>

namespace exd::app {

/// SDL3 window wrapper with OpenGL context and event polling.
class Window {
public:
    Window();
    ~Window();

    void die(const char* msg);
    void swap_buffers();
    void get_dimensions(int& width, int& height, float& aspect) const;
    void poll_events();

    InputMode input_mode() const { return input_mode_; }
    void set_input_mode(InputMode mode);

    bool should_close = false;
    bool wireframe = false;
    bool grid_visible = true;
    SDL_Window* sdl_window = nullptr;
    SDL_GLContext gl_context = nullptr;
    EventState event_state;
    std::vector<SDL_Event> event_buffer;

private:
    InputMode input_mode_ = InputMode::FPS;
};

} // namespace exd::app
