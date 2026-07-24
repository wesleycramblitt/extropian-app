#include <exd/app/window_state.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <memory>

namespace exd::app {
namespace {

struct TestWindowState : public WindowState {
    int w = 0, h = 0;
    float a = 0.0f;
    bool key_rel = false;
    bool mb_down = false;
    int  reset_count = 0;

    void get_dimensions(int& width, int& height, float& aspect) const override {
        width  = w;
        height = h;
        aspect = a;
    }
    bool was_key_released(int scancode) const override { return key_rel && scancode == 42; }
    void reset_mouse_delta() override { ++reset_count; }
    bool mouse_button_down(int button) const override { return mb_down && button == 1; }
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
    ws.w = 1920; ws.h = 1080; ws.a = 1920.0f / 1080.0f;
    int width = 0, height = 0; float aspect = 0.0f;
    ws.get_dimensions(width, height, aspect);
    CHECK(width == 1920);
    CHECK(height == 1080);
    CHECK(aspect == Catch::Approx(1920.0f / 1080.0f));
}

TEST_CASE("WindowState: was_key_released virtual dispatch", "[window_state]") {
    TestWindowState ws;
    ws.key_rel = false;
    CHECK_FALSE(ws.was_key_released(42));
    ws.key_rel = true;
    CHECK(ws.was_key_released(42));
}

TEST_CASE("WindowState: reset_mouse_delta virtual dispatch", "[window_state]") {
    TestWindowState ws;
    CHECK(ws.reset_count == 0);
    ws.reset_mouse_delta();
    CHECK(ws.reset_count == 1);
}

TEST_CASE("WindowState: mouse_button_down virtual dispatch", "[window_state]") {
    TestWindowState ws;
    ws.mb_down = true;
    CHECK(ws.mouse_button_down(1));
    CHECK_FALSE(ws.mouse_button_down(2));
}

TEST_CASE("WindowState: mutable state via base pointer", "[window_state]") {
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

TEST_CASE("InputMode: enum values", "[window_state]") {
    CHECK(static_cast<int>(InputMode::FPS) == 0);
    CHECK(static_cast<int>(InputMode::UI) == 1);
}

TEST_CASE("CursorMode: enum values", "[window_state]") {
    CHECK(static_cast<int>(CursorMode::Normal) == 0);
    CHECK(static_cast<int>(CursorMode::Hidden) == 1);
    CHECK(static_cast<int>(CursorMode::Captured) == 2);
}

TEST_CASE("WindowState: gamepad state defaults", "[window_state]") {
    TestWindowState ws;
    CHECK(ws.gamepad_left_x == 0.0f);
    CHECK(ws.gamepad_left_y == 0.0f);
    CHECK(ws.gamepad_right_x == 0.0f);
    CHECK(ws.gamepad_right_y == 0.0f);
    CHECK(ws.gamepad_left_trigger == 0.0f);
    CHECK(ws.gamepad_right_trigger == 0.0f);
    CHECK(ws.gamepad_buttons == 0);
}

TEST_CASE("WindowState: polymorphic deletion through base pointer", "[window_state]") {
    static bool destroyed = false;
    struct DtorState : public WindowState {
        ~DtorState() override { destroyed = true; }
        void get_dimensions(int& w, int& h, float& a) const override { w = h = 0; a = 1.0f; }
        bool was_key_released(int) const override { return false; }
        void reset_mouse_delta() override {}
        bool mouse_button_down(int) const override { return false; }
    };
    destroyed = false;
    { std::unique_ptr<WindowState> ptr = std::make_unique<DtorState>(); CHECK_FALSE(destroyed); }
    CHECK(destroyed);
}

} // namespace
} // namespace exd::app
