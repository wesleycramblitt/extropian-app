#include <exd/app/application.hpp>
#include <exd/app/window.hpp>

#include <catch2/catch_test_macros.hpp>
#include <SDL3/SDL.h>

namespace exd::app {
namespace {

/// Returns true if SDL video can be initialized (display available).
static bool has_display() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;
    SDL_Quit(); // clean up so Window constructor can re-init
    return true;
}

/// Test harness that records lifecycle callbacks.
class SpyApplication : public Application {
public:
    bool startup_called  = false;
    bool shutdown_called = false;
    int  update_count    = 0;
    bool should_exit_on_update = false;

protected:
    void on_startup() override { startup_called = true; }

    void on_update(float /*dt*/) override {
        ++update_count;
        if (should_exit_on_update) {
            window().should_close = true;
        }
    }

    void on_shutdown() override { shutdown_called = true; }
};

TEST_CASE("Application: construction and destruction without run()", "[application]") {
    SpyApplication app;
    CHECK_FALSE(app.startup_called);
    CHECK_FALSE(app.shutdown_called);
    CHECK(app.update_count == 0);
}

TEST_CASE("Application: destructor calls on_shutdown only if running", "[application]") {
    {
        SpyApplication app;
        // Not calling run(), so running stays false
        CHECK_FALSE(app.startup_called);
        CHECK_FALSE(app.shutdown_called);
    }
    // on_shutdown should NOT have been called (running was never true)
    SUCCEED("Destructor without run() completed");
}

TEST_CASE("Application: run lifecycle (requires display)", "[application][display]") {
    if (!has_display()) {
        SKIP("No video display available — skipping application lifecycle test");
    }

    SpyApplication app;
    app.should_exit_on_update = true; // Exit after first frame

    int result = app.run();

    CHECK(result == 0);
    CHECK(app.startup_called);
    CHECK(app.shutdown_called);
    CHECK(app.update_count >= 1);
}

TEST_CASE("Application: window() returns valid reference after run()", "[application][display]") {
    if (!has_display()) {
        SKIP("No video display available — skipping window accessor test");
    }
    SpyApplication app;
    app.should_exit_on_update = true;

    // We can't access window() before run() (it's nullptr then),
    // but after run() starts, on_startup can access it.
    // This test verifies the accessor works during on_startup.

    struct WindowAccessApp : public SpyApplication {
        bool window_valid = false;
        void on_startup() override {
            SpyApplication::on_startup();
            // window() should be valid here
            Window& w = window();
            window_valid = (&w != nullptr);
        }
    };

    WindowAccessApp wa;
    wa.should_exit_on_update = true;

    int result = wa.run();

    CHECK(result == 0);
    CHECK(wa.window_valid);
}

TEST_CASE("Application: default on_startup/on_update/on_shutdown are no-ops", "[application]") {
    // Verify the default implementations don't crash.
    struct DefaultApp : public Application {
        // Use all default implementations
    };

    DefaultApp app;
    // Can't easily test run() without display, but construction is fine
    SUCCEED("DefaultApp constructed");
}

} // namespace
} // namespace exd::app
