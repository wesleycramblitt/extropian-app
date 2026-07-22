#pragma once

#include <memory>

namespace exd::app {

class Window;

/// Minimal application skeleton.
///
/// Creates an SDL3 + OpenGL window and runs the main loop.
/// Inherit and override on_startup / on_update / on_shutdown.
///
/// Usage:
/// @code
///   class MyApp : public exd::app::Application {
///       void on_startup() override { ... }
///       void on_update(float dt) override { ... }
///   };
///   int main() { return MyApp().run(); }
/// @endcode
class Application {
public:
    Application();
    virtual ~Application();

    /// Run the main loop. Blocks until the window closes.
    /// Returns 0 on clean exit, nonzero on error.
    [[nodiscard]] int run();

    /// Access the underlying window (SDL3 + OpenGL).
    [[nodiscard]] Window& window();

protected:
    /// Called once after the window is created, before the first frame.
    virtual void on_startup() {}

    /// Called every frame. dt is wall-clock seconds since the last frame.
    virtual void on_update(float dt) {}

    /// Called after the main loop exits, before destruction.
    virtual void on_shutdown() {}

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace exd::app
