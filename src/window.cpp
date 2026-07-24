#include <exd/app/window.hpp>
#include <exd/core/macros.hpp>
#include <exd/core/logging.hpp>
#include <glad/gl.h>

namespace exd::app {

// ── EventState frame lifecycle ────────────────────────────────

void EventState::begin_frame() {
    key_down_.fill(false);
    key_up_.fill(false);
    mouse_buttons_.fill(false);
    scroll_x = 0.0f;
    scroll_y = 0.0f;
    // Other public fields (keyboard_state, events, mouse_rel_x/y,
    // gamepad state) are set explicitly by poll_events() and
    // persist across frames by design.
}

// ── Window construction ───────────────────────────────────────

Window::Window(const WindowDesc& desc)
    : desc_(desc)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD) < 0) {
        exd::core::log_error("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return;
    }

    apply_desc(desc);

    // ── Create window ──
    SDL_WindowFlags flags = SDL_WINDOW_OPENGL;
    if (desc.resizable)  flags |= SDL_WINDOW_RESIZABLE;
    if (desc.fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

    sdl_window_ = SDL_CreateWindow(desc.title.c_str(),
                                   desc.width, desc.height, flags);
    if (!sdl_window_) {
        exd::core::log_error("Window could not be created! SDL_Error: %s", SDL_GetError());
        return;
    }

    // ── Create GL context ──
    gl_context_ = SDL_GL_CreateContext(sdl_window_);
    if (!gl_context_) {
        exd::core::log_error("OpenGL context could not be created! SDL_Error: %s", SDL_GetError());
        SDL_DestroyWindow(sdl_window_);
        sdl_window_ = nullptr;
        return;
    }
    SDL_GL_MakeCurrent(sdl_window_, gl_context_);

    // ── Load GL ──
    if (!gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress)) {
        exd::core::log_error("Failed to initialize OpenGL loader!");
        SDL_GL_DestroyContext(gl_context_);
        SDL_DestroyWindow(sdl_window_);
        SDL_Quit();
        gl_context_ = nullptr;
        sdl_window_ = nullptr;
        return;
    }

    // ── Default GL state ──
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glEnable(GL_CULL_FACE));
    GL_CALL(glCullFace(GL_BACK));
    GL_CALL(glViewport(0, 0, desc.width, desc.height));

    // ── VSync ──
    SDL_GL_SetSwapInterval(desc.vsync ? 1 : 0);

    // ── Initial input mode ──
    set_input_mode(InputMode::FPS);
}

Window::~Window() {
    SDL_GL_DestroyContext(gl_context_);
    SDL_DestroyWindow(sdl_window_);
    SDL_Quit();
}

void Window::apply_desc(const WindowDesc& desc) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, desc.gl_major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, desc.gl_minor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    if (desc.msaa_samples > 0) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, desc.msaa_samples);
    }
}

// ── Per-frame ──────────────────────────────────────────────────

void Window::swap_buffers() {
    SDL_GL_SwapWindow(sdl_window_);
    // Note: glClear is the renderer's responsibility.
    // We only swap buffers here.
}

void Window::get_dimensions(int& width, int& height, float& aspect) const {
    SDL_GetWindowSize(sdl_window_, &width, &height);
    aspect = (height > 0) ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
}

void Window::set_title(const std::string& title) {
    if (sdl_window_) SDL_SetWindowTitle(sdl_window_, title.c_str());
}

void Window::set_icon(SDL_Surface* surface) {
    if (sdl_window_ && surface) SDL_SetWindowIcon(sdl_window_, surface);
}

// ── Event polling ──────────────────────────────────────────────

