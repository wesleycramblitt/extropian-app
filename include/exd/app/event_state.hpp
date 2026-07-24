#pragma once

#include <SDL3/SDL.h>
#include <array>

namespace exd::app {

/// Maximum number of mouse buttons we track (SDL defines 1-5).
constexpr int kMaxMouseButtons = 8;

/// Per-frame input snapshot. Created by Window::poll_events() each frame.
class EventState {
public:
    EventState() = default;

    /// True only on the frame that `sc` was pressed down.
    bool was_key_pressed(SDL_Scancode sc) const {
        return sc < SDL_SCANCODE_COUNT && key_down_[sc];
    }

    /// True only on the frame that `sc` was released.
    bool was_key_released(SDL_Scancode sc) const {
        return sc < SDL_SCANCODE_COUNT && key_up_[sc];
    }

    /// True if the mouse button is currently held this frame.
    bool mouse_button_down(int button) const {
        return button >= 0 && button < kMaxMouseButtons && mouse_buttons_[button];
    }

    /// Mark a key as just-pressed this frame.
    void set_key_down(SDL_Scancode sc) {
        if (sc < SDL_SCANCODE_COUNT) key_down_[sc] = true;
    }

    /// Mark a key as just-released this frame.
    void set_key_up(SDL_Scancode sc) {
        if (sc < SDL_SCANCODE_COUNT) key_up_[sc] = true;
    }

    /// Set mouse button state (called by Window::poll_events).
    void set_mouse_button(int button, bool down) {
        if (button >= 0 && button < kMaxMouseButtons) mouse_buttons_[button] = down;
    }

    /// Reset all per-frame state (called at the start of each poll).
    void begin_frame();

    // ── Public state fields (read by renderer) ──

    const bool* keyboard_state = nullptr;     ///< SDL_GetKeyboardState view
    SDL_Event*  events         = nullptr;     ///< raw event buffer
    int         num_events     = 0;
    float       mouse_rel_x    = 0.0f;        ///< accumulated relative X this frame
    float       mouse_rel_y    = 0.0f;        ///< accumulated relative Y this frame
    float       scroll_x       = 0.0f;        ///< scroll delta X this frame
    float       scroll_y       = 0.0f;        ///< scroll delta Y this frame

    // Gamepad state (from first connected gamepad)
    float       gamepad_left_x  = 0.0f;
    float       gamepad_left_y  = 0.0f;
    float       gamepad_right_x = 0.0f;
    float       gamepad_right_y = 0.0f;
    float       gamepad_left_trigger  = 0.0f;
    float       gamepad_right_trigger = 0.0f;
    Uint16      gamepad_buttons = 0;          ///< bitmask of held buttons

private:
    friend class Window;
    std::array<bool, SDL_SCANCODE_COUNT> key_down_ = {};
    std::array<bool, SDL_SCANCODE_COUNT> key_up_   = {};
    std::array<bool, kMaxMouseButtons>   mouse_buttons_ = {};
};

} // namespace exd::app
