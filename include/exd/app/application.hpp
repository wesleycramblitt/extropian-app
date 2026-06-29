#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <functional>

// Forward declarations
namespace exd::core { class Config; }
namespace exd::ecs  { class Registry; }
namespace exd::render { class IRenderer; }
namespace exd::app { class IUIHost; }

namespace exd::app {

class ModeManager;
class SystemGraph;
class CommandStack;

/// @brief Application skeleton for all Extropian desktop applications.
///
/// Inherit from this and override the on_* hooks for your app.
/// Handles: mode management, system registration, UI loading (RmlUi),
/// asset hot-reload, undo/redo, config persistence.
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
    [[nodiscard]] int run();

protected:
    /// ── Hooks (override in your app) ──────────────────
    virtual void on_configure(exd::core::Config& config) {}
    virtual void on_setup(exd::ecs::Registry& registry) {}
    virtual void on_register_systems(SystemGraph& graph) = 0;
    virtual void on_load_ui(IUIHost& ui) = 0;
    virtual void on_update(double dt) {}
    virtual void on_mode_changed(int from_mode_id, int to_mode_id) {}
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