void Window::poll_events() {
    SDL_PumpEvents();
    event_state.begin_frame();
    event_buffer_.clear();

    SDL_Event e;
    while (SDL_PollEvent(&e)) event_buffer_.push_back(e);

    const bool* keys = SDL_GetKeyboardState(nullptr);

    // ── Process events ──
    for (const auto& ev : event_buffer_) {
        switch (ev.type) {
        case SDL_EVENT_QUIT:
            should_close_ = true;
            if (desc_.callbacks.on_close) desc_.callbacks.on_close();
            break;

        case SDL_EVENT_KEY_DOWN:
            event_state.set_key_down(ev.key.scancode);
            if (ev.key.scancode == SDL_SCANCODE_ESCAPE && input_mode == InputMode::FPS) {
                should_close_ = true;
                if (desc_.callbacks.on_close) desc_.callbacks.on_close();
            }
            // Hard-coded toggles (convenience — app can also do its own)
            if (ev.key.scancode == SDL_SCANCODE_Z)
                set_input_mode(input_mode == InputMode::FPS ? InputMode::UI : InputMode::FPS);
            if (ev.key.scancode == SDL_SCANCODE_G)
                grid_visible = !grid_visible;
            if (ev.key.scancode == SDL_SCANCODE_X)
                wireframe = !wireframe;
            break;

        case SDL_EVENT_KEY_UP:
            event_state.set_key_up(ev.key.scancode);
            break;

        case SDL_EVENT_MOUSE_MOTION:
            event_state.mouse_rel_x += static_cast<float>(ev.motion.xrel);
            event_state.mouse_rel_y += static_cast<float>(ev.motion.yrel);
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            event_state.set_mouse_button(ev.button.button, true);
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            event_state.set_mouse_button(ev.button.button, false);
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            event_state.scroll_x += ev.wheel.x;
            event_state.scroll_y += ev.wheel.y;
            break;

        case SDL_EVENT_WINDOW_RESIZED:
            glViewport(0, 0, ev.window.data1, ev.window.data2);
            if (desc_.callbacks.on_resize)
                desc_.callbacks.on_resize(ev.window.data1, ev.window.data2);
            break;

        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            if (desc_.callbacks.on_focus) desc_.callbacks.on_focus(true);
            break;

        case SDL_EVENT_WINDOW_FOCUS_LOST:
            if (desc_.callbacks.on_focus) desc_.callbacks.on_focus(false);
            break;

        case SDL_EVENT_WINDOW_MINIMIZED:
            if (desc_.callbacks.on_minimize) desc_.callbacks.on_minimize(true);
            break;

        case SDL_EVENT_WINDOW_RESTORED:
            if (desc_.callbacks.on_minimize) desc_.callbacks.on_minimize(false);
            break;

        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            switch (ev.gaxis.axis) {
            case SDL_GAMEPAD_AXIS_LEFTX:  event_state.gamepad_left_x  = ev.gaxis.value / 32767.0f; break;
            case SDL_GAMEPAD_AXIS_LEFTY:  event_state.gamepad_left_y  = ev.gaxis.value / 32767.0f; break;
            case SDL_GAMEPAD_AXIS_RIGHTX: event_state.gamepad_right_x = ev.gaxis.value / 32767.0f; break;
            case SDL_GAMEPAD_AXIS_RIGHTY: event_state.gamepad_right_y = ev.gaxis.value / 32767.0f; break;
            case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:  event_state.gamepad_left_trigger  = ev.gaxis.value / 32767.0f; break;
            case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER: event_state.gamepad_right_trigger = ev.gaxis.value / 32767.0f; break;
            default: break;
            }
            break;

        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            event_state.gamepad_buttons = ev.gbutton.button;
            break;

        default:
            break;
        }
    }

    // ── Keyboard state ──
    event_state.keyboard_state = keys;
    event_state.events     = event_buffer_.data();
    event_state.num_events = static_cast<int>(event_buffer_.size());

    // ── Sync to WindowState base for renderer ──
    keyboard_state     = event_state.keyboard_state;
    mouse_rel_x        = event_state.mouse_rel_x;
    mouse_rel_y        = event_state.mouse_rel_y;
    scroll_x           = event_state.scroll_x;
    scroll_y           = event_state.scroll_y;
    gamepad_left_x     = event_state.gamepad_left_x;
    gamepad_left_y     = event_state.gamepad_left_y;
    gamepad_right_x    = event_state.gamepad_right_x;
    gamepad_right_y    = event_state.gamepad_right_y;
    gamepad_left_trigger  = event_state.gamepad_left_trigger;
    gamepad_right_trigger = event_state.gamepad_right_trigger;
    gamepad_buttons    = event_state.gamepad_buttons;
}

// ── Input ───────────────────────────────────────────────────────

void Window::set_input_mode(InputMode mode) {
    input_mode = mode;
    if (mode == InputMode::FPS)
        set_cursor_mode(CursorMode::Captured);
    else
        set_cursor_mode(CursorMode::Normal);
}

void Window::set_cursor_mode(CursorMode mode) {
    cursor_mode_ = mode;
    switch (mode) {
    case CursorMode::Normal:
        SDL_SetWindowRelativeMouseMode(sdl_window_, false);
        SDL_ShowCursor();
        break;
    case CursorMode::Hidden:
        SDL_SetWindowRelativeMouseMode(sdl_window_, false);
        SDL_HideCursor();
        break;
    case CursorMode::Captured:
        SDL_SetWindowRelativeMouseMode(sdl_window_, true);
        SDL_HideCursor();
        break;
    }
}

void Window::reset_mouse_delta() {
    event_state.mouse_rel_x = 0.0f;
    event_state.mouse_rel_y = 0.0f;
    mouse_rel_x = 0.0f;
    mouse_rel_y = 0.0f;
}

bool Window::was_key_pressed(int scancode) const {
    return event_state.was_key_pressed(static_cast<SDL_Scancode>(scancode));
}

bool Window::was_key_released(int scancode) const {
    return event_state.was_key_released(static_cast<SDL_Scancode>(scancode));
}

bool Window::mouse_button_down(int button) const {
    return event_state.mouse_button_down(button);
}

} // namespace exd::app
