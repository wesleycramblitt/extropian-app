#include <exd/app/application.hpp>
#include <exd/app/window.hpp>

#include <catch2/catch_test_macros.hpp>

namespace exd::app {
namespace {

class SpyApplication : public Application {
public:
    bool startup_called  = false;
    bool shutdown_called = false;
    int  update_count    = 0;
    bool should_exit_on_update = false;
    bool window_valid    = false;

    explicit SpyApplication(const WindowDesc& d = {}) : Application(d) {}

protected:
    void on_startup() override {
        startup_called = true;
        window_valid = window().is_valid();
    }
    void on_update(float /*dt*/) override {
        ++update_count;
        if (should_exit_on_update) window().close();
    }
    void on_shutdown() override { shutdown_called = true; }
};

TEST_CASE("Application: construction without run()", "[application]") {
    SpyApplication app;
    CHECK_FALSE(app.startup_called);
    CHECK_FALSE(app.shutdown_called);
    CHECK(app.update_count == 0);
}

TEST_CASE("Application: destructor without run()", "[application]") {
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
    if (!app.window_valid) { SKIP("Window creation failed — no usable display"); }
    CHECK(result == 0);
    CHECK(app.startup_called);
    CHECK(app.shutdown_called);
    CHECK(app.update_count >= 1);
}

TEST_CASE("Application: window() returns valid reference", "[application][display]") {
    struct WindowAccessApp : public SpyApplication {
        bool accessor_worked = false;
        void on_startup() override {
            SpyApplication::on_startup();
            Window& w = window();
            accessor_worked = (&w != nullptr && w.is_valid());
        }
    };
    WindowAccessApp wa;
    wa.should_exit_on_update = true;
    int result = wa.run();
    if (!wa.window_valid) { SKIP("Window creation failed — no usable display"); }
    CHECK(result == 0);
    CHECK(wa.accessor_worked);
}

TEST_CASE("Application: default callbacks are no-ops", "[application]") {
    struct DefaultApp : public Application { using Application::Application; };
    DefaultApp app;
    SUCCEED("DefaultApp constructed");
}

TEST_CASE("Application: shutdown not called twice", "[application][display]") {
    int count = 0;
    struct CountingApp : public SpyApplication {
        int* ctr = nullptr;
        void on_shutdown() override { SpyApplication::on_shutdown(); if (ctr) ++(*ctr); }
    };
    CountingApp app;
    app.ctr = &count;
    app.should_exit_on_update = true;
    std::ignore = app.run();
    if (!app.window_valid) { SKIP("Window creation failed — no usable display"); }
    CHECK(count == 1);
}

TEST_CASE("Application: startup before first update", "[application][display]") {
    struct OrderApp : public SpyApplication {
        bool startup_done = false;
        bool order_ok = false;
        void on_startup() override { SpyApplication::on_startup(); startup_done = true; }
        void on_update(float dt) override { SpyApplication::on_update(dt); order_ok = startup_done; }
    };
    OrderApp app;
    app.should_exit_on_update = true;
    int result = app.run();
    if (!app.window_valid) { SKIP("Window creation failed — no usable display"); }
    CHECK(result == 0);
    CHECK(app.order_ok);
}

TEST_CASE("Application: invalid window returns error code", "[application]") {
    // Providing invalid GL version should fail
    WindowDesc desc;
    desc.gl_major = 99;
    desc.gl_minor = 99;
    SpyApplication app(desc);
    int result = app.run();
    // Should return 1 when window is invalid
    if (app.window_valid) {
        // Somehow succeeded — run worked, that's fine too
        CHECK(result == 0);
    } else {
        CHECK(result == 1);
    }
}

TEST_CASE("Application: delta_time exposed", "[application][display]") {
    struct DtApp : public SpyApplication {
        float captured_dt = -1.0f;
        void on_update(float dt) override {
            SpyApplication::on_update(dt);
            captured_dt = delta_time();
        }
    };
    DtApp app;
    app.should_exit_on_update = true;
    std::ignore = app.run();
    if (!app.window_valid) { SKIP("Window creation failed — no usable display"); }
    CHECK(app.captured_dt >= 0.0f);
    CHECK(app.captured_dt < 1.0f); // first frame should be tiny
}

} // namespace
} // namespace exd::app
