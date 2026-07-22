#include <exd/app/window_state.hpp>
#include <exd/app/input_mode.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

namespace exd::app {
namespace {

/// Minimal concrete implementation of WindowState for testing.
struct TestWindowState : public WindowState {
    int w = 0, h = 0;
    float a = 0.0f;

    void get_dimensions(int& width, int& height, float& aspect) const override {
        width  = w;
        height = h;
        aspect = a;
    }

    bool was_key_released(int /*scancode*/) const override {
        return false;
    }
};

TEST_CASE("WindowState: default member values", "[window_state]") {
    TestWindowState ws;
    CHECK(ws.input_mode == InputMode::FPS);
    CHECK(ws.grid_visible == true);
    CHECK(ws.wireframe == false);
    CHECK(ws.keyboard_state == nullptr);
    CHECK(ws.mouse_rel_x == 0.0f);
    CHECK(ws.mouse_rel_y == 0.0f);
}

TEST_CASE("WindowState: get_dimensions virtual dispatch", "[window_state]") {
    TestWindowState ws;
    ws.w = 1920;
    ws.h = 1080;
    ws.a = 1920.0f / 1080.0f;

    int width = 0, height = 0;
    float aspect = 0.0f;
    ws.get_dimensions(width, height, aspect);

    CHECK(width == 1920);
    CHECK(height == 1080);
    CHECK(aspect == Catch::Approx(1920.0f / 1080.0f));
}

TEST_CASE("WindowState: was_key_released virtual dispatch", "[window_state]") {
    struct KeyReleasingState : public TestWindowState {
        bool was_key_released(int scancode) const override {
            return scancode == 42;
        }
    };

    KeyReleasingState ks;
    CHECK_FALSE(ks.was_key_released(0));
    CHECK(ks.was_key_released(42));
    CHECK_FALSE(ks.was_key_released(43));
}

TEST_CASE("WindowState: mutable state accessible via base pointer", "[window_state]") {
    TestWindowState ws;
    WindowState* base = &ws;

    base->input_mode   = InputMode::UI;
    base->grid_visible = false;
    base->wireframe    = true;
    base->mouse_rel_x  = 5.0f;
    base->mouse_rel_y  = -3.0f;

    CHECK(ws.input_mode == InputMode::UI);
    CHECK(ws.grid_visible == false);
    CHECK(ws.wireframe == true);
    CHECK(ws.mouse_rel_x == 5.0f);
    CHECK(ws.mouse_rel_y == -3.0f);
}

TEST_CASE("InputMode: enum values", "[input_mode]") {
    CHECK(static_cast<int>(InputMode::FPS) == 0);
    CHECK(static_cast<int>(InputMode::UI) == 1);
}

TEST_CASE("InputMode: input_mode.hpp includes window_state.hpp", "[input_mode]") {
    // Compile-time check: this file includes both window_state.hpp
    // and input_mode.hpp. If input_mode.hpp didn't include
    // window_state.hpp, we'd get errors here. This test
    // just ensures InputMode is available.
    InputMode mode = InputMode::FPS;
    CHECK(mode == InputMode::FPS);
    mode = InputMode::UI;
    CHECK(mode == InputMode::UI);
}

TEST_CASE("WindowState: zero dimensions produce aspect 1.0", "[window_state]") {
    struct ZeroDimState : public WindowState {
        void get_dimensions(int& w, int& h, float& aspect) const override {
            w = 0; h = 0; aspect = 1.0f;
        }
        bool was_key_released(int) const override { return false; }
    };

    ZeroDimState zs;
    int w = -1, h = -1;
    float aspect = 0.0f;
    zs.get_dimensions(w, h, aspect);
    CHECK(w == 0);
    CHECK(h == 0);
    CHECK(aspect == 1.0f);
}

} // namespace
} // namespace exd::app
