#include <exd/app/application.hpp>

#include <exd/app/mode.hpp>
#include <exd/app/system_graph.hpp>
#include <exd/app/command.hpp>
#include <exd/render/renderer.hpp>
#include <exd/app/ui_host.hpp>
#include <exd/app/window.hpp>

#include <exd/core/config.hpp>
#include <exd/ecs/registry.hpp>

#include <cstdio>
#include <chrono>
#include <memory>

namespace exd::app {

struct Application::Impl {
    std::unique_ptr<exd::ecs::Registry> registry;
    std::unique_ptr<Window> window;
    std::unique_ptr<IUIHost> ui;
    std::unique_ptr<ModeManager> mode_manager;
    std::unique_ptr<SystemGraph> system_graph;
    std::unique_ptr<CommandStack> command_stack;
    std::unique_ptr<exd::core::Config> config;
    bool running = false;
};

Application::Application(int argc, char** argv)
    : impl_(std::make_unique<Impl>()) {
    impl_->registry = std::make_unique<exd::ecs::Registry>();
    impl_->mode_manager = std::make_unique<ModeManager>();
    impl_->command_stack = std::make_unique<CommandStack>();
    impl_->config = std::make_unique<exd::core::Config>();
    if (argc > 0)
        impl_->config->set("app.name", argv[0]);
}

Application::~Application() {
    if (impl_->running) on_shutdown();
}

int Application::run() {
    std::printf("[App] Starting...\n");

    // 1. Configure
    on_configure(*impl_->config);

    // 2. Create window (SDL3 + OpenGL)
    impl_->window = std::make_unique<Window>();

    // 3. Register systems
    impl_->system_graph = std::make_unique<SystemGraph>();
    on_register_systems(*impl_->system_graph);
    impl_->system_graph->build();

    // 4. Setup initial ECS state
    on_setup(*impl_->registry);

    // 5. Main loop
    impl_->running = true;
    using clock = std::chrono::steady_clock;
    auto last_frame = clock::now();

    while (impl_->running && !impl_->window->should_close) {
        auto frame_start = clock::now();
        float dt = std::chrono::duration<float>(frame_start - last_frame).count();
        last_frame = frame_start;

        // Poll SDL events (keyboard, mouse, window)
        impl_->window->poll_events();

        // Update systems with current mode
        int current_mode = impl_->mode_manager->current().id;
        impl_->system_graph->update(*impl_->registry, dt, current_mode);

        // App-specific per-frame update
        on_update(dt);

        // Swap buffers (presents the rendered frame)
        impl_->window->swap_buffers();
    }

    impl_->running = false;
    on_shutdown();
    return 0;
}

exd::ecs::Registry& Application::registry() { return *impl_->registry; }
IUIHost& Application::ui() { return *impl_->ui; }
CommandStack& Application::commands() { return *impl_->command_stack; }
exd::core::Config& Application::config() { return *impl_->config; }
ModeManager& Application::modes() { return *impl_->mode_manager; }
Window& Application::window() { return *impl_->window; }

exd::render::IRenderer& Application::renderer() {
    static auto null = exd::render::IRenderer::create(exd::render::IRenderer::Backend::Null);
    return *null;
}

void Application::watch_asset(const std::string&, std::function<void()>) {}

} // namespace exd::app
