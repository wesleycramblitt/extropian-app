#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <memory>

namespace ext::app {

/// @brief Application-defined mode handle.
///
/// Each application defines its own mode enum (e.g., SimMode::Setup, GameMode::Play).
/// The ModeManager stores them as int handles. Use your enum values.
struct Mode {
    int         id;
    std::string name;        ///< Human-readable (for UI display)
    std::string description; ///< Tooltip text
};

/// @brief Manages mode transitions and notifies systems.
///
/// Usage:
/// @code
///   modes().define({0, "Edit",  "Build geometry and assign BCs"});
///   modes().define({1, "Sim",   "Run the solver"});
///   modes().request_transition(1); // switch to Sim mode
/// @endcode
class ModeManager {
public:
    ModeManager() = default;

    void define(Mode mode);
    void request_transition(int target_mode_id);

    [[nodiscard]] const Mode& current() const;
    [[nodiscard]] bool is_transitioning() const;
    [[nodiscard]] const Mode* find(int id) const;

    /// Register a callback for mode changes.
    void on_change(std::function<void(int from, int to)> callback);

private:
    std::vector<Mode> modes_;
    int current_id_{0};
    int pending_id_{-1};
    bool transitioning_{false};
    std::vector<std::function<void(int, int)>> callbacks_;
};

} // namespace ext::app
