#pragma once

#include <SDL3/SDL.h>
#include <exd/app/window_state.hpp>
#include <exd/app/event_state.hpp>
#include <exd/app/input_mode.hpp>
#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace exd::app {

/// Window creation parameters.
struct WindowDesc {
    std::string title    = "Extropian";
    int         width    = 1280;
    int         height   = 720;
    bool        fullscreen = false;
    bool        resizable  = true;
    bool        vsync      = true;
    int         msaa_samples = 0;      ///< 0 = off, 4 = 4x MSAA
    int         gl_major   = 4;
    int         gl_minor   = 6;

    /// Optional callbacks. Fired from poll_events() on the calling thread.
    WindowEvents callbacks;
};

/// SDL3 window wrapper with OpenGL context and event polling.
///
/// Implements WindowState so renderer systems can read dimensions,
/// input state, and toggle flags without knowing about SDL.
///
/// Usage:
/// @code
///   WindowDesc desc;
///   desc.title = "My App";
///   desc.width = 1920;
///   desc.height = 1080;
///   desc.callbacks.on_close = [&] { running = false; };
///   Window win(desc);
///   while (!win.should_close()) {
///       win.poll_events();
///       // ... render, consume input ...
///       win.reset_mouse_delta();
///       win.swap_buffers();
///   }
/// @endcode
class Window : public WindowState {
public:
    /// Create and open a window + OpenGL context.
    /// On failure, is_valid() returns false — check after construction.
    explicit Window(const WindowDesc& desc = {});
    ~Window();

    // ── Lifecycle ─────────────────────────────────────────────

    /// True if the window and GL context were created successfully.
    [[nodiscard]] bool is_valid() const { return sdl_window_ != nullptr && gl_context_ != nullptr; }

    /// True once the user has requested close (or die() was called).
    [[nodiscard]] bool should_close() const { return should_close_; }

    /// Request the window to close at the end of the current frame.
    void close() { should_close_ = true; }

    // ── Per-frame ─────────────────────────────────────────────

    void poll_events();
    void swap_buffers();
    void reset_mouse_delta() override;

    // ── Dimensions ────────────────────────────────────────────

    void get_dimensions(int& w, int& h, float& aspect) const override;
    void set_title(const std::string& title);

    // ── Input queries ─────────────────────────────────────────

    bool was_key_pressed(int scancode) const override;
    bool was_key_released(int scancode) const override;
    bool mouse_button_down(int button) const override;

    // ── Input mode / cursor ───────────────────────────────────

    void set_input_mode(InputMode mode);
    void set_cursor_mode(CursorMode mode);
    [[nodiscard]] CursorMode cursor_mode() const { return cursor_mode_; }

    // ── Window decoration ─────────────────────────────────────

    void set_icon(SDL_Surface* surface);

    [[nodiscard]] SDL_Window*   native_window()  const { return sdl_window_; }
    [[nodiscard]] SDL_GLContext native_context() const { return gl_context_; }
    [[nodiscard]] const EventState& events()     const { return event_state; }

    // ── Legacy / public state (prefer methods above) ──────────
    // These are kept public for backward compat with existing
    // renderer code that accesses them directly. Prefer the
    // methods on WindowState for new code.

    EventState event_state;

private:
    void apply_desc(const WindowDesc& desc);

    SDL_Window*   sdl_window_ = nullptr;
    SDL_GLContext gl_context_ = nullptr;
    bool          should_close_ = false;
    CursorMode    cursor_mode_  = CursorMode::Normal;
    std::vector<SDL_Event> event_buffer_;    ///< raw events for the frame

    WindowDesc    desc_;
};

} // namespace exd::app
