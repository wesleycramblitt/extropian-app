#include <exd/app/application.hpp>
#include <exd/core/logging.hpp>
#include <exd/core/clock.hpp>

namespace exd::app {

struct Application::Impl {
    std::unique_ptr<Window> window;
    bool running = false;
    exd::core::FrameClock frame_clock;
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
        exd::core::log_error("[App] Window creation failed — aborting run()");
        return 1;
    }

    exd::core::log_info("[App] Starting...");

    on_startup();

    impl_->running = true;

    while (impl_->running && !impl_->window->should_close()) {
        last_dt_ = static_cast<float>(impl_->frame_clock.mark());

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
