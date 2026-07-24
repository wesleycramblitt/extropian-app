#include <exd/app/application.hpp>

#include <cstdio>
#include <chrono>

namespace exd::app {

struct Application::Impl {
    std::unique_ptr<Window> window;
    bool running = false;
};

Application::Application(const WindowDesc& desc)
    : impl_(std::make_unique<Impl>())
{
    impl_->window = std::make_unique<Window>(desc);
}

Application::~Application() {
    if (impl_->running) on_shutdown();
}

int Application::run() {
    if (!impl_->window->is_valid()) {
        std::fprintf(stderr, "[App] Window creation failed — aborting run()\n");
        return 1;
    }

    std::printf("[App] Starting...\n");

    on_startup();

    impl_->running = true;
    using clock = std::chrono::steady_clock;
    auto last_frame = clock::now();

    while (impl_->running && !impl_->window->should_close()) {
        auto frame_start = clock::now();
        last_dt_ = std::chrono::duration<float>(frame_start - last_frame).count();
        last_frame = frame_start;

        impl_->window->poll_events();

        on_update(last_dt_);

        impl_->window->swap_buffers();
    }

    impl_->running = false;
    on_shutdown();
    return 0;
}

Window& Application::window() { return *impl_->window; }

} // namespace exd::app
