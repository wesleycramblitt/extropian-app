#include <exd/app/window.hpp>
#include <exd/core/macros.hpp>
#include <glad/gl.h>
#include <iostream>
#include <cstdlib>

namespace exd::app {

Window::Window() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    sdl_window = SDL_CreateWindow("Extropian Simulator", 1280, 720,
                                   SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!sdl_window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }

    gl_context = SDL_GL_CreateContext(sdl_window);
    if (!gl_context) {
        std::cerr << "OpenGL context could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }
    SDL_GL_MakeCurrent(sdl_window, gl_context);

    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL loader!" << std::endl;
        SDL_GL_DestroyContext(gl_context);
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        return;
    }

    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glEnable(GL_CULL_FACE));
    GL_CALL(glCullFace(GL_BACK));
    GL_CALL(glViewport(0, 0, 1280, 720));

    SDL_SetWindowRelativeMouseMode(sdl_window, true);
    set_input_mode(InputMode::FPS);
}

Window::~Window() {
    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}

void Window::die(const char* msg) {
    const char* err = SDL_GetError();
    SDL_Log("FATAL: %s | SDL error: %s", msg, (err && *err) ? err : "(none)");
    std::exit(1);
}

void Window::swap_buffers() {
    SDL_GL_SwapWindow(sdl_window);
    glClearColor(0.2f, 0.2f, 0.2f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

void Window::get_dimensions(int& width, int& height, float& aspect) const {
    SDL_GetWindowSize(sdl_window, &width, &height);
    aspect = (height > 0) ? (float)width / (float)height : 1.0f;
}

void Window::poll_events() {
    SDL_PumpEvents();
    event_buffer.clear();
    event_state.key_up_.fill(false);

    SDL_Event e;
    while (SDL_PollEvent(&e)) event_buffer.push_back(e);

    const bool* keys = SDL_GetKeyboardState(nullptr);
    float mouse_rel_x = event_state.mouse_rel_x;
    float mouse_rel_y = event_state.mouse_rel_y;

    for (const auto& ev : event_buffer) {
        if (ev.type == SDL_EVENT_QUIT) should_close = true;
        if (ev.type == SDL_EVENT_KEY_DOWN && ev.key.scancode == SDL_SCANCODE_ESCAPE) {
            if (input_mode_ == InputMode::FPS) should_close = true;
        }
        if (ev.type == SDL_EVENT_KEY_DOWN && ev.key.scancode == SDL_SCANCODE_Z) {
            set_input_mode(input_mode_ == InputMode::FPS ? InputMode::UI : InputMode::FPS);
        }
        if (ev.type == SDL_EVENT_KEY_DOWN && ev.key.scancode == SDL_SCANCODE_G) {
            grid_visible = !grid_visible;
        }
        if (ev.type == SDL_EVENT_KEY_UP) {
            if (ev.key.scancode < SDL_SCANCODE_COUNT)
                event_state.key_up_[ev.key.scancode] = true;
        }
        if (ev.type == SDL_EVENT_MOUSE_MOTION) {
            mouse_rel_x += (float)ev.motion.xrel;
            mouse_rel_y += (float)ev.motion.yrel;
        }
        if (ev.type == SDL_EVENT_WINDOW_RESIZED) {
            glViewport(0, 0, ev.window.data1, ev.window.data2);
        }
    }

    event_state.set_state(event_buffer.data(), static_cast<int>(event_buffer.size()),
                           keys, mouse_rel_x, mouse_rel_y);
}

void Window::set_input_mode(InputMode mode) {
    input_mode_ = mode;
    SDL_SetWindowRelativeMouseMode(sdl_window, mode == InputMode::FPS);
}

} // namespace exd::app
