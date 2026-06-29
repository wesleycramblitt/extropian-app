#pragma once

#include <memory>
#include <string_view>
#include <functional>

/// Forward declarations — extropian-app depends on extropian-core and extropian-render
namespace exd::core { class Config; }
namespace exd::ecs  { class Registry; }
namespace exd::render { class IRenderer; }

namespace exd::app {

class ModeManager;
class SystemGraph;
class CommandStack;
struct IUIHost;

/// @brief Application skeleton for all Extropian desktop applications.
///
/// Inherit from this and override the on_* hooks for your app.
/// Handles: mode management, system registration, UI loading, asset hot-reload,
/// undo/redo, config persistence.
///
/// Usage:
/// @code
///   class MyApp : public exd::app::Application {
///       void on_register_systems(SystemGraph& g) override { ... }
///       void on_load_ui(IUIHost& ui) override { ... }
///   };
///   int main(int argc, char** argv) { return MyApp(argc, argv).run(); }
/// @endcode
class Application {
public:
    Application(int argc = 0, char** argv = nullptr);
    virtual ~Application();

    /// ── Main entry ────────────────────────────────────
    /// Call once. Runs the main loop until the window closes.
    /// Returns exit code.
    [[nodiscard]] int run();

protected:
    /// ── Hooks (override in your app) ──────────────────

    /// Parse command-line args into config. Called once before anything else.
    virtual void on_configure(exd::core::Config& config) {}

    /// Create initial ECS entities. Called after systems are registered.
    virtual void on_setup(exd::ecs::Registry& registry) {}

    /// Register systems with mode affinity and ordering.
    /// This is where you call graph.add<YourSystem>().in_mode(...).
    virtual void on_register_systems(SystemGraph& graph) = 0;

    /// Load RmlUi documents for your app's UI.
    virtual void on_load_ui(IUIHost& ui) = 0;

    /// Per-frame update. dt in seconds.
    virtual void on_update(double dt) {}

    /// Called when the mode changes (e.g., Edit → Simulate).
    virtual void on_mode_changed(int from_mode_id, int to_mode_id) {}

    /// Cleanup before exit.
    virtual void on_shutdown() {}

    /// ── Services ──────────────────────────────────────
    [[nodiscard]] exd::ecs::Registry&     registry();
    [[nodiscard]] exd::render::IRenderer& renderer();
    [[nodiscard]] IUIHost&                ui();
    [[nodiscard]] CommandStack&           commands();
    [[nodiscard]] exd::core::Config&      config();
    [[nodiscard]] ModeManager&            modes();

    /// ── Asset hot-reload ──────────────────────────────
    void watch_asset(const std::string& path, std::function<void()> on_change);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace exd::app
