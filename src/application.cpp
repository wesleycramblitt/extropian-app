#include <exd/app/application.hpp>

#include <exd/app/mode.hpp>
#include <exd/app/system_graph.hpp>
#include <exd/app/command.hpp>
#include <exd/app/ui_host.hpp>

// RmlUi host (concrete implementation used by default)
#include "../src/rmlui/host.hpp"

#include <exd/core/config.hpp>
#include <exd/ecs/registry.hpp>

#include <cstdio>
#include <chrono>
#include <memory>

namespace exd::app {

struct Application::Impl {
    std::unique_ptr<exd::ecs::Registry> registry;
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

    if (argc > 0) {
        // Parse CLI args into config (future: use CLI11 or similar)
        impl_->config->set("app.name", argv[0] ? argv[0] : "extropian-app");
    }
}

Application::~Application() {
    if (impl_->running) {
        on_shutdown();
    }
}

int Application::run() {
    std::printf("[App] Starting...\n");

    // 1. Configure
    on_configure(*impl_->config);

    // 2. Create UI (RmlUi backend by default)
    impl_->ui = IUIHost::create(IUIHost::Backend::RmlUi, nullptr);

    // 3. Load UI documents
    if (impl_->ui) {
        on_load_ui(*impl_->ui);
    }

    // 4. Register systems
    impl_->system_graph = std::make_unique<SystemGraph>();
    on_register_systems(*impl_->system_graph);
    impl_->system_graph->build();

    // 5. Setup initial ECS state
    on_setup(*impl_->registry);

    // 6. Main loop
    impl_->running = true;
    using clock = std::chrono::steady_clock;
    auto last_frame = clock::now();

    while (impl_->running) {
        auto frame_start = clock::now();
        float dt = std::chrono::duration<float>(frame_start - last_frame).count();
        last_frame = frame_start;

        // UI frame
        if (impl_->ui) {
            impl_->ui->begin_frame();
        }

        // Update systems
        int current_mode = impl_->mode_manager->current().id;
        impl_->system_graph->update(*impl_->registry, dt, current_mode);

        // App-specific update
        on_update(dt);

        // UI end frame (renders RmlUi)
        if (impl_->ui) {
            impl_->ui->end_frame();
        }

        // Cap frame rate (optional, vsync usually handles this)
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

exd::render::IRenderer& Application::renderer() {
    // TODO: Create real renderer via IRenderer::create(backend, window_handle)
    // For now, return a null reference — apps should call IRenderer::create() directly
    static auto null_renderer = exd::render::IRenderer::create(exd::render::IRenderer::Backend::Null);
    if (!null_renderer) {
        // Headless mode — caller must check
    }
    return *null_renderer;
}

void Application::watch_asset(const std::string& path, std::function<void()> on_change) {
    // TODO: Implement file watcher
    (void)path;
    (void)on_change;
}

} // namespace exd::app
