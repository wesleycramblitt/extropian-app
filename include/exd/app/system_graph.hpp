#pragma once

#include <memory>
#include <vector>
#include <set>
#include <functional>

namespace exd::ecs { class ISystem; class Registry; }

namespace exd::app {

/// @brief Ordered, mode-gated collection of ECS systems.
///
/// Usage:
/// @code
///   SystemGraph g;
///   g.add<RenderSystem>()        .always();
///   g.add<PhysicsSystem>()       .in_mode(SimMode::Simulate);
///   g.add<GeometryEditorSystem>().in_mode(SimMode::Edit);
///   g.build(); // must call after all registrations
/// @endcode
class SystemGraph {
public:
    SystemGraph() = default;

    /// Register a system type. Returns a builder for mode constraints.
    template <typename T, typename... Args>
    SystemGraph& add(Args&&... args) {
        auto sys = std::make_unique<T>(std::forward<Args>(args)...);
        entries_.push_back(Entry{std::move(sys), std::set<int>{}, true});
        last_added_ = static_cast<int>(entries_.size()) - 1;
        return *this;
    }

    /// This system runs in all modes (default).
    SystemGraph& always();

    /// This system runs only in the given mode(s).
    SystemGraph& in_mode(int mode_id);

    /// This system runs only in the given set of modes.
    SystemGraph& in_modes(std::set<int> mode_ids);

    /// Finalize — must be called after all add() calls, before first update().
    void build();

    /// Called each frame. Only runs systems whose mode matches current_mode.
    void update(exd::ecs::Registry& registry, double dt, int current_mode);

    /// Reorder systems by moving one before another (for dependency ordering).
    void order_before(const std::string& before, const std::string& after);

private:
    struct Entry {
        std::unique_ptr<exd::ecs::ISystem> system;
        std::set<int>                     active_modes; ///< empty = always
        bool                              always_active{true};
    };
    std::vector<Entry> entries_;
    int last_added_{-1};
};

} // namespace exd::app
