#include <exd/app/application.hpp>
#include <exd/app/window.hpp>

#include <cstdio>
#include <chrono>

namespace exd::app {

struct Application::Impl {
    std::unique_ptr<Window> window;
    bool running = false;
};

Application::Application()
    : impl_(std::make_unique<Impl>()) {}

Application::~Application() {
    if (impl_->running) on_shutdown();
}

int Application::run() {
    std::printf("[App] Starting...\n");

    // Create window (SDL3 + OpenGL)
    impl_->window = std::make_unique<Window>();

    // App-specific setup
    on_startup();

    // Main loop
    impl_->running = true;
    using clock = std::chrono::steady_clock;
    auto last_frame = clock::now();

    while (impl_->running && !impl_->window->should_close) {
        auto frame_start = clock::now();
        float dt = std::chrono::duration<float>(frame_start - last_frame).count();
        last_frame = frame_start;

        impl_->window->poll_events();

        on_update(dt);

        impl_->window->swap_buffers();
    }

    impl_->running = false;
    on_shutdown();
    return 0;
}

Window& Application::window() { return *impl_->window; }

} // namespace exd::app
