#pragma once

namespace exd::app {

/// Toggles between first-person camera control and UI interaction mode.
enum class InputMode {
    FPS,  ///< Relative mouse, camera moves with WASD+Mouse
    UI    ///< Absolute mouse, UI captures input, camera frozen
};

} // namespace exd::app
