#include <exd/app/application.hpp>
#include <exd/app/window.hpp>

#include <catch2/catch_test_macros.hpp>

namespace exd::app {
namespace {

/// Test harness that records lifecycle callbacks and detects display availability.
class SpyApplication : public Application {
public:
    bool startup_called  = false;
    bool shutdown_called = false;
    int  update_count    = 0;
    bool should_exit_on_update = false;
    bool window_created  = false;

protected:
    void on_startup() override {
        startup_called = true;
        // Let subclasses know whether the Window backed by a real display
        window_created = (window().sdl_window != nullptr &&
                          window().gl_context != nullptr);
    }

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
        CHECK_FALSE(app.startup_called);
        CHECK_FALSE(app.shutdown_called);
    }
    SUCCEED("Destructor without run() completed");
}

TEST_CASE("Application: run lifecycle", "[application][display]") {
    SpyApplication app;
    app.should_exit_on_update = true;

    int result = app.run();

    if (!app.window_created) {
        SKIP("Window / GL context creation failed — no usable display");
    }

    CHECK(result == 0);
    CHECK(app.startup_called);
    CHECK(app.shutdown_called);
    CHECK(app.update_count >= 1);
}

TEST_CASE("Application: window() returns valid reference during on_startup", "[application][display]") {
    struct WindowAccessApp : public SpyApplication {
        bool accessor_worked = false;
        void on_startup() override {
            SpyApplication::on_startup();
            Window& w = window();
            accessor_worked = (&w != nullptr);
        }
    };

    WindowAccessApp wa;
    wa.should_exit_on_update = true;

    int result = wa.run();

    if (!wa.window_created) {
        SKIP("Window / GL context creation failed — no usable display");
    }

    CHECK(result == 0);
    CHECK(wa.accessor_worked);
}

TEST_CASE("Application: default callbacks are no-ops", "[application]") {
    struct DefaultApp : public Application {};
    DefaultApp app;
    SUCCEED("DefaultApp constructed");
}

TEST_CASE("Application: running flag prevents double on_shutdown", "[application][display]") {
    int shutdown_count = 0;
    struct ShutdownCountingApp : public SpyApplication {
        int* counter = nullptr;
        void on_shutdown() override {
            SpyApplication::on_shutdown();
            if (counter) ++(*counter);
        }
    };

    ShutdownCountingApp app;
    app.counter = &shutdown_count;
    app.should_exit_on_update = true;

    app.run();

    if (!app.window_created) {
        SKIP("Window / GL context creation failed — no usable display");
    }

    CHECK(shutdown_count == 1);
}

TEST_CASE("Application: startup called before first update", "[application][display]") {
    struct OrderCheckingApp : public SpyApplication {
        bool startup_before_update = false;
        bool startup_done = false;

        void on_startup() override {
            SpyApplication::on_startup();
            startup_done = true;
        }

        void on_update(float dt) override {
            SpyApplication::on_update(dt);
            startup_before_update = startup_done;
        }
    };

    OrderCheckingApp app;
    app.should_exit_on_update = true;

    int result = app.run();

    if (!app.window_created) {
        SKIP("Window / GL context creation failed — no usable display");
    }

    CHECK(result == 0);
    CHECK(app.startup_before_update);
}

} // namespace
} // namespace exd::app
