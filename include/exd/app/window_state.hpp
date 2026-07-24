#pragma once

#include <cstdint>
#include <functional>

namespace exd::app {

// ── Enums ────────────────────────────────────────────────────────

enum class InputMode {
    FPS = 0,  ///< Relative mouse, camera moves with WASD + Mouse
    UI  = 1   ///< Absolute mouse, UI captures input, camera frozen
};

enum class CursorMode {
    Normal   = 0,  ///< Visible, not captured
    Hidden   = 1,  ///< Invisible but free to move
    Captured = 2   ///< Hidden + locked to window (for FPS camera)
};

// ── Window events ────────────────────────────────────────────────

struct WindowEvents {
    /// Called when the framebuffer size changes (pixels).
    std::function<void(int width, int height)> on_resize;

    /// Called when the window gains or loses keyboard focus.
    std::function<void(bool focused)> on_focus;

    /// Called when the window is minimized or restored.
    std::function<void(bool minimized)> on_minimize;

    /// Called when the user requests window close (X button, Alt+F4).
    std::function<void()> on_close;
};

// ── WindowState (interface consumed by renderer systems) ─────────

/// Minimal window-state interface used by the renderer.
///
/// Implemented by `Window` to provide dimension / input / toggle
/// values that the rendering systems need each frame.
struct WindowState {
    virtual ~WindowState() = default;

    /// Retrieve the current framebuffer dimensions and aspect ratio.
    virtual void get_dimensions(int& w, int& h, float& aspect) const = 0;

    /// True on the frame that the key was released.
    virtual bool was_key_released(int scancode) const = 0;

    /// Reset accumulated mouse delta to zero (call after consuming).
    virtual void reset_mouse_delta() = 0;

    /// Query mouse button state.
    virtual bool mouse_button_down(int button) const = 0;

    // ── Mutable state (read / written by both app & renderer) ──

    InputMode   input_mode      = InputMode::FPS;
    const bool* keyboard_state  = nullptr;   ///< SDL_GetKeyboardState view
    float       mouse_rel_x     = 0.0f;
    float       mouse_rel_y     = 0.0f;
    bool        grid_visible    = true;
    bool        wireframe       = false;

    // Gamepad state
    float       gamepad_left_x  = 0.0f;
    float       gamepad_left_y  = 0.0f;
    float       gamepad_right_x = 0.0f;
    float       gamepad_right_y = 0.0f;
    float       gamepad_left_trigger  = 0.0f;
    float       gamepad_right_trigger = 0.0f;
    uint16_t    gamepad_buttons = 0;
};

} // namespace exd::app
